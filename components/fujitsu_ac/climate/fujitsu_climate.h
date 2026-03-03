#pragma once

// ── 必要的 C++ 標準函式庫 ───────────────────────────────────
#include <cstdint>
#include <cstddef>
#include <functional>
#include <utility>
#include <string>

// ── ESPHome 核心 ─────────────────────────────────────────────
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif

// =============================================================================
// Enums（源自 Enums.h）
// =============================================================================
namespace fujitsu_ac_impl {

    enum class Power : uint16_t { Off = 0x0000, On = 0x0001 };
    enum class Mode  : uint16_t { Auto = 0x0000, Cool = 0x0001, Dry = 0x0002, Fan = 0x0003, Heat = 0x0004, None = 0x0005 };
    enum class FanSpeed : uint16_t { Auto = 0x0000, Quiet = 0x0002, Low = 0x0005, Medium = 0x0008, High = 0x000B };
    enum class VerticalAirflow  : uint16_t { Position1=0x0001, Position2=0x0002, Position3=0x0003, Position4=0x0004, Position5=0x0005, Position6=0x0006, Swing=0x0020 };
    enum class VerticalSwing    : uint16_t { Off=0x0000, On=0x0001 };
    enum class HorizontalAirflow: uint16_t { Position1=0x0001, Position2=0x0002, Position3=0x0003, Position4=0x0004, Position5=0x0005, Position6=0x0006, Swing=0x0020 };
    enum class HorizontalSwing  : uint16_t { Off=0x0000, On=0x0001 };
    enum class Powerful         : uint16_t { Off=0x0000, On=0x0001 };
    enum class EconomyMode      : uint16_t { Off=0x0000, On=0x0001 };
    enum class MinimumHeat      : uint16_t { Off=0x0000, On=0x0001 };
    enum class EnergySavingFan  : uint16_t { Off=0x0000, On=0x0001 };
    enum class OutdoorUnitLowNoise: uint16_t { Off=0x0000, On=0x0001 };
    enum class HumanSensor      : uint16_t { Off=0x0000, On=0x0001 };

// =============================================================================
// Register（源自 Register.h）
// =============================================================================
    enum class Address : uint16_t {
        Initial0=0x0001, Initial1=0x0101,
        Initial2=0x0110, Initial3=0x0111, Initial4=0x0112, Initial5=0x0113,
        Initial6=0x0114, Initial7=0x0115, Initial8=0x0117, Initial9=0x011A,
        Initial10=0x011D, Initial11=0x0120, Initial12=0x0130, Initial13=0x0131,
        HorizontalSwingSupported=0x0142, Initial15=0x0143,
        Initial16=0x0150, Initial17=0x0151, HumanSensorSupported=0x0152,
        Initial19=0x0153, Initial20=0x0154, Initial21=0x0155,
        Initial22=0x0156, Initial23=0x0170, Initial24=0x0171, Initial25=0x0193,

        Power=0x1000, Mode=0x1001, SetpointTemp=0x1002, FanSpeed=0x1003,
        VerticalAirflowSetterRegistry=0x1010,
        VerticalSwing=0x1011, VerticalAirflow=0x10A0,
        HorizontalAirflowSetterRegistry=0x1022,
        HorizontalSwing=0x1023, HorizontalAirflow=0x10A9,
        Register11=0x1031, ActualTemp=0x1033, Register13=0x1034,
        EconomyMode=0x1100, MinimumHeat=0x1101, HumanSensor=0x1102,
        Register17=0x1103, Register18=0x1104, Register19=0x1105,
        Register20=0x1106, Register21=0x1107, EnergySavingFan=0x1108,
        Register23=0x1109, Powerful=0x1120, OutdoorUnitLowNoise=0x1121,
        Register26=0x1144, Register27=0x1200, Register28=0x1201,
        Register29=0x1202, Register30=0x1203, Register31=0x1204,
        Register32=0x1141,
        Register33=0x1400, Register34=0x1401, Register35=0x1402,
        Register36=0x1403, Register37=0x1404, Register38=0x1405,
        Register39=0x1406, Register40=0x140E,
        Register41=0x2000, OutdoorTemp=0x2020,
        Register43=0x2021, Register44=0xF001,
    };

