#include "scala1.h"

#include "esphome/core/log.h"

#ifdef USE_ESP32

#include <cstring>
#include <cstdio>

namespace esphome::scala1 {

static const char *const TAG = "scala1";

static uint8_t STATUS_REQUEST[] = {0x27, 0x07, 0xE7, 0xF8, 0x0A, 0x03, 0x56, 0x00, 0x08, 0x24, 0x94};
static uint8_t A4_REQUEST[] = {0x27, 0x07, 0xE7, 0xF8, 0x0A, 0x03, 0x5B, 0x00, 0xA4, 0x12, 0xA3};

static uint8_t STOP_FRAGMENT_1[] = {0x27, 0x14, 0xE7, 0xF8, 0x0A, 0x90, 0x56, 0x00, 0x06, 0x01,
                                    0x2F, 0x01, 0x00, 0x00, 0x07, 0x01, 0x01, 0x00, 0x00, 0x00};
static uint8_t STOP_FRAGMENT_2[] = {0x00, 0x00, 0x6A, 0x76};

static uint8_t START_FRAGMENT_1[] = {0x27, 0x14, 0xE7, 0xF8, 0x0A, 0x90, 0x56, 0x00, 0x06, 0x01,
                                     0x2F, 0x01, 0x00, 0x00, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00};
static uint8_t START_FRAGMENT_2[] = {0x00, 0x00, 0x2F, 0xD6};

static uint8_t SYNC_FRAGMENT_1[] = {0x27, 0x17, 0xE7, 0xF8, 0x0A, 0x93, 0x54, 0x00, 0x01, 0x00,
                                    0xDA, 0x01, 0x00, 0x00, 0x0A, 0x05, 0x0A, 0x00, 0x0A, 0x00};
static uint8_t SYNC_FRAGMENT_2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xF8};

void Scala1Component::setup() {}

void Scala1Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Grundfos SCALA1");
  ESP_LOGCONFIG(TAG, "  BLE address: %s", this->parent_->address_str());
  LOG_UPDATE_INTERVAL(this);
  LOG_SWITCH("  ", "Control", this->control_switch_);
  LOG_BINARY_SENSOR("  ", "BLE connected", this->connected_sensor_);
  LOG_BINARY_SENSOR("  ", "Pump running", this->running_sensor_);
  LOG_BINARY_SENSOR("  ", "No water", this->no_water_sensor_);
  LOG_BINARY_SENSOR("  ", "Manual STOP", this->manual_stop_sensor_);
  LOG_TEXT_SENSOR("  ", "State", this->state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "State code", this->state_code_sensor_);
  LOG_SENSOR("  ", "A4 raw", this->a4_raw_sensor_);
  LOG_TEXT_SENSOR("  ", "A4 hex", this->a4_hex_sensor_);
  LOG_SENSOR("  ", "RX frames", this->rx_frames_sensor_);
  LOG_TEXT_SENSOR("  ", "Last frame", this->last_frame_sensor_);
}

bool Scala1Component::ready_() const {
  return this->node_state == espbt::ClientState::ESTABLISHED && this->geni_handle_ != 0 && this->io_ready_;
}

bool Scala1Component::send_request_(uint8_t *data, size_t len) {
  if (!this->ready_()) {
    ESP_LOGW(TAG, "Cannot write: SCALA1 BLE link is not established");
    return false;
  }

  auto status = esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(),
                                         this->geni_handle_, len, data, ESP_GATT_WRITE_TYPE_NO_RSP,
                                         ESP_GATT_AUTH_REQ_NONE);
  if (status != ESP_GATT_OK) {
    ESP_LOGW(TAG, "[%s] BLE write failed, status=%d", this->parent_->address_str(), status);
    return false;
  }
  return true;
}

void Scala1Component::query_status_() {
  if (!this->ready_())
    return;
  this->send_request_(STATUS_REQUEST, sizeof(STATUS_REQUEST));
}

void Scala1Component::query_a4_() {
  if (!this->ready_())
    return;
  this->send_request_(A4_REQUEST, sizeof(A4_REQUEST));
}

void Scala1Component::schedule_a4_query_() {
  if (this->a4_raw_sensor_ == nullptr && this->a4_hex_sensor_ == nullptr)
    return;

  this->set_timeout("scala1_a4", 800, [this]() {
    if (this->ready_() && !this->command_busy_)
      this->query_a4_();
  });
}

void Scala1Component::update() {
  if (!this->ready_()) {
    ESP_LOGV(TAG, "Cannot poll: SCALA1 is not connected");
    return;
  }
  if (this->command_busy_)
    return;

  this->query_status_();
  this->schedule_a4_query_();
}

