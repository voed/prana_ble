import logging

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import (
    CONF_ID,
)
from .. import (
    PRANA_BLE_CLIENT_SCHEMA,
    prana_ble_ns,
    register_bedjet_child,
)

_LOGGER = logging.getLogger(__name__)
CODEOWNERS = ["@voed"]
DEPENDENCIES = ["prana_ble"]

PranaBLESwitch = prana_ble_ns.class_("PranaBLESwitch", switch.Switch, cg.PollingComponent)

CONFIG_SCHEMA = (
    switch.SWITCH_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(PranaBLESwitch),
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(PRANA_BLE_CLIENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await switch.register_switch(var, config)
    await register_bedjet_child(var, config)