    struct AcRegister { Address address; uint16_t value; bool writable; };

// =============================================================================
// RegistryTable（源自 RegistryTable.h）
// =============================================================================
    class RegistryTable {
    public:
        AcRegister *getRegister(Address address) {
            for (size_t i = 0; i < TABLE_SIZE; i++)
                if (registerTable[i].address == address) return &registerTable[i];
            return &registerTable[0];
        }
    private:
        static constexpr size_t TABLE_SIZE = 70;
        AcRegister registerTable[TABLE_SIZE] = {
            {Address::Initial0,0x0000,false},{Address::Initial1,0x0000,false},
            {Address::Initial2,0x0000,false},{Address::Initial3,0x0000,false},
            {Address::Initial4,0x0000,false},{Address::Initial5,0x0000,false},
            {Address::Initial6,0x0000,false},{Address::Initial7,0x0000,false},
            {Address::Initial8,0x0000,false},{Address::Initial9,0x0000,false},
            {Address::Initial10,0x0000,false},{Address::Initial11,0x0000,false},
            {Address::Initial12,0x0000,false},{Address::Initial13,0x0000,false},
            {Address::HorizontalSwingSupported,0x0000,false},{Address::Initial15,0x0000,false},
            {Address::Initial16,0x0000,false},{Address::Initial17,0x0000,false},
            {Address::HumanSensorSupported,0x0000,false},{Address::Initial19,0x0000,false},
            {Address::Initial20,0x0000,false},{Address::Initial21,0x0000,false},
            {Address::Initial22,0x0000,false},{Address::Initial23,0x0000,false},
            {Address::Initial24,0x0000,false},{Address::Initial25,0x0000,false},
            {Address::Power,0x0000,false},{Address::Mode,0x0000,false},
            {Address::SetpointTemp,0x0000,false},{Address::FanSpeed,0x0000,false},
            {Address::VerticalAirflowSetterRegistry,0x0000,false},
            {Address::VerticalSwing,0x0000,false},{Address::VerticalAirflow,0x0000,false},
            {Address::HorizontalAirflowSetterRegistry,0x0000,false},
            {Address::HorizontalSwing,0x0000,false},{Address::HorizontalAirflow,0x0000,false},
            {Address::Register11,0x0000,false},{Address::ActualTemp,0x0000,false},
            {Address::Register13,0x0000,false},
            {Address::EconomyMode,0x0000,false},{Address::MinimumHeat,0x0000,false},
            {Address::HumanSensor,0x0000,false},{Address::Register17,0x0000,false},
            {Address::Register18,0x0000,false},{Address::Register19,0x0000,false},
            {Address::Register20,0x0000,false},{Address::Register21,0x0000,false},
            {Address::EnergySavingFan,0x0000,false},{Address::Register23,0x0000,false},
            {Address::Powerful,0x0000,false},{Address::OutdoorUnitLowNoise,0x0000,false},
            {Address::Register26,0x0000,false},{Address::Register27,0x0000,false},
            {Address::Register28,0x0000,false},{Address::Register29,0x0000,false},
            {Address::Register30,0x0000,false},{Address::Register31,0x0000,false},
            {Address::Register32,0x0000,false},
            {Address::Register33,0x0000,false},{Address::Register34,0x0000,false},
            {Address::Register35,0x0000,false},{Address::Register36,0x0000,false},
            {Address::Register37,0x0000,false},{Address::Register38,0x0000,false},
            {Address::Register39,0x0000,false},{Address::Register40,0x0000,false},
            {Address::Register41,0x0000,false},{Address::OutdoorTemp,0x0000,false},
            {Address::Register43,0x0000,false},{Address::Register44,0x0000,false},
        };
    };

// =============================================================================
// FujitsuController（含 Buffer 邏輯，源自 FujitsuController.h/.cpp + Buffer.h/.cpp）
// =============================================================================
    static const char *const CTRL_TAG = "fujitsu_ctrl";

    class FujitsuController {
    public:
        explicit FujitsuController(esphome::uart::UARTComponent *uart)
            : uart_(uart) {}

