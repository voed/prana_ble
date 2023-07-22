import logging
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_CO2,
    CONF_FREQUENCY,
    CONF_HUMIDITY,
    CONF_ID,
    CONF_PRESSURE,
    CONF_TEMPERATURE,
    CONF_TVOC,
    CONF_VOLTAGE,
    DEVICE_CLASS_CARBON_DIOXIDE,
    DEVICE_CLASS_FREQUENCY,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_PRESSURE,
    DEVICE_CLASS_VOLATILE_ORGANIC_COMPOUNDS,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_HERTZ,
    UNIT_PARTS_PER_BILLION,
    UNIT_PARTS_PER_MILLION,
    UNIT_PERCENT,
    UNIT_VOLT,
    UNIT_HECTOPASCAL,
)

from .. import (
    PRANA_BLE_CLIENT_SCHEMA,
    prana_ble_ns,
    register_bedjet_child,
)

_LOGGER = logging.getLogger(__name__)
CODEOWNERS = ["@voed"]
DEPENDENCIES = ["prana_ble"]
CONF_INLET_TEMP = "temperature_in"
CONF_OUTLET_TEMP = "temperature_out"


PranaBLESensor = prana_ble_ns.class_("PranaBLESensors", cg.PollingComponent)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PranaBLESensor),
            cv.Optional(CONF_INLET_TEMP): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_OUTLET_TEMP): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_HUMIDITY): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_HUMIDITY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PRESSURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_HECTOPASCAL,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_PRESSURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CO2): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_CARBON_DIOXIDE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TVOC): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_BILLION,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FREQUENCY): sensor.sensor_schema(
                unit_of_measurement=UNIT_HERTZ,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_FREQUENCY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(PRANA_BLE_CLIENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await register_bedjet_child(var, config)

    if CONF_INLET_TEMP in config:
        sens = await sensor.new_sensor(config[CONF_INLET_TEMP])
        cg.add(var.set_temp_in(sens))

    if CONF_OUTLET_TEMP in config:
        sens = await sensor.new_sensor(config[CONF_OUTLET_TEMP])
        cg.add(var.set_temp_out(sens))

    if CONF_HUMIDITY in config:
        sens = await sensor.new_sensor(config[CONF_HUMIDITY])
        cg.add(var.set_humidity(sens))

    if CONF_PRESSURE in config:
        sens = await sensor.new_sensor(config[CONF_PRESSURE])
        cg.add(var.set_pressure(sens))

    if CONF_CO2 in config:
        sens = await sensor.new_sensor(config[CONF_CO2])
        cg.add(var.set_co2(sens))

    if CONF_TVOC in config:
        sens = await sensor.new_sensor(config[CONF_TVOC])
        cg.add(var.set_voc(sens))

    if CONF_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VOLTAGE])
        cg.add(var.set_voltage(sens))

    if CONF_FREQUENCY in config:
        sens = await sensor.new_sensor(config[CONF_FREQUENCY])
        cg.add(var.set_frequency(sens))
