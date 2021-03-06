#pragma once

#ifdef USE_ESP32

#include <esp_gattc_api.h>
#include <algorithm>
#include <iterator>
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"

namespace esphome {
namespace prana_ble {

static const char *const SERVICE_UUID = "0000baba-0000-1000-8000-00805f9b34fb";
static const char *const RW_CHARACTERISTIC_UUID = "0000cccc-0000-1000-8000-00805f9b34fb";
static const char *const TAG = "prana_ble";

class PranaBLE : public PollingComponent, public ble_client::BLEClientNode {
 public:
  PranaBLE();

  void dump_config() override;
  void update() override;

  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;



 protected:
  bool is_valid_radon_value_(float radon);

  void read_sensors_(uint8_t *value, uint16_t value_len);
  void write_query_message_();
  void write_notify_message_();
  void request_read_values_();
  uint8_t set_notify_(const bool enable);


  uint16_t char_handle_;
  esp32_ble_tracker::ESPBTUUID service_uuid_;
  esp32_ble_tracker::ESPBTUUID sensors_char_uuid_;

  union RadonValue {
    char chars[4];
    float number;
  };
};

}  // namespace radon_eye_rd200
}  // namespace esphome

#endif  // USE_ESP32
