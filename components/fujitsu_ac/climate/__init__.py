import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import uart, climate, sensor, select
from .. import fujitsu_ac_ns

# ── 類別宣告 ────────────────────────────────────────────────
FujitsuAcClimate = fujitsu_ac_ns.class_(
    "FujitsuAcClimate", climate.Climate, cg.Component, uart.UARTDevice
)
FujitsuVerticalVaneSelect = fujitsu_ac_ns.class_(
    "FujitsuVerticalVaneSelect", select.Select
)
FujitsuHorizontalVaneSelect = fujitsu_ac_ns.class_(
    "FujitsuHorizontalVaneSelect", select.Select
)

# ── DEPENDENCIES ──────────────────────────────────────────────
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "select"]
CODEOWNERS = ["@local"]

# ── config key 常數 ──────────────────────────────────────────
CONF_INDOOR_TEMPERATURE     = "indoor_temperature_sensor"
CONF_OUTDOOR_TEMPERATURE    = "outdoor_temperature_sensor"
CONF_VERTICAL_VANE_SELECT   = "vertical_vane_select"
CONF_HORIZONTAL_VANE_SELECT = "horizontal_vane_select"

# ── CONFIG_SCHEMA ─────────────────────────────────────────────
CONFIG_SCHEMA = cv.All(
    climate.climate_schema(FujitsuAcClimate).extend(
        {
            cv.GenerateID(): cv.declare_id(FujitsuAcClimate),
            # 可選感測器
            cv.Optional(CONF_INDOOR_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement="°C",
                accuracy_decimals=1,
                icon="mdi:thermometer",
            ),
            cv.Optional(CONF_OUTDOOR_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement="°C",
                accuracy_decimals=1,
                icon="mdi:thermometer-chevron-up",
            ),
            # 可選選擇器
            cv.Optional(CONF_VERTICAL_VANE_SELECT): select.select_schema(
                FujitsuVerticalVaneSelect,
            ),
            cv.Optional(CONF_HORIZONTAL_VANE_SELECT): select.select_schema(
                FujitsuHorizontalVaneSelect,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    await uart.register_uart_device(var, config)

    # 室內溫度感測器
    if CONF_INDOOR_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_INDOOR_TEMPERATURE])
        cg.add(var.set_indoor_temperature_sensor(sens))

    # 戶外溫度感測器
    if CONF_OUTDOOR_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_OUTDOOR_TEMPERATURE])
        cg.add(var.set_outdoor_temperature_sensor(sens))

    # 垂直風向選擇器
    if CONF_VERTICAL_VANE_SELECT in config:
        sel = await select.new_select(
            config[CONF_VERTICAL_VANE_SELECT],
            options=["Position 1", "Position 2", "Position 3",
                     "Position 4", "Position 5", "Position 6", "Swing"],
        )
        cg.add(var.set_vertical_vane_select(sel))

    # 水平風向選擇器
    if CONF_HORIZONTAL_VANE_SELECT in config:
        sel = await select.new_select(
            config[CONF_HORIZONTAL_VANE_SELECT],
            options=["Position 1", "Position 2", "Position 3",
                     "Position 4", "Position 5", "Position 6", "Swing"],
        )
        cg.add(var.set_horizontal_vane_select(sel))
