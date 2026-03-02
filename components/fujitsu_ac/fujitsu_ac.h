#pragma once

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

#include "fujitsu_controller.h"
#include "fujitsu_enums.h"
#include "fujitsu_register.h"

namespace esphome {
namespace fujitsu_ac {

static const char *const TAG = "fujitsu_ac";

// =============================================================================
// Select: 垂直風向
// =============================================================================
#ifdef USE_SELECT
class FujitsuVerticalVaneSelect : public select::Select {
public:
    void set_controller(::fujitsu_ac::FujitsuController *ctrl) { ctrl_ = ctrl; }

    void control(const std::string &value) override {
        if (!ctrl_) return;
        if (value == "Swing") {
            ctrl_->setVerticalSwing(::fujitsu_ac::VerticalSwing::On);
        } else {
            static const std::pair<const char *, ::fujitsu_ac::VerticalAirflow> MAP[] = {
                {"Position 1", ::fujitsu_ac::VerticalAirflow::Position1},
                {"Position 2", ::fujitsu_ac::VerticalAirflow::Position2},
                {"Position 3", ::fujitsu_ac::VerticalAirflow::Position3},
                {"Position 4", ::fujitsu_ac::VerticalAirflow::Position4},
                {"Position 5", ::fujitsu_ac::VerticalAirflow::Position5},
                {"Position 6", ::fujitsu_ac::VerticalAirflow::Position6},
            };
            for (auto &m : MAP) {
                if (value == m.first) { ctrl_->setVerticalAirflow(m.second); return; }
            }
        }
    }

    void update_from_reg(uint16_t vs_val, uint16_t va_val) {
        if ((uint16_t)::fujitsu_ac::VerticalSwing::On == vs_val) {
            publish_state("Swing");
            return;
        }
        static const std::pair<uint16_t, const char *> MAP[] = {
            {(uint16_t)::fujitsu_ac::VerticalAirflow::Position1, "Position 1"},
            {(uint16_t)::fujitsu_ac::VerticalAirflow::Position2, "Position 2"},
            {(uint16_t)::fujitsu_ac::VerticalAirflow::Position3, "Position 3"},
            {(uint16_t)::fujitsu_ac::VerticalAirflow::Position4, "Position 4"},
            {(uint16_t)::fujitsu_ac::VerticalAirflow::Position5, "Position 5"},
            {(uint16_t)::fujitsu_ac::VerticalAirflow::Position6, "Position 6"},
        };
        for (auto &m : MAP) {
            if (va_val == m.first) { publish_state(m.second); return; }
        }
    }

private:
    ::fujitsu_ac::FujitsuController *ctrl_ = nullptr;
};

// =============================================================================
// Select: 水平風向
// =============================================================================
class FujitsuHorizontalVaneSelect : public select::Select {
public:
    void set_controller(::fujitsu_ac::FujitsuController *ctrl) { ctrl_ = ctrl; }

    void control(const std::string &value) override {
        if (!ctrl_) return;
        if (value == "Swing") {
            ctrl_->setHorizontalSwing(::fujitsu_ac::HorizontalSwing::On);
        } else {
            static const std::pair<const char *, ::fujitsu_ac::HorizontalAirflow> MAP[] = {
                {"Position 1", ::fujitsu_ac::HorizontalAirflow::Position1},
                {"Position 2", ::fujitsu_ac::HorizontalAirflow::Position2},
                {"Position 3", ::fujitsu_ac::HorizontalAirflow::Position3},
                {"Position 4", ::fujitsu_ac::HorizontalAirflow::Position4},
                {"Position 5", ::fujitsu_ac::HorizontalAirflow::Position5},
                {"Position 6", ::fujitsu_ac::HorizontalAirflow::Position6},
            };
            for (auto &m : MAP) {
                if (value == m.first) { ctrl_->setHorizontalAirflow(m.second); return; }
            }
        }
    }

    void update_from_reg(uint16_t hs_val, uint16_t ha_val) {
        if ((uint16_t)::fujitsu_ac::HorizontalSwing::On == hs_val) {
            publish_state("Swing");
            return;
        }
        static const std::pair<uint16_t, const char *> MAP[] = {
            {(uint16_t)::fujitsu_ac::HorizontalAirflow::Position1, "Position 1"},
            {(uint16_t)::fujitsu_ac::HorizontalAirflow::Position2, "Position 2"},
            {(uint16_t)::fujitsu_ac::HorizontalAirflow::Position3, "Position 3"},
            {(uint16_t)::fujitsu_ac::HorizontalAirflow::Position4, "Position 4"},
            {(uint16_t)::fujitsu_ac::HorizontalAirflow::Position5, "Position 5"},
            {(uint16_t)::fujitsu_ac::HorizontalAirflow::Position6, "Position 6"},
        };
        for (auto &m : MAP) {
            if (ha_val == m.first) { publish_state(m.second); return; }
        }
    }

private:
    ::fujitsu_ac::FujitsuController *ctrl_ = nullptr;
};
#endif  // USE_SELECT

// =============================================================================
// 主 Climate 類別
// =============================================================================
class FujitsuAcClimate : public climate::Climate, public Component, public uart::UARTDevice {
public:
    explicit FujitsuAcClimate() : controller_(nullptr) {}

