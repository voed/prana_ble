#include "prana_ble_hub.h"
#include "prana_ble_child.h"
#include "prana_ble_const.h"

namespace esphome {
namespace prana_ble {



static const LogString *prana_cmd_to_string(PranaCommand command) {
  switch (command) {
    case CMD_OFF:
      return LOG_STR("OFF");
    case CMD_BRIGHTNESS:
      return LOG_STR("CMD_BRIGHTNESS");
    case CMD_HEATING:
      return LOG_STR("CMD_HEATING");
    case CMD_NIGHT_MODE:
      return LOG_STR("CMD_NIGHT_MODE");
    case CMD_HIGH_SPEED:
      return LOG_STR("CMD_HIGH_SPEED");
    case CMD_FAN_LOCK:
      return LOG_STR("CMD_FAN_LOCK");
    case CMD_FAN_OUT_OFF:
      return LOG_STR("CMD_FAN_OUT_OFF");
    case CMD_FAN_OUT_SPEED_UP:
      return LOG_STR("CMD_FAN_OUT_SPEED_UP");
    case CMD_FAN_OUT_SPEED_DOWN:
      return LOG_STR("CMD_FAN_OUT_SPEED_DOWN");
    case CMD_WINTER_MODE:
      return LOG_STR("CMD_WINTER_MODE");
    case CMD_AUTO_MODE:
      return LOG_STR("CMD_AUTO_MODE");
    case CMD_FAN_SPEED_DOWN:
      return LOG_STR("CMD_FAN_SPEED_DOWN");
    case CMD_FAN_SPEED_UP:
      return LOG_STR("CMD_FAN_SPEED_UP");
    case CMD_FAN_IN_SPEED_UP:
      return LOG_STR("CMD_FAN_IN_SPEED_UP");
    case CMD_FAN_IN_SPEED_DOWN:
      return LOG_STR("CMD_FAN_IN_SPEED_DOWN");
    case CMD_FAN_IN_OFF:
      return LOG_STR("CMD_FAN_IN_OFF");
    default:
      return LOG_STR("unknown");
  }
}

/* Public */

bool PranaBLEHub::command_off() { return send_command(CMD_OFF, true); }
bool PranaBLEHub::command_brightness() { return send_command(CMD_BRIGHTNESS, true); }
bool PranaBLEHub::command_heating() { return send_command(CMD_HEATING, true); }
bool PranaBLEHub::command_night_mode() { return send_command(CMD_NIGHT_MODE, true); }
bool PranaBLEHub::command_high_speed() { return send_command(CMD_HIGH_SPEED, true); }
bool PranaBLEHub::command_fan_lock() { return send_command(CMD_FAN_LOCK, true); }
bool PranaBLEHub::command_fan_out_off() { return send_command(CMD_FAN_OUT_OFF, true); }
bool PranaBLEHub::command_fan_out_speed_up() { return send_command(CMD_FAN_OUT_SPEED_UP, true); }
bool PranaBLEHub::command_fan_out_speed_down() { return send_command(CMD_FAN_OUT_SPEED_DOWN, true); }
bool PranaBLEHub::command_winter_mode() { return send_command(CMD_WINTER_MODE, true); }
bool PranaBLEHub::command_auto_mode() { return send_command(CMD_AUTO_MODE, true); }
bool PranaBLEHub::command_fan_speed_down() { return send_command(CMD_FAN_SPEED_DOWN, true); }
bool PranaBLEHub::command_fan_speed_up() { return send_command(CMD_FAN_SPEED_UP, true); }
bool PranaBLEHub::command_fan_in_speed_up() { return send_command(CMD_FAN_IN_SPEED_UP, true); }
bool PranaBLEHub::command_fan_in_speed_down() { return send_command(CMD_FAN_IN_SPEED_DOWN, true); }
bool PranaBLEHub::command_fan_in_off() { return send_command(CMD_FAN_IN_OFF, true); }



bool PranaBLEHub::send_command(PranaCommand command, bool update) {
  auto packet = new PranaCmdPacket(command);
  auto status = this->send_packet(packet);

  if (status) 
  {

    ESP_LOGW(TAG, "[%s] writing button %s failed, status=%d", this->get_name().c_str(),
             LOG_STR_ARG(prana_cmd_to_string(command)), status);
  } else {
    ESP_LOGD(TAG, "[%s] writing button %s success", this->get_name().c_str(),
             LOG_STR_ARG(prana_cmd_to_string(command)));
  }
  return status == 0;
}

uint8_t PranaBLEHub::send_packet(PranaCmdPacket *pkt, bool update)
{
  return this->send_data((uint8_t *) &pkt, sizeof(pkt));
}

uint8_t PranaBLEHub::send_data(uint8_t data[], uint8_t len, bool update) {
  auto status = esp_ble_gattc_write_char(
                      this->parent_->get_gattc_if(), 
                      this->parent()->get_conn_id(),
                      this->char_handle_,
                      len, 
                      data, 
                      ESP_GATT_WRITE_TYPE_RSP,
                      ESP_GATT_AUTH_REQ_NONE
                      );

  return status;
}

uint8_t PranaBLEHub::send_update_request()
{
  uint8_t data[] = {0xbe, 0xef, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x5A};
  return send_data(data, sizeof(data), false);
}


/** Configures the local ESP BLE client to register (`true`) or unregister (`false`) for status notifications. */
uint8_t PranaBLEHub::set_notify_(const bool enable) {
  uint8_t status;
  if (enable) {
    status = esp_ble_gattc_register_for_notify(this->parent_->get_gattc_if(), this->parent_->get_remote_bda(),
                                               this->char_handle_);
    if (status) {
      ESP_LOGW(TAG, "[%s] esp_ble_gattc_register_for_notify failed, status=%d", this->get_name().c_str(), status);
    }
  } else {
    status = esp_ble_gattc_unregister_for_notify(this->parent_->get_gattc_if(), this->parent_->get_remote_bda(),
                                                 this->char_handle_);
    if (status) {
      ESP_LOGW(TAG, "[%s] esp_ble_gattc_unregister_for_notify failed, status=%d", this->get_name().c_str(), status);
    }
  }
  ESP_LOGV(TAG, "[%s] set_notify: enable=%d; result=%d", this->get_name().c_str(), enable, status);
  return status;
}

bool PranaBLEHub::discover_characteristics_() {
  bool result = true;
  esphome::ble_client::BLECharacteristic *chr;


  if (!this->char_handle_) {
    chr = this->parent_->get_characteristic(BEDJET_SERVICE_UUID, BEDJET_CHAR_UUID);
    if (chr == nullptr) {
      ESP_LOGW(TAG, "[%s] No status service found at device, not a BedJet..?", this->get_name().c_str());
      result = false;
    } else {
      this->char_handle_ = chr->handle;
    }
  }

  if (!this->config_descr_status_) {
    // We also need to obtain the config descriptor for this handle.
    // Otherwise once we set node_state=Established, the parent will flush all handles/descriptors, and we won't be
    // able to look it up.
    auto *descr = this->parent_->get_config_descriptor(this->char_handle_);
    if (descr == nullptr) {
      ESP_LOGW(TAG, "No config descriptor for status handle 0x%x. Will not be able to receive status notifications",
               this->char_handle_);
      result = false;
    } else if (descr->uuid.get_uuid().len != ESP_UUID_LEN_16 ||
               descr->uuid.get_uuid().uuid.uuid16 != ESP_GATT_UUID_CHAR_CLIENT_CONFIG) {
      ESP_LOGW(TAG, "Config descriptor 0x%x (uuid %s) is not a client config char uuid", this->char_handle_,
               descr->uuid.to_string().c_str());
      result = false;
    } else {
      this->config_descr_status_ = descr->handle;
    }
  }


  ESP_LOGI(TAG, "[%s] Discovered service characteristics: ", this->get_name().c_str());
  ESP_LOGI(TAG, "     - Status char: 0x%x", this->char_handle_);
  ESP_LOGI(TAG, "       - config descriptor: 0x%x", this->config_descr_status_);
  return result;
}

void PranaBLEHub::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                    esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGV(TAG, "Disconnected: reason=%d", param->disconnect.reason);
      this->status_set_warning();
      this->dispatch_state_(false);
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      auto result = this->discover_characteristics_();

