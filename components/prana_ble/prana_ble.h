#pragma once

#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/climate/climate.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "bedjet_base.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

#ifdef USE_ESP32

#include <esp_gattc_api.h>

namespace esphome {
namespace prana_ble {

namespace espbt = esphome::esp32_ble_tracker;

static const espbt::ESPBTUUID BEDJET_SERVICE_UUID = espbt::ESPBTUUID::from_raw("0000baba-0000-1000-8000-00805f9b34fb");
static const espbt::ESPBTUUID BEDJET_STATUS_UUID = espbt::ESPBTUUID::from_raw("0000cccc-0000-1000-8000-00805f9b34fb");
static const espbt::ESPBTUUID BEDJET_COMMAND_UUID = espbt::ESPBTUUID::from_raw("0000cccc-0000-1000-8000-00805f9b34fb");
//static const espbt::ESPBTUUID BEDJET_NAME_UUID = espbt::ESPBTUUID::from_raw("00002001-bed0-0080-aa55-4265644a6574");

class Bedjet : public climate::Climate, public esphome::ble_client::BLEClientNode, public PollingComponent {
 public:
  void setup() override;
  void update() override;
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

#ifdef USE_TIME
  void set_time_id(time::RealTimeClock *time_id) { this->time_id_ = time_id; }
#endif
  void set_status_timeout(uint32_t timeout) { this->timeout_ = timeout; }


  uint32_t timeout_{DEFAULT_STATUS_TIMEOUT};

  static const uint32_t MIN_NOTIFY_THROTTLE = 5000;
  static const uint32_t NOTIFY_WARN_THRESHOLD = 300000;
  static const uint32_t DEFAULT_STATUS_TIMEOUT = 900000;

  uint8_t set_notify_(bool enable);

  uint32_t last_notify_ = 0;
  bool force_refresh_ = false;

  uint16_t char_handle_cmd_;
  uint16_t char_handle_name_;
  uint16_t char_handle_status_;
  uint16_t config_descr_status_;

  uint8_t write_notify_config_descriptor_(bool enable);
};

}  // namespace bedjet
}  // namespace esphome

#endif