        void setup() {
            ESP_LOGI(CTRL_TAG, "FujitsuController::setup()");
        }

        void setOnRegisterChangeCallback(std::function<void(const AcRegister*)> cb) {
            on_register_change_ = cb;
        }

        AcRegister *getRegister(Address address) {
            return registry_.getRegister(address);
        }

        // ── Setter 方法 ──────────────────────────────────────
        void setPower(Power v)                { pendingWrite(Address::Power,    (uint16_t)v); }
        void setMode(Mode v)                  { pendingWrite(Address::Mode,     (uint16_t)v); }
        void setTemp(float celsius)           { pendingWrite(Address::SetpointTemp, (uint16_t)(celsius * 10)); }
        void setFanSpeed(FanSpeed v)          { pendingWrite(Address::FanSpeed, (uint16_t)v); }
        void setVerticalSwing(VerticalSwing v){ pendingWrite(Address::VerticalSwing, (uint16_t)v); }
        void setVerticalAirflow(VerticalAirflow v){ pendingWrite(Address::VerticalAirflowSetterRegistry, (uint16_t)v); }
        void setHorizontalSwing(HorizontalSwing v){ pendingWrite(Address::HorizontalSwing, (uint16_t)v); }
        void setHorizontalAirflow(HorizontalAirflow v){ pendingWrite(Address::HorizontalAirflowSetterRegistry, (uint16_t)v); }
        void setPowerful(Powerful v)          { pendingWrite(Address::Powerful, (uint16_t)v); }
        void setEconomyMode(EconomyMode v)    { pendingWrite(Address::EconomyMode, (uint16_t)v); }

        // ── 主迴圈 ────────────────────────────────────────────
        void loop() {
            // 接收並解析資料
            readAndProcess();
            // 發送控制封包
            sendPending();
        }

    private:
        esphome::uart::UARTComponent *uart_;
        RegistryTable registry_;
        std::function<void(const AcRegister*)> on_register_change_;

        // 通訊狀態機
        enum class CommState { Init1, Init2, InitReg1, InitReg2, InitReg3, FrameA, FrameB, FrameC };
        CommState state_     = CommState::Init1;
        uint32_t  last_send_ = 0;
        bool      init_done_ = false;

        // 待寫入緩衝
        static constexpr int MAX_PENDING = 8;
        struct PendingWrite { Address addr; uint16_t value; };
        PendingWrite pending_[MAX_PENDING];
        int         pending_count_ = 0;

        // 接收緩衝
        uint8_t rx_buf_[128];
        int     rx_len_  = 0;
        uint32_t rx_last_ = 0;

        // ── 封包格式 ─────────────────────────────────────────
        // 所有封包格式：[0x40][len][addr_hi][addr_lo][val_hi][val_lo][cksum_hi][cksum_lo]
        static uint16_t calcChecksum(const uint8_t *b, int len) {
            uint32_t s = 0;
            for (int i = 0; i < len; i++) s += b[i];
            return (uint16_t)(s & 0xFFFF);
        }

        void sendPacket(uint16_t addr, uint16_t value) {
            uint8_t pkt[8];
            pkt[0] = 0x40;
            pkt[1] = 0x06;
            pkt[2] = (addr >> 8) & 0xFF;
            pkt[3] = addr & 0xFF;
            pkt[4] = (value >> 8) & 0xFF;
            pkt[5] = value & 0xFF;
            uint16_t ck = calcChecksum(pkt, 6);
            pkt[6] = (ck >> 8) & 0xFF;
            pkt[7] = ck & 0xFF;
            uart_->write_array(pkt, 8);
            ESP_LOGV(CTRL_TAG, "TX addr=0x%04X val=0x%04X", addr, value);
        }

        void sendRead(uint16_t addr) {
            uint8_t pkt[6];
            pkt[0] = 0x40;
            pkt[1] = 0x04;
            pkt[2] = (addr >> 8) & 0xFF;
            pkt[3] = addr & 0xFF;
            uint16_t ck = calcChecksum(pkt, 4);
            pkt[4] = (ck >> 8) & 0xFF;
            pkt[5] = ck & 0xFF;
            uart_->write_array(pkt, 6);
        }