      if (result) {
        ESP_LOGD(TAG, "[%s] Services complete: obtained char handles.", this->get_name().c_str());
        this->node_state = espbt::ClientState::ESTABLISHED;
        this->set_notify_(true);

#ifdef USE_TIME
        if (this->time_id_.has_value()) {
          this->send_local_time();
        }
#endif

        this->dispatch_state_(true);
      } else {
        ESP_LOGW(TAG, "[%s] Failed discovering service characteristics.", this->get_name().c_str());
        this->parent()->set_enabled(false);
        this->status_set_warning();
        this->dispatch_state_(false);
      }
      break;
    }

    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      // This event means that ESP received the request to enable notifications on the client side. But we also have to
      // tell the server that we want it to send notifications. Normally BLEClient parent would handle this
      // automatically, but as soon as we set our status to Established, the parent is going to purge all the
      // service/char/descriptor handles, and then get_config_descriptor() won't work anymore. There's no way to disable
      // the BLEClient parent behavior, so our only option is to write the handle anyway, and hope a double-write
      // doesn't break anything.

      if (param->reg_for_notify.handle != this->char_handle_) {
        ESP_LOGW(TAG, "[%s] Register for notify on unexpected handle 0x%04x, expecting 0x%04x",
                 this->get_name().c_str(), param->reg_for_notify.handle, this->char_handle_);
        break;
      }

