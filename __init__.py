import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client, time
from esphome.const import (
    CONF_ID,
    CONF_RECEIVE_TIMEOUT,
    CONF_TIME_ID,
)

CODEOWNERS = ["@voed"]
DEPENDENCIES = ["ble_client"]
MULTI_CONF = True
CONF_PRANA_ID = "prana_ble_id"

prana_ble_ns = cg.esphome_ns.namespace("prana_ble")
PranaBLEHub = prana_ble_ns.class_(
    "PranaBLEHub", ble_client.BLEClientNode, cg.PollingComponent
)

CONFIG_SCHEMA = (
    cv.COMPONENT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(PranaBLEHub),
            cv.Optional(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
            cv.Optional(
                CONF_RECEIVE_TIMEOUT, default="0s"
            ): cv.positive_time_period_milliseconds,
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.polling_component_schema("10s"))
)

PRANA_BLE_CLIENT_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_PRANA_ID): cv.use_id(PranaBLEHub),
    }
)


async def register_bedjet_child(var, config):
    parent = await cg.get_variable(config[CONF_PRANA_ID])
    cg.add(parent.register_child(var))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
    if CONF_TIME_ID in config:
        time_ = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time_id(time_))
    if CONF_RECEIVE_TIMEOUT in config:
        cg.add(var.set_status_timeout(config[CONF_RECEIVE_TIMEOUT]))
