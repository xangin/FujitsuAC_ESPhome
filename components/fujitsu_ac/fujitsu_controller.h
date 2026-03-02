#pragma once
#include "esphome/components/uart/uart.h"
#include "fujitsu_register.h"
#include "fujitsu_enums.h"
#include "fujitsu_registry_table.h"
#include <functional>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace fujitsu_ac {

    class FujitsuBuffer {
    public:
        explicit FujitsuBuffer(esphome::uart::UARTComponent *uart) : uart_(uart) {}

        void loop(std::function<void(uint8_t buf[128], int size, bool valid)> cb) {
            while (uart_->available()) {
                uint8_t b;
                if (!uart_->read_byte(&b)) break;
                uint32_t now = millis();
                if ((now - last_millis_) >= 20) current_index_ = 0;
                last_millis_ = now;
                buffer_[current_index_++] = b;
                if (current_index_ > 4 && current_index_ == (int)buffer_[4] + 7) {
                    cb(buffer_, (int)buffer_[4] + 7, is_valid_frame(buffer_, (int)buffer_[4] + 7));
                }
            }
        }

    private:
        esphome::uart::UARTComponent *uart_;
        uint32_t last_millis_ = 0;
        uint8_t  buffer_[128];
        int      current_index_ = 0;

        bool is_valid_frame(uint8_t buf[128], int size) {
            uint16_t frame_cs = ((uint16_t)buf[size - 2] << 8) | buf[size - 1];
            uint16_t cs = 0xFFFF;
            for (int i = 0; i < size - 2; i++) cs -= buf[i];
            return frame_cs == cs;
        }
    };

    // -----------------------------------------------------------------------

    class FujitsuController {
    public:
        explicit FujitsuController(esphome::uart::UARTComponent *uart)
            : uart_(uart), buffer_(uart) {}

        void setup() {
            initialized_ = true;
            last_request_ms_ = millis();
        }

        void loop() {
            if (!initialized_) return;
            send_request();
            buffer_.loop([this](uint8_t buf[128], int size, bool valid) {
                on_frame(buf, size, valid);
            });
        }

        // ---- setters ----
        void setPower(Power p) {
            queue_send(Address::Power, (uint16_t)p);
        }
        void setMode(Mode m) {
            if (is_minimum_heat_enabled()) return;
            queue_send(Address::Mode, (uint16_t)m);
        }
        void setFanSpeed(FanSpeed f) {
            if (is_minimum_heat_enabled()) return;
            queue_send(Address::FanSpeed, (uint16_t)f);
        }
        void setTemp(float celsius) {
            if (is_minimum_heat_enabled()) return;
            auto *mode_reg = registry_.getRegister(Address::Mode);
            if ((uint16_t)Mode::Fan == mode_reg->value) return;

            int min_temp = ((uint16_t)Mode::Heat == mode_reg->value) ? 160 : 180;
            int result = (int)(celsius * 10 + 0.5f);
            result = (result + 2) / 5 * 5;
            if (result < min_temp) result = min_temp;
            if (result > 300)      result = 300;
            queue_send(Address::SetpointTemp, (uint16_t)result);
        }
        void setVerticalSwing(VerticalSwing vs) {
            queue_send(Address::VerticalSwing, (uint16_t)vs);
        }
        void setVerticalAirflow(VerticalAirflow va) {
            queue_send2(Address::VerticalSwing, (uint16_t)VerticalSwing::Off,
                        Address::VerticalAirflowSetterRegistry, (uint16_t)va);
        }
        void setHorizontalSwing(HorizontalSwing hs) {
            if (!is_horizontal_swing_supported()) return;
            queue_send(Address::HorizontalSwing, (uint16_t)hs);
        }
        void setHorizontalAirflow(HorizontalAirflow ha) {
            if (!is_horizontal_swing_supported()) return;
            queue_send2(Address::HorizontalSwing, (uint16_t)HorizontalSwing::Off,
                        Address::HorizontalAirflowSetterRegistry, (uint16_t)ha);
        }
        void setPowerful(Powerful pf) {
            if (is_minimum_heat_enabled()) return;
            queue_send(Address::Powerful, (uint16_t)pf);
        }
        void setEconomy(EconomyMode em) {
            if (is_minimum_heat_enabled()) return;
            queue_send(Address::EconomyMode, (uint16_t)em);
        }

        // ---- getters ----
        bool isPoweredOn() {
            return (uint16_t)Power::On == registry_.getRegister(Address::Power)->value;
        }
        bool isHorizontalSwingSupported() { return is_horizontal_swing_supported(); }
        bool isHumanSensorSupported() {
            return 0x0001 == registry_.getRegister(Address::HumanSensorSupported)->value;
        }

        AcRegister *getRegister(Address a) { return registry_.getRegister(a); }

        void setOnRegisterChangeCallback(std::function<void(const AcRegister *)> cb) {
            on_change_cb_ = cb;
        }

    private:
        esphome::uart::UARTComponent *uart_;
        FujitsuBuffer  buffer_;
        RegistryTable  registry_;

        uint32_t last_request_ms_ = 0;
        bool     last_response_received_ = true;
        bool     no_response_notified_   = false;
        bool     initialized_  = false;
        bool     terminated_   = false;

        std::function<void(const AcRegister *)> on_change_cb_;

        // pending write frame (max 2 registers at once for swing combos)
        struct SendFrame {
            size_t   size = 0;
            Address  regs[19];
            uint16_t vals[19];
        } send_frame_;

        enum class FrameType : int {
            None = -1, Init1 = 0, Init2 = 1,
            InitRegs1 = 2, InitRegs2 = 3, InitRegs3 = 4,
            FrameA = 5, FrameB = 6, FrameC = 7,
            SendRegs = 8, CheckRegs = 9,
        };
        FrameType last_frame_ = FrameType::None;

        struct ReadFrame {
            FrameType type;
            size_t    size;
            Address   regs[19];
        };

        ReadFrame init_regs1_ = {FrameType::InitRegs1, 2,  {Address::Initial0, Address::Initial1}};
        ReadFrame init_regs2_ = {FrameType::InitRegs2, 14, {
            Address::Initial2,  Address::Initial3,  Address::Initial4,  Address::Initial5,
            Address::Initial6,  Address::Initial7,  Address::Initial8,  Address::Initial9,
            Address::Initial10, Address::Initial11, Address::Initial12, Address::Initial13,
            Address::HorizontalSwingSupported, Address::Initial15,
        }};
        ReadFrame init_regs3_ = {FrameType::InitRegs3, 10, {
            Address::Initial16, Address::Initial17, Address::HumanSensorSupported,
            Address::Initial19, Address::Initial20, Address::Initial21, Address::Initial22,
            Address::Initial23, Address::Initial24, Address::Initial25,
        }};
        ReadFrame frame_a_ = {FrameType::FrameA, 13, {
            Address::Power, Address::Mode, Address::SetpointTemp, Address::FanSpeed,
            Address::VerticalAirflowSetterRegistry, Address::VerticalSwing, Address::VerticalAirflow,
            Address::HorizontalAirflowSetterRegistry, Address::HorizontalSwing, Address::HorizontalAirflow,
            Address::Register11, Address::ActualTemp, Address::Register13,
        }};
        ReadFrame frame_b_ = {FrameType::FrameB, 19, {
            Address::EconomyMode, Address::MinimumHeat, Address::HumanSensor,
            Address::Register17, Address::Register18, Address::Register19, Address::Register20,
            Address::Register21, Address::EnergySavingFan, Address::Register23,
            Address::Powerful,   Address::OutdoorUnitLowNoise, Address::Register26,
            Address::Register27, Address::Register28, Address::Register29, Address::Register30,
            Address::Register31, Address::Register32,
        }};
        ReadFrame frame_c_ = {FrameType::FrameC, 12, {
            Address::Register33, Address::Register34, Address::Register35, Address::Register36,
            Address::Register37, Address::Register38, Address::Register39, Address::Register40,
            Address::Register41, Address::OutdoorTemp, Address::Register43, Address::Register44,
        }};

        // helpers
        void queue_send(Address a, uint16_t v) {
            if (send_frame_.size > 0) return;
            send_frame_.size = 1;
            send_frame_.regs[0] = a;
            send_frame_.vals[0] = v;
        }
        void queue_send2(Address a0, uint16_t v0, Address a1, uint16_t v1) {
            if (send_frame_.size > 0) return;
            send_frame_.size = 2;
            send_frame_.regs[0] = a0; send_frame_.vals[0] = v0;
            send_frame_.regs[1] = a1; send_frame_.vals[1] = v1;
        }
        bool is_minimum_heat_enabled() {
            return (uint16_t)MinimumHeat::On == registry_.getRegister(Address::MinimumHeat)->value;
        }
        bool is_horizontal_swing_supported() {
            return 0x0015 == registry_.getRegister(Address::HorizontalSwingSupported)->value;
        }

        uint16_t compute_checksum(const uint8_t *buf, int len) {
            uint16_t cs = 0xFFFF;
            for (int i = 0; i < len; i++) cs -= buf[i];
            return cs;
        }

        void uart_write(const uint8_t *buf, size_t sz) {
            for (size_t i = 0; i < sz; i++) uart_->write_byte(buf[i]);
        }

        void request_registries(ReadFrame &f) {
            last_frame_ = f.type;
            last_response_received_ = false;
            size_t buf_sz = 2 * f.size + 7;
            uint8_t req[128] = {0x03, 0x00, 0x00, 0x00, (uint8_t)(2 * f.size)};
            uint16_t cs = 0xFFFF - 0x03 - (uint8_t)(2 * f.size);
            for (size_t i = 0; i < f.size; i++) {
                int idx = 5 + i * 2;
                req[idx]     = ((uint16_t)f.regs[i] >> 8) & 0xFF;
                req[idx + 1] = (uint16_t)f.regs[i] & 0xFF;
                cs -= req[idx];
                cs -= req[idx + 1];
            }
            req[buf_sz - 2] = (cs >> 8) & 0xFF;
            req[buf_sz - 1] = cs & 0xFF;
            uart_write(req, buf_sz);
        }

        void send_registries() {
            last_frame_ = FrameType::SendRegs;
            last_response_received_ = false;
            size_t buf_sz = 4 * send_frame_.size + 7;
            uint8_t req[128] = {0x02, 0x00, 0x00, 0x00, (uint8_t)(4 * send_frame_.size)};
            uint16_t cs = 0xFFFF - 0x02 - (uint8_t)(4 * send_frame_.size);
            for (size_t i = 0; i < send_frame_.size; i++) {
                int idx = 5 + i * 4;
                req[idx]     = ((uint16_t)send_frame_.regs[i] >> 8) & 0xFF;
                req[idx + 1] = (uint16_t)send_frame_.regs[i] & 0xFF;
                req[idx + 2] = (send_frame_.vals[i] >> 8) & 0xFF;
                req[idx + 3] = send_frame_.vals[i] & 0xFF;
                cs -= req[idx]; cs -= req[idx+1]; cs -= req[idx+2]; cs -= req[idx+3];
            }
            req[buf_sz - 2] = (cs >> 8) & 0xFF;
            req[buf_sz - 1] = cs & 0xFF;
            uart_write(req, buf_sz);
        }

        void send_request() {
            if (terminated_) return;
            uint32_t now = millis();

            if (!last_response_received_ && (now - last_request_ms_) >= 200) {
                if (last_frame_ == FrameType::None || last_frame_ == FrameType::Init1) {
                    last_frame_ = FrameType::None;
                    last_response_received_ = true;
                }
                return;
            }

            if (last_response_received_ && (now - last_request_ms_) >= 400) {
                last_request_ms_ = now;
                switch (last_frame_) {
                    case FrameType::None: {
                        last_frame_ = FrameType::Init1;
                        last_response_received_ = false;
                        uint8_t p[] = {0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0xFF,0xFB};
                        uart_write(p, sizeof(p));
                        break;
                    }
                    case FrameType::Init1: {
                        last_frame_ = FrameType::Init2;
                        last_response_received_ = false;
                        uint8_t p[] = {0x01,0x00,0x00,0x00,0x04,0x00,0x04,0x00,0x01,0xFF,0xF5};
                        uart_write(p, sizeof(p));
                        break;
                    }
                    case FrameType::Init2:    request_registries(init_regs1_); break;
                    case FrameType::InitRegs1: request_registries(init_regs2_); break;
                    case FrameType::InitRegs2: request_registries(init_regs3_); break;
                    case FrameType::InitRegs3:
                    case FrameType::FrameC:   request_registries(frame_a_); break;
                    case FrameType::FrameA:
                        if (send_frame_.size > 0) { send_registries(); break; }
                        // fall through
                    case FrameType::SendRegs:
                        if (send_frame_.size > 0) {
                            // verify written registers
                            ReadFrame verify = {FrameType::CheckRegs, send_frame_.size};
                            for (size_t i = 0; i < send_frame_.size; i++)
                                verify.regs[i] = send_frame_.regs[i];
                            send_frame_.size = 0;
                            request_registries(verify);
                            break;
                        }
                        // fall through
                    case FrameType::CheckRegs: request_registries(frame_b_); break;
                    case FrameType::FrameB:    request_registries(frame_c_); break;
                    default: break;
                }
            }
        }

        void update_registries(uint8_t buf[128], int size) {
            int count = buf[4] / 4;
            for (int i = 0; i < count; i++) {
                int idx = 6 + i * 4;
                uint16_t addr_raw = ((uint16_t)buf[idx] << 8) | buf[idx+1];
                uint16_t new_val  = ((uint16_t)buf[idx+2] << 8) | buf[idx+3];
                Address  addr = (Address)addr_raw;
                AcRegister *reg = registry_.getRegister(addr);
                if (reg->value != new_val) {
                    reg->value = new_val;
                    if (on_change_cb_) on_change_cb_(reg);
                }
            }
        }

        void on_frame(uint8_t buf[128], int size, bool valid) {
            if (!initialized_ || terminated_) return;
            if (!valid) return;
            no_response_notified_ = false;

            if (last_frame_ == FrameType::Init1) {
                uint8_t exp1[8] = {0xFE,0x00,0x00,0x00,0x01,0x02,0xFE,0xFE};
                uint8_t exp2[8] = {0x00,0x00,0x00,0x00,0x01,0x01,0xFF,0xFD};
                if (size == 8 && memcmp(buf, exp1, 8) == 0) return;  // restart ack — wait
                if (size != 8 || memcmp(buf, exp2, 8) != 0) { terminated_ = true; return; }
                last_response_received_ = true;
                return;
            }
            if (last_frame_ == FrameType::Init2) {
                uint8_t exp[8] = {0x01,0x00,0x00,0x00,0x01,0x01,0xFF,0xFC};
                if (size != 8 || memcmp(buf, exp, 8) != 0) { terminated_ = true; return; }
                last_response_received_ = true;
                return;
            }
            if (buf[0] == 0x03) {
                last_response_received_ = true;
                if (buf[5] == 0x01) update_registries(buf, size);
                return;
            }
            if (buf[0] == 0x02) {
                last_response_received_ = true;
            }
        }
    };

}  // namespace fujitsu_ac