      this->write_notify_config_descriptor_(true);
      this->last_notify_ = 0;
      this->force_refresh_ = true;
      break;
    }
    case ESP_GATTC_UNREG_FOR_NOTIFY_EVT: {
      // This event is not handled by the parent BLEClient, so we need to do this either way.
      if (param->unreg_for_notify.handle != this->char_handle_) {
        ESP_LOGW(TAG, "[%s] Unregister for notify on unexpected handle 0x%04x, expecting 0x%04x",
                 this->get_name().c_str(), param->unreg_for_notify.handle, this->char_handle_);
        break;
      }

      this->write_notify_config_descriptor_(false);
      this->last_notify_ = 0;
      // Now we wait until the next update() poll to re-register notify...
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      if (param->notify.conn_id != this->parent_->get_conn_id()) {
        ESP_LOGW(TAG, "[%s] Received notify event for unexpected parent conn: expect %x, got %x",
                 this->get_name().c_str(), this->parent_->get_conn_id(), param->notify.conn_id);
        // FIXME: bug in BLEClient holding wrong conn_id.
      }

      if (param->notify.handle != this->char_handle_) {
        ESP_LOGW(TAG, "[%s] Unexpected notify handle, wanted %04X, got %04X", this->get_name().c_str(),
                 this->char_handle_, param->notify.handle);
        break;
      }



      ESP_LOGW(TAG, "NOTIFY len %d", param->notify.value_len);
      if(param->notify.value_len > 0)
      {
        PranaStatusPacket* packet = (PranaStatusPacket*)param->notify.value;
        if (packet != nullptr) {
          this->status = *packet;
          ESP_LOGD(TAG, "[%s] Notifying %d children of latest status @%p.", this->get_name().c_str(), this->children_.size(),
                  packet);
          for (auto *child : this->children_) {
            child->on_status(packet);
          }
        } 
        this->last_notify_ = millis();
        ESP_LOGW(TAG, "speed: %d", param->notify.value[26] / 10);
        ESP_LOGW(TAG, "speed: %d", packet->speed / 10);
        ESP_LOGW(TAG, "speedIn: %d", param->notify.value[30]/ 10);
        ESP_LOGW(TAG, "speedIn: %d", packet->speed_in / 10);
        ESP_LOGW(TAG, "speedOut: %d", param->notify.value[34] / 10);
        ESP_LOGW(TAG, "speedOut: %d", packet->speed_out/ 10);
        ESP_LOGW(TAG, "heat: %d", param->notify.value[14]);
        ESP_LOGW(TAG, "heat: %d", packet->heating_on);
        ESP_LOGW(TAG, "winter_mode: %d", param->notify.value[42]);
        ESP_LOGW(TAG, "winter_mode: %d", packet->winter_mode);
        ESP_LOGW(TAG, "winter_mode: %d", param->notify.value[96]);
        ESP_LOGW(TAG, "voltage: %d", packet->voltage);
      }

      // FIXME: notify events come in every ~200-300 ms, which is too fast to be helpful. So we
      //  throttle the updates to once every MIN_NOTIFY_THROTTLE (5 seconds).
      //  Another idea would be to keep notify off by default, and use update() as an opportunity to turn on
      //  notify to get enough data to update status, then turn off notify again.

      uint32_t now = millis();
      auto delta = now - this->last_notify_;

      break;
    }
    default:
      ESP_LOGVV(TAG, "[%s] gattc unhandled event: enum=%d", this->get_name().c_str(), event);
      break;
  }
}