    // ---- ESPHome 生命週期 ----
    void setup() override {
        controller_ = new ::fujitsu_ac::FujitsuController(this->parent_);
        controller_->setOnRegisterChangeCallback([this](const ::fujitsu_ac::AcRegister *reg) {
            this->on_register_change(reg);
        });
        controller_->setup();
        ESP_LOGCONFIG(TAG, "FujitsuAC: 初始化完成");
    }

    void loop() override {
        controller_->loop();
    }

    float get_setup_priority() const override { return setup_priority::DATA; }

    void dump_config() override {
        ESP_LOGCONFIG(TAG, "Fujitsu AC Climate:");
        LOG_CLIMATE("  ", "Climate", this);
    }

    // ---- 子元件 setter（由 __init__.py 產生的 codegen 呼叫）----
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
    // ---- Climate 介面 ----
    void control(const climate::ClimateCall &call) override {
        if (!controller_) return;

        if (call.get_mode().has_value()) {
            auto m = *call.get_mode();
            if (m == climate::CLIMATE_MODE_OFF) {
                controller_->setPower(::fujitsu_ac::Power::Off);
            } else {
                controller_->setPower(::fujitsu_ac::Power::On);
                switch (m) {
                    case climate::CLIMATE_MODE_COOL:      controller_->setMode(::fujitsu_ac::Mode::Cool); break;
                    case climate::CLIMATE_MODE_HEAT:      controller_->setMode(::fujitsu_ac::Mode::Heat); break;
                    case climate::CLIMATE_MODE_DRY:       controller_->setMode(::fujitsu_ac::Mode::Dry);  break;
                    case climate::CLIMATE_MODE_FAN_ONLY:  controller_->setMode(::fujitsu_ac::Mode::Fan);  break;
                    case climate::CLIMATE_MODE_HEAT_COOL: controller_->setMode(::fujitsu_ac::Mode::Auto); break;
                    default: break;
                }
            }
        }

        if (call.get_target_temperature().has_value()) {
            controller_->setTemp(*call.get_target_temperature());
        }

        if (call.get_fan_mode().has_value()) {
            switch (*call.get_fan_mode()) {
                case climate::CLIMATE_FAN_AUTO:   controller_->setFanSpeed(::fujitsu_ac::FanSpeed::Auto);   break;
                case climate::CLIMATE_FAN_LOW:    controller_->setFanSpeed(::fujitsu_ac::FanSpeed::Low);    break;
                case climate::CLIMATE_FAN_MEDIUM: controller_->setFanSpeed(::fujitsu_ac::FanSpeed::Medium); break;
                case climate::CLIMATE_FAN_HIGH:   controller_->setFanSpeed(::fujitsu_ac::FanSpeed::High);   break;
                default: break;
            }
        }

        if (call.has_custom_fan_mode()) {
            if (*call.get_custom_fan_mode() == "Quiet") {
                controller_->setFanSpeed(::fujitsu_ac::FanSpeed::Quiet);
            }
        }
    }

    climate::ClimateTraits traits() override {
        auto t = climate::ClimateTraits();
        t.set_supports_current_temperature(true);
        t.set_supported_modes({
            climate::CLIMATE_MODE_OFF,
            climate::CLIMATE_MODE_HEAT_COOL,
            climate::CLIMATE_MODE_COOL,
            climate::CLIMATE_MODE_HEAT,
            climate::CLIMATE_MODE_DRY,
            climate::CLIMATE_MODE_FAN_ONLY,
        });
        t.set_supported_fan_modes({
            climate::CLIMATE_FAN_AUTO,
            climate::CLIMATE_FAN_LOW,
            climate::CLIMATE_FAN_MEDIUM,
            climate::CLIMATE_FAN_HIGH,
        });
        t.add_supported_custom_fan_mode("Quiet");
        t.set_visual_min_temperature(16.0f);
        t.set_visual_max_temperature(30.0f);
        t.set_visual_temperature_step(0.5f);
        t.set_visual_current_temperature_step(0.1f);
        return t;
    }

private:
    ::fujitsu_ac::FujitsuController *controller_ = nullptr;
    bool first_update_ = true;

#ifdef USE_SENSOR
    sensor::Sensor *indoor_temp_sensor_  = nullptr;
    sensor::Sensor *outdoor_temp_sensor_ = nullptr;
#endif
#ifdef USE_SELECT
    FujitsuVerticalVaneSelect   *vertical_vane_select_   = nullptr;
    FujitsuHorizontalVaneSelect *horizontal_vane_select_ = nullptr;
#endif