void Scala1Component::request_control(bool enabled) {
  if (!this->ready_()) {
    ESP_LOGW(TAG, "Cannot %s SCALA1: BLE link is not established", enabled ? "START" : "STOP");
    return;
  }
  if (this->command_busy_) {
    ESP_LOGW(TAG, "Ignoring %s: another SCALA1 command is still in progress", enabled ? "START" : "STOP");
    return;
  }

  this->command_busy_ = true;
  ESP_LOGI(TAG, "Sending physical %s sequence", enabled ? "START" : "STOP");

  uint8_t *fragment_1 = enabled ? START_FRAGMENT_1 : STOP_FRAGMENT_1;
  size_t fragment_1_len = enabled ? sizeof(START_FRAGMENT_1) : sizeof(STOP_FRAGMENT_1);
  uint8_t *fragment_2 = enabled ? START_FRAGMENT_2 : STOP_FRAGMENT_2;
  size_t fragment_2_len = enabled ? sizeof(START_FRAGMENT_2) : sizeof(STOP_FRAGMENT_2);

  bool ok = true;
  ok &= this->send_request_(fragment_1, fragment_1_len);
  delay(2);
  ok &= this->send_request_(fragment_2, fragment_2_len);
  delay(30);
  ok &= this->send_request_(SYNC_FRAGMENT_1, sizeof(SYNC_FRAGMENT_1));
  delay(2);
  ok &= this->send_request_(SYNC_FRAGMENT_2, sizeof(SYNC_FRAGMENT_2));

  if (!ok) {
    this->command_busy_ = false;
    ESP_LOGW(TAG, "%s sequence was not fully written", enabled ? "START" : "STOP");
    return;
  }

  this->set_timeout("scala1_command_status", 35, [this]() {
    if (this->ready_())
      this->query_status_();
  });

  if (this->a4_raw_sensor_ != nullptr || this->a4_hex_sensor_ != nullptr) {
    this->set_timeout("scala1_command_a4", 850, [this]() {
      if (this->ready_())
        this->query_a4_();
    });
  }

  this->set_timeout("scala1_command_done", 1100, [this]() { this->command_busy_ = false; });
}

void Scala1Component::publish_disconnected_() {
  if (this->connected_sensor_ != nullptr)
    this->connected_sensor_->publish_state(false);
  if (this->running_sensor_ != nullptr)
    this->running_sensor_->publish_state(false);
  if (this->no_water_sensor_ != nullptr)
    this->no_water_sensor_->publish_state(false);
  if (this->manual_stop_sensor_ != nullptr)
    this->manual_stop_sensor_->publish_state(false);
  if (this->state_text_sensor_ != nullptr)
    this->state_text_sensor_->publish_state("Disconnected");
}

void Scala1Component::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                          esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      if (param->open.status == ESP_GATT_OK) {
        this->rx_buffer_.clear();
        this->expected_len_ = 0;
        this->geni_handle_ = 0;
        this->command_busy_ = false;
        this->io_ready_ = false;
      }
      break;
    }

    case ESP_GATTC_CONNECT_EVT: {
      if (std::memcmp(param->connect.remote_bda, this->parent_->get_remote_bda(), 6) != 0)
        return;

      auto status = this->parent_->pair();
      if (status != ESP_OK)
        ESP_LOGD(TAG, "Pair/encryption request returned status=%d", status);
      break;
    }

    case ESP_GATTC_SEARCH_CMPL_EVT: {
      auto *chr = this->parent_->get_characteristic(SCALA1_GENI_SERVICE_UUID, SCALA1_GENI_CHARACTERISTIC_UUID);
      if (chr == nullptr) {
        ESP_LOGE(TAG, "[%s] SCALA1 GENI characteristic not found", this->parent_->address_str());
        break;
      }

      this->geni_handle_ = chr->handle;
      auto status = esp_ble_gattc_register_for_notify(this->parent_->get_gattc_if(), this->parent_->get_remote_bda(),
                                                      this->geni_handle_);
      if (status != ESP_GATT_OK)
        ESP_LOGW(TAG, "Register-for-notify failed, status=%d", status);
      break;
    }

    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      if (param->reg_for_notify.handle != this->geni_handle_)
        break;
      if (param->reg_for_notify.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Notify registration failed, status=%d", param->reg_for_notify.status);
        break;
      }

      this->node_state = espbt::ClientState::ESTABLISHED;
      this->io_ready_ = false;
      if (this->connected_sensor_ != nullptr)
        this->connected_sensor_->publish_state(true);
      ESP_LOGI(TAG, "[%s] SCALA1 BLE established; waiting for pairing/authentication", this->parent_->address_str());
      this->set_timeout("scala1_ready", 6000, [this]() {
        if (this->node_state == espbt::ClientState::ESTABLISHED && this->geni_handle_ != 0) {
          this->io_ready_ = true;
          ESP_LOGI(TAG, "[%s] SCALA1 ready for GENI traffic", this->parent_->address_str());
          this->update();
        }
      });
      break;
    }

    case ESP_GATTC_NOTIFY_EVT: {
      if (param->notify.handle == this->geni_handle_)
        this->handle_notification_(param->notify.value, param->notify.value_len);
      break;
    }

    case ESP_GATTC_DISCONNECT_EVT: {
      this->node_state = espbt::ClientState::IDLE;
      this->geni_handle_ = 0;
      this->command_busy_ = false;
      this->io_ready_ = false;
      this->rx_buffer_.clear();
      this->expected_len_ = 0;
      this->publish_disconnected_();
      break;
    }

    default:
      break;
  }
}