/*inline void PranaBLEHub::status_packet_ready_() {
  this->last_notify_ = millis();
  this->processing_ = false;

  if (this->force_refresh_) {
    // If we requested an immediate update, do that now.
    this->update();
    this->force_refresh_ = false;
  }
}*/

/** Reimplementation of BLEClient.gattc_event_handler() for ESP_GATTC_REG_FOR_NOTIFY_EVT.
 *
 * This is a copy of ble_client's automatic handling of `ESP_GATTC_REG_FOR_NOTIFY_EVT`, in order
 * to undo the same on unregister. It also allows us to maintain the config descriptor separately,
 * since the parent BLEClient is going to purge all descriptors once we set our connection status
 * to `Established`.
 */
uint8_t PranaBLEHub::write_notify_config_descriptor_(bool enable) {
  auto handle = this->config_descr_status_;
  if (handle == 0) {
    ESP_LOGW(TAG, "No descriptor found for notify of handle 0x%x", this->char_handle_);
    return -1;
  }

  // NOTE: BLEClient uses `uint8_t*` of length 1, but BLE spec requires 16 bits.
  uint16_t notify_en = enable ? 1 : 0;
  auto status = esp_ble_gattc_write_char_descr(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), handle,
                                               sizeof(notify_en), (uint8_t *) &notify_en, ESP_GATT_WRITE_TYPE_RSP,
                                               ESP_GATT_AUTH_REQ_NONE);
  if (status) {
    ESP_LOGW(TAG, "esp_ble_gattc_write_char_descr error, status=%d", status);
    return status;
  }
  ESP_LOGD(TAG, "[%s] wrote notify=%s to status config 0x%04x, for conn %d", this->get_name().c_str(),
           enable ? "true" : "false", handle, this->parent_->get_conn_id());
  return ESP_GATT_OK;
}


/* Internal */

void PranaBLEHub::loop() {}
void PranaBLEHub::update() { this->dispatch_status_(); }

void PranaBLEHub::dump_config() {
  ESP_LOGCONFIG(TAG, "Prana BLE Hub '%s'", this->get_name().c_str());
  ESP_LOGCONFIG(TAG, "  ble_client.app_id: %d", this->parent()->app_id);
  ESP_LOGCONFIG(TAG, "  ble_client.conn_id: %d", this->parent()->get_conn_id());
  LOG_UPDATE_INTERVAL(this)
  ESP_LOGCONFIG(TAG, "  Child components (%d):", this->children_.size());
  for (auto *child : this->children_) {
    ESP_LOGCONFIG(TAG, "    - %s", child->describe().c_str());
  }
}

void PranaBLEHub::dispatch_state_(bool is_ready) {
  for (auto *child : this->children_) {
    child->on_bedjet_state(is_ready);
  }
}

void PranaBLEHub::dispatch_status_() {
  //auto *status = this->codec_->get_status_packet();


  if (!this->is_connected()) {
    ESP_LOGD(TAG, "[%s] Not connected, will not send status.", this->get_name().c_str());
  } else {
    uint32_t now = millis();
    uint32_t diff = now - this->last_notify_;

    if (this->last_notify_ == 0) {
      // This means we're connected and haven't received a notification, so it likely means that the BedJet is off.
      // However, it could also mean that it's running, but failing to send notifications.
      // We can try to unregister for notifications now, and then re-register, hoping to clear it up...
      // But how do we know for sure which state we're in, and how do we actually clear out the buggy state?
      send_update_request();
      ESP_LOGI(TAG, "[%s] Still waiting for first GATT notify event.", this->get_name().c_str());
    } else if (diff > NOTIFY_WARN_THRESHOLD) {
      ESP_LOGW(TAG, "[%s] Last GATT notify was %d seconds ago.", this->get_name().c_str(), diff / 1000);
      send_update_request();
    }

    if (this->timeout_ > 0 && diff > this->timeout_ && this->parent()->enabled) {
      ESP_LOGW(TAG, "[%s] Timed out after %d sec. Retrying...", this->get_name().c_str(), this->timeout_);
      // set_enabled(false) will only close the connection if state != IDLE.
      this->parent()->set_state(espbt::ClientState::CONNECTING);
      this->parent()->set_enabled(false);
      this->parent()->set_enabled(true);
    }
  }
}

void PranaBLEHub::register_child(PranaBLEClient *obj) {
  this->children_.push_back(obj);
  obj->set_parent(this);
}

}  // namespace prana_ble
}  // namespace esphome