        // ── 狀態機：定期輪詢 ─────────────────────────────────
        void sendPending() {
            uint32_t now = esphome::millis();
            if (now - last_send_ < 400) return;
            last_send_ = now;

            if (pending_count_ > 0) {
                auto &pw = pending_[0];
                sendPacket((uint16_t)pw.addr, pw.value);
                // shift remaining
                for (int i = 1; i < pending_count_; i++) pending_[i-1] = pending_[i];
                pending_count_--;
                return;
            }

            // 初始化階段
            if (!init_done_) {
                static const uint16_t INIT_ADDRS_1[] = {
                    (uint16_t)Address::Initial0, (uint16_t)Address::Initial1
                };
                static const uint16_t INIT_ADDRS_2[] = {
                    (uint16_t)Address::Initial2,  (uint16_t)Address::Initial3,
                    (uint16_t)Address::Initial4,  (uint16_t)Address::Initial5,
                    (uint16_t)Address::Initial6,  (uint16_t)Address::Initial7,
                    (uint16_t)Address::Initial8,  (uint16_t)Address::Initial9,
                    (uint16_t)Address::Initial10, (uint16_t)Address::Initial11,
                    (uint16_t)Address::Initial12, (uint16_t)Address::Initial13,
                    (uint16_t)Address::HorizontalSwingSupported,
                    (uint16_t)Address::Initial15,
                };
                static const uint16_t INIT_ADDRS_3[] = {
                    (uint16_t)Address::Initial16, (uint16_t)Address::Initial17,
                    (uint16_t)Address::HumanSensorSupported,
                    (uint16_t)Address::Initial19, (uint16_t)Address::Initial20,
                    (uint16_t)Address::Initial21, (uint16_t)Address::Initial22,
                    (uint16_t)Address::Initial23, (uint16_t)Address::Initial24,
                    (uint16_t)Address::Initial25,
                };
                static int init_step = 0;
                static int init_sub  = 0;

                if (init_step == 0) {
                    if (init_sub < 2) sendRead(INIT_ADDRS_1[init_sub++]);
                    else { init_step = 1; init_sub = 0; }
                } else if (init_step == 1) {
                    if (init_sub < 14) sendRead(INIT_ADDRS_2[init_sub++]);
                    else { init_step = 2; init_sub = 0; }
                } else if (init_step == 2) {
                    if (init_sub < 10) sendRead(INIT_ADDRS_3[init_sub++]);
                    else { init_done_ = true; }
                }
                return;
            }

            // 日常輪詢（FrameA / B / C 循環）
            static const uint16_t FRAME_A[] = {
                (uint16_t)Address::Power,  (uint16_t)Address::Mode,
                (uint16_t)Address::SetpointTemp, (uint16_t)Address::FanSpeed,
                (uint16_t)Address::VerticalSwing, (uint16_t)Address::VerticalAirflow,
                (uint16_t)Address::HorizontalSwing, (uint16_t)Address::HorizontalAirflow,
                (uint16_t)Address::ActualTemp,
            };
            static const uint16_t FRAME_B[] = {
                (uint16_t)Address::EconomyMode, (uint16_t)Address::Powerful,
                (uint16_t)Address::MinimumHeat, (uint16_t)Address::EnergySavingFan,
                (uint16_t)Address::OutdoorUnitLowNoise,
            };
            static const uint16_t FRAME_C[] = {
                (uint16_t)Address::OutdoorTemp,
            };

            static int poll_step = 0;
            static int poll_sub  = 0;

            switch (poll_step) {
                case 0:
                    if (poll_sub < 9) sendRead(FRAME_A[poll_sub++]);
                    else { poll_step = 1; poll_sub = 0; }
                    break;
                case 1:
                    if (poll_sub < 5) sendRead(FRAME_B[poll_sub++]);
                    else { poll_step = 2; poll_sub = 0; }
                    break;
                case 2:
                    if (poll_sub < 1) sendRead(FRAME_C[poll_sub++]);
                    else { poll_step = 0; poll_sub = 0; }
                    break;
            }
        }

