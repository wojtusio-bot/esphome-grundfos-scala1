import esphome.codegen as cg
from esphome.components import binary_sensor, ble_client, sensor, switch, text_sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CONNECTIVITY,
    DEVICE_CLASS_PROBLEM,
    DEVICE_CLASS_RUNNING,
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from esphome.types import ConfigType

CODEOWNERS = ["@wojtusio-bot"]
DEPENDENCIES = ["ble_client"]
AUTO_LOAD = ["binary_sensor", "sensor", "switch", "text_sensor"]
MULTI_CONF = True

CONF_CONTROL = "control"
CONF_CONNECTED = "connected"
CONF_RUNNING = "running"
CONF_NO_WATER = "no_water"
CONF_MANUAL_STOP = "manual_stop"
CONF_STATE = "state"
CONF_STATE_CODE = "state_code"
CONF_A4_RAW = "a4_raw"
CONF_A4_HEX = "a4_hex"
CONF_RX_FRAMES = "rx_frames"
CONF_LAST_FRAME = "last_frame"

scala1_ns = cg.esphome_ns.namespace("scala1")
Scala1Component = scala1_ns.class_(
    "Scala1Component", ble_client.BLEClientNode, cg.PollingComponent
)
Scala1ControlSwitch = scala1_ns.class_("Scala1ControlSwitch", switch.Switch)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Scala1Component),
            cv.Optional(CONF_CONTROL): switch.switch_schema(
                Scala1ControlSwitch,
                icon="mdi:pump",
                default_restore_mode="DISABLED",
            ),
            cv.Optional(CONF_CONNECTED): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_CONNECTIVITY,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_RUNNING): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_RUNNING,
            ),
            cv.Optional(CONF_NO_WATER): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_PROBLEM,
            ),
            cv.Optional(CONF_MANUAL_STOP): binary_sensor.binary_sensor_schema(
                icon="mdi:stop-circle",
            ),
            cv.Optional(CONF_STATE): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_STATE_CODE): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_A4_RAW): sensor.sensor_schema(
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_A4_HEX): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_RX_FRAMES): sensor.sensor_schema(
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_LAST_FRAME): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.polling_component_schema("5s"))
)


async def to_code(config: ConfigType) -> None:
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    if conf := config.get(CONF_CONTROL):
        ent = await switch.new_switch(conf)
        cg.add(ent.set_parent(var))
        cg.add(var.set_control_switch(ent))

    if conf := config.get(CONF_CONNECTED):
        ent = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_connected_sensor(ent))

    if conf := config.get(CONF_RUNNING):
        ent = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_running_sensor(ent))

    if conf := config.get(CONF_NO_WATER):
        ent = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_no_water_sensor(ent))

    if conf := config.get(CONF_MANUAL_STOP):
        ent = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_manual_stop_sensor(ent))

    if conf := config.get(CONF_STATE):
        ent = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_state_text_sensor(ent))

    if conf := config.get(CONF_STATE_CODE):
        ent = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_state_code_sensor(ent))

    if conf := config.get(CONF_A4_RAW):
        ent = await sensor.new_sensor(conf)
        cg.add(var.set_a4_raw_sensor(ent))

    if conf := config.get(CONF_A4_HEX):
        ent = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_a4_hex_sensor(ent))

    if conf := config.get(CONF_RX_FRAMES):
        ent = await sensor.new_sensor(conf)
        cg.add(var.set_rx_frames_sensor(ent))

    if conf := config.get(CONF_LAST_FRAME):
        ent = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_last_frame_sensor(ent))
