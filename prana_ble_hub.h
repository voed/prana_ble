#pragma once

#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "prana_ble_child.h"
#include "prana_ble_const.h"
#include <vector>

#ifdef USE_ESP32

#include <esp_gattc_api.h>

namespace esphome {
namespace prana_ble {



namespace espbt = esphome::esp32_ble_tracker;

// Forward declare PranaBLEClient
class PranaBLEClient;

static const espbt::ESPBTUUID BEDJET_SERVICE_UUID = espbt::ESPBTUUID::from_uint16(0xBABA);
static const espbt::ESPBTUUID BEDJET_CHAR_UUID    = espbt::ESPBTUUID::from_uint16(0xCCCC);

/**
 * Hub component connecting to the BedJet device over Bluetooth.
 */
class PranaBLEHub : public esphome::ble_client::BLEClientNode, public PollingComponent {
 public:
  /* BedJet functionality exposed to `BedJetClient` children and/or accessible from action lambdas. */
  

  bool command_off();
  bool command_brightness();
  bool command_heating();
  bool command_night_mode();
  bool command_high_speed();
  bool command_fan_lock();
  bool command_fan_out_off();
  bool command_fan_out_speed_up();
  bool command_fan_out_speed_down();
  bool command_winter_mode();
  bool command_auto_mode();
  bool command_fan_speed_down();
  bool command_fan_speed_up();
  bool command_fan_in_speed_up();
  bool command_fan_in_speed_down();
  bool command_fan_in_off();


  /** Send the `button`. */
  bool send_command(const PranaCommand& command);


  /** @return `true` if the `BLEClient::node_state` is `ClientState::ESTABLISHED`. */
  bool is_connected() { return this->node_state == espbt::ClientState::ESTABLISHED; }

  bool has_status() { return true;} //this->status != nullptr; }

  /** Register a `BedJetClient` child component. */
  void register_child(PranaBLEClient *obj);

  /** Set the status timeout.
   *
   * This is the max time to wait for a status update before the connection is presumed unusable.
   */
  void set_status_timeout(uint32_t timeout) { this->timeout_ = timeout; }

  /* Component overrides */
  void loop() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

  PranaStatusPacket* get_status_packet() { send_update_request(); return &status; }

  /** @return The BedJet's configured name, or the MAC address if not discovered yet. */
  std::string get_name() {
    if (this->name_.empty()) {
      return this->parent_->address_str();
    } else {
      return this->name_;
    }
  }

  /* BLEClient overrides */
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

 protected:
  std::vector<PranaBLEClient *> children_;
  void dispatch_status_();
  void dispatch_state_(bool is_ready);

  uint32_t timeout_{DEFAULT_STATUS_TIMEOUT};
  static const uint32_t MIN_NOTIFY_THROTTLE = 15000;
  static const uint32_t NOTIFY_WARN_THRESHOLD = 300000;
  static const uint32_t DEFAULT_STATUS_TIMEOUT = 900000;

  PranaStatusPacket status;

  uint8_t set_notify_(bool enable);
  uint8_t send_packet(PranaCmdPacket *pkt, bool update=false);
  /** Send the `BedjetPacket` to the device. */
  uint8_t send_data(uint8_t data[], uint8_t len, bool update=false);
  bool send_command(PranaCommand command, bool update=false);
  void set_name_(const std::string &name) { this->name_ = name; }
  uint8_t send_update_request();
  std::string name_;

  uint32_t last_notify_ = 0;
  bool force_refresh_ = false;

  bool discover_characteristics_();
  uint16_t char_handle_;
  uint16_t config_descr_status_;

  uint8_t write_notify_config_descriptor_(bool enable);
};

}  // namespace bedjet
}  // namespace esphome

#endif