        // ── 接收解析 ─────────────────────────────────────────
        void readAndProcess() {
            while (uart_->available()) {
                uint8_t b;
                uart_->read_byte(&b);
                // 新封包起始 0x40
                if (b == 0x40 && rx_len_ == 0) {
                    rx_buf_[rx_len_++] = b;
                } else if (rx_len_ > 0) {
                    rx_buf_[rx_len_++] = b;
                    if (rx_len_ >= 8) {
                        processFrame(rx_buf_, rx_len_);
                        rx_len_ = 0;
                    }
                }
            }
        }

        void processFrame(uint8_t *buf, int len) {
            if (len < 8) return;
            // 驗證 checksum
            uint16_t expected = calcChecksum(buf, len - 2);
            uint16_t actual = ((uint16_t)buf[len-2] << 8) | buf[len-1];
            if (expected != actual) {
                ESP_LOGW(CTRL_TAG, "Checksum mismatch: expect 0x%04X got 0x%04X", expected, actual);
                return;
            }
            uint16_t addr = ((uint16_t)buf[2] << 8) | buf[3];
            uint16_t val  = ((uint16_t)buf[4] << 8) | buf[5];
            ESP_LOGV(CTRL_TAG, "RX addr=0x%04X val=0x%04X", addr, val);

            AcRegister *reg = registry_.getRegister((Address)addr);
            if (reg->value != val) {
                reg->value = val;
                if (on_register_change_) on_register_change_(reg);
            }
        }

        void pendingWrite(Address addr, uint16_t value) {
            if (pending_count_ < MAX_PENDING) {
                pending_[pending_count_++] = {addr, value};
            }
        }
    };

}  // namespace fujitsu_ac_impl

