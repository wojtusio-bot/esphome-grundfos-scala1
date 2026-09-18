#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

#ifdef USE_ESP32

#include <esp_gattc_api.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace esphome::scala1 {

namespace espbt = esphome::esp32_ble_tracker;

static const espbt::ESPBTUUID SCALA1_GENI_SERVICE_UUID = espbt::ESPBTUUID::from_uint16(0xFE5D);
static const espbt::ESPBTUUID SCALA1_GENI_CHARACTERISTIC_UUID = espbt::ESPBTUUID::from_raw(
    {0xA9, 0x7B, 0xB8, 0x85, 0x00, 0x1A, 0x28, 0xAA, 0x2A, 0x43, 0x6E, 0x03, 0xD1, 0xFF, 0x9C, 0x85});

class Scala1ControlSwitch;

class Scala1Component final : public ble_client::BLEClientNode, public PollingComponent {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  void request_control(bool enabled);

  void set_control_switch(Scala1ControlSwitch *entity) { this->control_switch_ = entity; }
  void set_connected_sensor(binary_sensor::BinarySensor *entity) { this->connected_sensor_ = entity; }
  void set_running_sensor(binary_sensor::BinarySensor *entity) { this->running_sensor_ = entity; }
  void set_no_water_sensor(binary_sensor::BinarySensor *entity) { this->no_water_sensor_ = entity; }
  void set_manual_stop_sensor(binary_sensor::BinarySensor *entity) { this->manual_stop_sensor_ = entity; }
  void set_state_text_sensor(text_sensor::TextSensor *entity) { this->state_text_sensor_ = entity; }
  void set_state_code_sensor(text_sensor::TextSensor *entity) { this->state_code_sensor_ = entity; }
  void set_a4_raw_sensor(sensor::Sensor *entity) { this->a4_raw_sensor_ = entity; }
  void set_a4_hex_sensor(text_sensor::TextSensor *entity) { this->a4_hex_sensor_ = entity; }
  void set_rx_frames_sensor(sensor::Sensor *entity) { this->rx_frames_sensor_ = entity; }
  void set_last_frame_sensor(text_sensor::TextSensor *entity) { this->last_frame_sensor_ = entity; }

 protected:
  bool ready_() const;
  bool send_request_(uint8_t *data, size_t len);
  void query_status_();
  void query_a4_();
  void schedule_a4_query_();
  void handle_notification_(const uint8_t *data, size_t len);
  void process_frame_(const std::vector<uint8_t> &frame);
  void publish_state_(uint8_t state1, uint8_t state2);
  void publish_disconnected_();

  uint16_t geni_handle_{0};
  bool command_busy_{false};
  bool io_ready_{false};
  uint32_t rx_count_{0};

  std::vector<uint8_t> rx_buffer_{};
  size_t expected_len_{0};

  Scala1ControlSwitch *control_switch_{nullptr};
  binary_sensor::BinarySensor *connected_sensor_{nullptr};
  binary_sensor::BinarySensor *running_sensor_{nullptr};
  binary_sensor::BinarySensor *no_water_sensor_{nullptr};
  binary_sensor::BinarySensor *manual_stop_sensor_{nullptr};
  text_sensor::TextSensor *state_text_sensor_{nullptr};
  text_sensor::TextSensor *state_code_sensor_{nullptr};
  sensor::Sensor *a4_raw_sensor_{nullptr};
  text_sensor::TextSensor *a4_hex_sensor_{nullptr};
  sensor::Sensor *rx_frames_sensor_{nullptr};
  text_sensor::TextSensor *last_frame_sensor_{nullptr};
};

class Scala1ControlSwitch : public switch_::Switch {
 public:
  void set_parent(Scala1Component *parent) { this->parent_ = parent; }

 protected:
  void write_state(bool state) override;
  Scala1Component *parent_{nullptr};
};

}  // namespace esphome::scala1

#endif
