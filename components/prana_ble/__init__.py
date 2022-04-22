import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client
from esphome.const import (
    CONF_ID,
    CONF_MAC_ADDRESS,
    CONF_NAME,
    CONF_ON_CONNECT,
    CONF_ON_DISCONNECT,
    CONF_TRIGGER_ID,
)

DEPENDENCIES = ["ble_client"]

prana_ble_ns = cg.esphome_ns.namespace("prana_ble")
Bedjet = prana_ble_ns.class_(                                  
    "Bedjet", ble_client.BLEClientNode, cg.PollingComponent    
)


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Bedjet),    
        }
    )
    .extend(cv.polling_component_schema("1min"))
    .extend(ble_client.BLE_CLIENT_SCHEMA),
)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield ble_client.register_ble_node(var, config)
    yield cg.register_component(var, config)