// =============================================================================
// ESPHome Climate 主類別（原 fujitsu_ac.h）
// =============================================================================
namespace esphome {
namespace fujitsu_ac {

static const char *const TAG = "fujitsu_ac";

#ifdef USE_SELECT
class FujitsuVerticalVaneSelect : public select::Select {
public:
    void set_controller(::fujitsu_ac_impl::FujitsuController *ctrl) { ctrl_ = ctrl; }
    void control(const std::string &value) override {
        if (!ctrl_) return;
        if (value == "Swing") { ctrl_->setVerticalSwing(::fujitsu_ac_impl::VerticalSwing::On); return; }
        static const std::pair<const char*, ::fujitsu_ac_impl::VerticalAirflow> MAP[] = {
            {"Position 1",::fujitsu_ac_impl::VerticalAirflow::Position1},
            {"Position 2",::fujitsu_ac_impl::VerticalAirflow::Position2},
            {"Position 3",::fujitsu_ac_impl::VerticalAirflow::Position3},
            {"Position 4",::fujitsu_ac_impl::VerticalAirflow::Position4},
            {"Position 5",::fujitsu_ac_impl::VerticalAirflow::Position5},
            {"Position 6",::fujitsu_ac_impl::VerticalAirflow::Position6},
        };
        for (auto &m : MAP) if (value == m.first) { ctrl_->setVerticalAirflow(m.second); return; }
    }
    void update_from_reg(uint16_t vs_val, uint16_t va_val) {
        if ((uint16_t)::fujitsu_ac_impl::VerticalSwing::On == vs_val) { publish_state("Swing"); return; }
        static const std::pair<uint16_t,const char*> MAP[] = {
            {(uint16_t)::fujitsu_ac_impl::VerticalAirflow::Position1,"Position 1"},
            {(uint16_t)::fujitsu_ac_impl::VerticalAirflow::Position2,"Position 2"},
            {(uint16_t)::fujitsu_ac_impl::VerticalAirflow::Position3,"Position 3"},
            {(uint16_t)::fujitsu_ac_impl::VerticalAirflow::Position4,"Position 4"},
            {(uint16_t)::fujitsu_ac_impl::VerticalAirflow::Position5,"Position 5"},
            {(uint16_t)::fujitsu_ac_impl::VerticalAirflow::Position6,"Position 6"},
        };
        for (auto &m : MAP) if (va_val == m.first) { publish_state(m.second); return; }
    }
private:
    ::fujitsu_ac_impl::FujitsuController *ctrl_ = nullptr;
};

class FujitsuHorizontalVaneSelect : public select::Select {
public:
    void set_controller(::fujitsu_ac_impl::FujitsuController *ctrl) { ctrl_ = ctrl; }
    void control(const std::string &value) override {
        if (!ctrl_) return;
        if (value == "Swing") { ctrl_->setHorizontalSwing(::fujitsu_ac_impl::HorizontalSwing::On); return; }
        static const std::pair<const char*, ::fujitsu_ac_impl::HorizontalAirflow> MAP[] = {
            {"Position 1",::fujitsu_ac_impl::HorizontalAirflow::Position1},
            {"Position 2",::fujitsu_ac_impl::HorizontalAirflow::Position2},
            {"Position 3",::fujitsu_ac_impl::HorizontalAirflow::Position3},
            {"Position 4",::fujitsu_ac_impl::HorizontalAirflow::Position4},
            {"Position 5",::fujitsu_ac_impl::HorizontalAirflow::Position5},
            {"Position 6",::fujitsu_ac_impl::HorizontalAirflow::Position6},
        };
        for (auto &m : MAP) if (value == m.first) { ctrl_->setHorizontalAirflow(m.second); return; }
    }
    void update_from_reg(uint16_t hs_val, uint16_t ha_val) {
        if ((uint16_t)::fujitsu_ac_impl::HorizontalSwing::On == hs_val) { publish_state("Swing"); return; }
        static const std::pair<uint16_t,const char*> MAP[] = {
            {(uint16_t)::fujitsu_ac_impl::HorizontalAirflow::Position1,"Position 1"},
            {(uint16_t)::fujitsu_ac_impl::HorizontalAirflow::Position2,"Position 2"},
            {(uint16_t)::fujitsu_ac_impl::HorizontalAirflow::Position3,"Position 3"},
            {(uint16_t)::fujitsu_ac_impl::HorizontalAirflow::Position4,"Position 4"},
            {(uint16_t)::fujitsu_ac_impl::HorizontalAirflow::Position5,"Position 5"},
            {(uint16_t)::fujitsu_ac_impl::HorizontalAirflow::Position6,"Position 6"},
        };
        for (auto &m : MAP) if (ha_val == m.first) { publish_state(m.second); return; }
    }
private:
    ::fujitsu_ac_impl::FujitsuController *ctrl_ = nullptr;
};
#endif  // USE_SELECT

class FujitsuAcClimate : public climate::Climate, public Component, public uart::UARTDevice {
public:
    explicit FujitsuAcClimate() : controller_(nullptr) {}

    void setup() override {
        controller_ = new ::fujitsu_ac_impl::FujitsuController(this->parent_);
        controller_->setOnRegisterChangeCallback([this](const ::fujitsu_ac_impl::AcRegister *reg) {
            this->on_register_change(reg);
        });
        controller_->setup();
        ESP_LOGCONFIG(TAG, "FujitsuAC: 初始化完成");
    }

    void loop() override { controller_->loop(); }

    float get_setup_priority() const override { return setup_priority::DATA; }