void Scala1Component::handle_notification_(const uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    const uint8_t byte = data[i];

    if (this->rx_buffer_.empty()) {
      if (byte != 0x24)
        continue;
      this->expected_len_ = 0;
    }

    this->rx_buffer_.push_back(byte);

    if (this->rx_buffer_.size() == 2) {
      this->expected_len_ = static_cast<size_t>(this->rx_buffer_[1]) + 4;
      if (this->expected_len_ < 4 || this->expected_len_ > 260) {
        ESP_LOGW(TAG, "Invalid GENI frame length: %u", static_cast<unsigned>(this->expected_len_));
        this->rx_buffer_.clear();
        this->expected_len_ = 0;
        continue;
      }
    }

    if (this->expected_len_ != 0 && this->rx_buffer_.size() == this->expected_len_) {
      this->process_frame_(this->rx_buffer_);
      this->rx_buffer_.clear();
      this->expected_len_ = 0;
    }
  }
}

void Scala1Component::process_frame_(const std::vector<uint8_t> &frame) {
  this->rx_count_++;

  if (this->rx_frames_sensor_ != nullptr)
    this->rx_frames_sensor_->publish_state(this->rx_count_);

  std::string hex;
  hex.reserve(frame.size() * 3);
  char tmp[4];
  for (size_t i = 0; i < frame.size(); i++) {
    std::snprintf(tmp, sizeof(tmp), "%02X", frame[i]);
    if (i != 0)
      hex += ' ';
    hex += tmp;
  }

  ESP_LOGD(TAG, "RX #%u len=%u: %s", static_cast<unsigned>(this->rx_count_),
           static_cast<unsigned>(frame.size()), hex.c_str());

  if (this->last_frame_sensor_ != nullptr)
    this->last_frame_sensor_->publish_state(hex);

  if (frame.size() == 22 && frame.size() > 14 && frame[8] == 0x2F) {
    this->publish_state_(frame[13], frame[14]);
    return;
  }

  if (frame.size() == 26 && frame.size() > 14 && frame[8] == 0xA4) {
    const uint8_t low = frame[13];
    const uint8_t high = frame[14];
    const uint16_t raw = static_cast<uint16_t>(low) | (static_cast<uint16_t>(high) << 8);

    if (this->a4_raw_sensor_ != nullptr)
      this->a4_raw_sensor_->publish_state(raw);

    if (this->a4_hex_sensor_ != nullptr) {
      char a4_hex[8];
      std::snprintf(a4_hex, sizeof(a4_hex), "%02X %02X", low, high);
      this->a4_hex_sensor_->publish_state(a4_hex);
    }
  }
}

void Scala1Component::publish_state_(uint8_t state1, uint8_t state2) {
  char state_hex[8];
  std::snprintf(state_hex, sizeof(state_hex), "%02X %02X", state1, state2);

  if (this->state_code_sensor_ != nullptr)
    this->state_code_sensor_->publish_state(state_hex);

  bool running = false;
  bool no_water = false;
  bool manual_stop = false;
  bool enabled = true;
  const char *state_text = nullptr;

  if (state1 == 0x01 && state2 == 0x00) {
    running = true;
    state_text = "Running";
  } else if (state1 == 0x6D && state2 == 0x01) {
    state_text = "Ready / idle";
  } else if (state1 == 0x01 && state2 == 0x01) {
    manual_stop = true;
    enabled = false;
    state_text = "Manual STOP";
  } else if (state1 == 0x08 && state2 == 0x01) {
    no_water = true;
    state_text = "No water / dry-run";
  } else {
    enabled = false;
  }

  if (this->running_sensor_ != nullptr)
    this->running_sensor_->publish_state(running);
  if (this->no_water_sensor_ != nullptr)
    this->no_water_sensor_->publish_state(no_water);
  if (this->manual_stop_sensor_ != nullptr)
    this->manual_stop_sensor_->publish_state(manual_stop);

  if (this->control_switch_ != nullptr && state_text != nullptr)
    this->control_switch_->publish_state(enabled);

  if (this->state_text_sensor_ != nullptr) {
    if (state_text != nullptr) {
      this->state_text_sensor_->publish_state(state_text);
    } else {
      std::string unknown = "Unknown: ";
      unknown += state_hex;
      this->state_text_sensor_->publish_state(unknown);
    }
  }
}

void Scala1ControlSwitch::write_state(bool state) {
  if (this->parent_ != nullptr)
    this->parent_->request_control(state);
}

}  // namespace esphome::scala1

#endif