    void on_register_change(const ::fujitsu_ac::AcRegister *reg) {
        bool publish = false;

        switch (reg->address) {
            case ::fujitsu_ac::Address::Power:
            case ::fujitsu_ac::Address::Mode: {
                auto *pw = controller_->getRegister(::fujitsu_ac::Address::Power);
                auto *md = controller_->getRegister(::fujitsu_ac::Address::Mode);
                if ((uint16_t)::fujitsu_ac::Power::Off == pw->value) {
                    this->mode = climate::CLIMATE_MODE_OFF;
                } else {
                    switch ((::fujitsu_ac::Mode)md->value) {
                        case ::fujitsu_ac::Mode::Cool: this->mode = climate::CLIMATE_MODE_COOL; break;
                        case ::fujitsu_ac::Mode::Heat: this->mode = climate::CLIMATE_MODE_HEAT; break;
                        case ::fujitsu_ac::Mode::Dry:  this->mode = climate::CLIMATE_MODE_DRY;  break;
                        case ::fujitsu_ac::Mode::Fan:  this->mode = climate::CLIMATE_MODE_FAN_ONLY; break;
                        case ::fujitsu_ac::Mode::Auto: this->mode = climate::CLIMATE_MODE_HEAT_COOL; break;
                        default: break;
                    }
                }
                publish = true;
                break;
            }

            case ::fujitsu_ac::Address::SetpointTemp: {
                this->target_temperature = reg->value / 10.0f;
                publish = true;
                break;
            }

            case ::fujitsu_ac::Address::FanSpeed: {
                switch ((::fujitsu_ac::FanSpeed)reg->value) {
                    case ::fujitsu_ac::FanSpeed::Auto:   this->set_fan_mode_(climate::CLIMATE_FAN_AUTO);   break;
                    case ::fujitsu_ac::FanSpeed::Low:    this->set_fan_mode_(climate::CLIMATE_FAN_LOW);    break;
                    case ::fujitsu_ac::FanSpeed::Medium: this->set_fan_mode_(climate::CLIMATE_FAN_MEDIUM); break;
                    case ::fujitsu_ac::FanSpeed::High:   this->set_fan_mode_(climate::CLIMATE_FAN_HIGH);   break;
                    case ::fujitsu_ac::FanSpeed::Quiet:  this->set_custom_fan_mode_("Quiet"); break;
                    default: break;
                }
                publish = true;
                break;
            }

            case ::fujitsu_ac::Address::ActualTemp: {
                float t = reg->value / 10.0f;
                this->current_temperature = t;
#ifdef USE_SENSOR
                if (indoor_temp_sensor_) indoor_temp_sensor_->publish_state(t);
#endif
                publish = true;
                break;
            }

            case ::fujitsu_ac::Address::OutdoorTemp: {
#ifdef USE_SENSOR
                if (outdoor_temp_sensor_) {
                    outdoor_temp_sensor_->publish_state(reg->value / 10.0f);
                }
#endif
                break;
            }

            case ::fujitsu_ac::Address::VerticalSwing:
            case ::fujitsu_ac::Address::VerticalAirflow: {
#ifdef USE_SELECT
                if (vertical_vane_select_) {
                    auto *vs = controller_->getRegister(::fujitsu_ac::Address::VerticalSwing);
                    auto *va = controller_->getRegister(::fujitsu_ac::Address::VerticalAirflow);
                    vertical_vane_select_->update_from_reg(vs->value, va->value);
                }
#endif
                break;
            }

            case ::fujitsu_ac::Address::HorizontalSwing:
            case ::fujitsu_ac::Address::HorizontalAirflow: {
#ifdef USE_SELECT
                if (horizontal_vane_select_) {
                    auto *hs = controller_->getRegister(::fujitsu_ac::Address::HorizontalSwing);
                    auto *ha = controller_->getRegister(::fujitsu_ac::Address::HorizontalAirflow);
                    horizontal_vane_select_->update_from_reg(hs->value, ha->value);
                }
#endif
                break;
            }

            default: break;
        }

        if (publish) {
            this->publish_state();
            first_update_ = false;
        }
    }
};

}  // namespace fujitsu_ac
}  // namespace esphome