    void dump_config() override {
        ESP_LOGCONFIG(TAG, "Fujitsu AC Climate:");
        LOG_CLIMATE("  ", "Climate", this);
    }

#ifdef USE_SENSOR
    void set_indoor_temperature_sensor(sensor::Sensor *s)  { indoor_temp_sensor_  = s; }
    void set_outdoor_temperature_sensor(sensor::Sensor *s) { outdoor_temp_sensor_ = s; }
#endif
#ifdef USE_SELECT
    void set_vertical_vane_select(FujitsuVerticalVaneSelect *s) {
        vertical_vane_select_ = s;
        if (s && controller_) s->set_controller(controller_);
    }
    void set_horizontal_vane_select(FujitsuHorizontalVaneSelect *s) {
        horizontal_vane_select_ = s;
        if (s && controller_) s->set_controller(controller_);
    }
#endif

protected:
    void control(const climate::ClimateCall &call) override {
        if (!controller_) return;
        if (call.get_mode().has_value()) {
            auto m = *call.get_mode();
            if (m == climate::CLIMATE_MODE_OFF) {
                controller_->setPower(::fujitsu_ac_impl::Power::Off);
            } else {
                controller_->setPower(::fujitsu_ac_impl::Power::On);
                switch (m) {
                    case climate::CLIMATE_MODE_COOL:      controller_->setMode(::fujitsu_ac_impl::Mode::Cool); break;
                    case climate::CLIMATE_MODE_HEAT:      controller_->setMode(::fujitsu_ac_impl::Mode::Heat); break;
                    case climate::CLIMATE_MODE_DRY:       controller_->setMode(::fujitsu_ac_impl::Mode::Dry);  break;
                    case climate::CLIMATE_MODE_FAN_ONLY:  controller_->setMode(::fujitsu_ac_impl::Mode::Fan);  break;
                    case climate::CLIMATE_MODE_HEAT_COOL: controller_->setMode(::fujitsu_ac_impl::Mode::Auto); break;
                    default: break;
                }
            }
            // 樂觀更新：立即反映模式到 HA，不等冷氣回應
            this->mode = m;
        }
        if (call.get_target_temperature().has_value()) {
            controller_->setTemp(*call.get_target_temperature());
            // 樂觀更新：立即反映目標溫度到 HA
            this->target_temperature = *call.get_target_temperature();
        }
        if (call.get_fan_mode().has_value()) {
            switch (*call.get_fan_mode()) {
                case climate::CLIMATE_FAN_AUTO:   controller_->setFanSpeed(::fujitsu_ac_impl::FanSpeed::Auto);   break;
                case climate::CLIMATE_FAN_LOW:    controller_->setFanSpeed(::fujitsu_ac_impl::FanSpeed::Low);    break;
                case climate::CLIMATE_FAN_MEDIUM: controller_->setFanSpeed(::fujitsu_ac_impl::FanSpeed::Medium); break;
                case climate::CLIMATE_FAN_HIGH:   controller_->setFanSpeed(::fujitsu_ac_impl::FanSpeed::High);   break;
                case climate::CLIMATE_FAN_QUIET:  controller_->setFanSpeed(::fujitsu_ac_impl::FanSpeed::Quiet);  break;
                default: break;
            }
            // 樂觀更新：立即反映風速到 HA
            this->set_fan_mode_(*call.get_fan_mode());
        }
        // 樂觀更新：立即發布狀態到 HA（不等冷氣回應確認）
        // 備註：若冷氣有連接，後續冷氣回應會再次 publish_state() 以確保同步
        this->publish_state();
    }

    climate::ClimateTraits traits() override {
        auto t = climate::ClimateTraits();
        t.set_supports_current_temperature(true);
        t.set_supported_modes({
            climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT_COOL,
            climate::CLIMATE_MODE_COOL, climate::CLIMATE_MODE_HEAT,
            climate::CLIMATE_MODE_DRY, climate::CLIMATE_MODE_FAN_ONLY,
        });
        t.set_supported_fan_modes({
            climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW,
            climate::CLIMATE_FAN_MEDIUM, climate::CLIMATE_FAN_HIGH,
            climate::CLIMATE_FAN_QUIET,
        });
        t.set_visual_min_temperature(16.0f);
        t.set_visual_max_temperature(30.0f);
        t.set_visual_temperature_step(0.5f);
        t.set_visual_current_temperature_step(0.1f);
        return t;
    }

private:
    ::fujitsu_ac_impl::FujitsuController *controller_ = nullptr;
    bool first_update_ = true;
#ifdef USE_SENSOR
    sensor::Sensor *indoor_temp_sensor_  = nullptr;
    sensor::Sensor *outdoor_temp_sensor_ = nullptr;
#endif
#ifdef USE_SELECT
    FujitsuVerticalVaneSelect   *vertical_vane_select_   = nullptr;
    FujitsuHorizontalVaneSelect *horizontal_vane_select_ = nullptr;
#endif

    void on_register_change(const ::fujitsu_ac_impl::AcRegister *reg) {
        bool publish = false;
        switch (reg->address) {
            case ::fujitsu_ac_impl::Address::Power:
            case ::fujitsu_ac_impl::Address::Mode: {
                auto *pw = controller_->getRegister(::fujitsu_ac_impl::Address::Power);
                auto *md = controller_->getRegister(::fujitsu_ac_impl::Address::Mode);
                if ((uint16_t)::fujitsu_ac_impl::Power::Off == pw->value) {
                    this->mode = climate::CLIMATE_MODE_OFF;
                } else {
                    switch ((::fujitsu_ac_impl::Mode)md->value) {
                        case ::fujitsu_ac_impl::Mode::Cool: this->mode = climate::CLIMATE_MODE_COOL; break;
                        case ::fujitsu_ac_impl::Mode::Heat: this->mode = climate::CLIMATE_MODE_HEAT; break;
                        case ::fujitsu_ac_impl::Mode::Dry:  this->mode = climate::CLIMATE_MODE_DRY;  break;
                        case ::fujitsu_ac_impl::Mode::Fan:  this->mode = climate::CLIMATE_MODE_FAN_ONLY; break;
                        case ::fujitsu_ac_impl::Mode::Auto: this->mode = climate::CLIMATE_MODE_HEAT_COOL; break;
                        default: break;
                    }
                }
                publish = true; break;
            }
            case ::fujitsu_ac_impl::Address::SetpointTemp:
                this->target_temperature = reg->value / 10.0f; publish = true; break;
            case ::fujitsu_ac_impl::Address::FanSpeed:
                switch ((::fujitsu_ac_impl::FanSpeed)reg->value) {
                    case ::fujitsu_ac_impl::FanSpeed::Auto:   this->set_fan_mode_(climate::CLIMATE_FAN_AUTO);   break;
                    case ::fujitsu_ac_impl::FanSpeed::Low:    this->set_fan_mode_(climate::CLIMATE_FAN_LOW);    break;
                    case ::fujitsu_ac_impl::FanSpeed::Medium: this->set_fan_mode_(climate::CLIMATE_FAN_MEDIUM); break;
                    case ::fujitsu_ac_impl::FanSpeed::High:   this->set_fan_mode_(climate::CLIMATE_FAN_HIGH);   break;
                    case ::fujitsu_ac_impl::FanSpeed::Quiet:  this->set_fan_mode_(climate::CLIMATE_FAN_QUIET);  break;
                    default: break;
                }
                publish = true; break;
            case ::fujitsu_ac_impl::Address::ActualTemp: {
                float t = reg->value / 10.0f;
                this->current_temperature = t;
#ifdef USE_SENSOR
                if (indoor_temp_sensor_) indoor_temp_sensor_->publish_state(t);
#endif
                publish = true; break;
            }
            case ::fujitsu_ac_impl::Address::OutdoorTemp:
#ifdef USE_SENSOR
                if (outdoor_temp_sensor_) outdoor_temp_sensor_->publish_state(reg->value / 10.0f);
#endif
                break;
            case ::fujitsu_ac_impl::Address::VerticalSwing:
            case ::fujitsu_ac_impl::Address::VerticalAirflow:
#ifdef USE_SELECT
                if (vertical_vane_select_) {
                    auto *vs = controller_->getRegister(::fujitsu_ac_impl::Address::VerticalSwing);
                    auto *va = controller_->getRegister(::fujitsu_ac_impl::Address::VerticalAirflow);
                    vertical_vane_select_->update_from_reg(vs->value, va->value);
                }
#endif
                break;
            case ::fujitsu_ac_impl::Address::HorizontalSwing:
            case ::fujitsu_ac_impl::Address::HorizontalAirflow:
#ifdef USE_SELECT
                if (horizontal_vane_select_) {
                    auto *hs = controller_->getRegister(::fujitsu_ac_impl::Address::HorizontalSwing);
                    auto *ha = controller_->getRegister(::fujitsu_ac_impl::Address::HorizontalAirflow);
                    horizontal_vane_select_->update_from_reg(hs->value, ha->value);
                }
#endif
                break;
            default: break;
        }
        if (publish) { this->publish_state(); first_update_ = false; }
    }
};

}  // namespace fujitsu_ac
}  // namespace esphome
