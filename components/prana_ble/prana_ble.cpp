#include "prana_ble.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32

namespace esphome {
namespace prana_ble {


void Bedjet::dump_config() {
  ESP_LOGW("", "BedJet Climate");
}

void Bedjet::setup() {
      parent()->connect();

#ifdef USE_TIME
  this->setup_time_();
#endif
}


void Bedjet::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
  switch (event) {

    case ESP_GATTC_CONNECT_EVT: {
      ESP_LOGW(TAG, "[] ESP_GATTC_CONNECT_EVT");

    }
    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGV(TAG, "Disconnected: reason=%d", param->disconnect.reason);
      this->status_set_warning();
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      ESP_LOGW(TAG, "ESP_GATTC_SEARCH_CMPL_EVT");
      auto *chr = this->parent_->get_characteristic(BEDJET_SERVICE_UUID, BEDJET_COMMAND_UUID);
      if (chr == nullptr) {
        ESP_LOGW(TAG, "No control service found at device, not a BedJet..?");
        break;
      }
      this->char_handle_cmd_ = chr->handle;

      chr = this->parent_->get_characteristic(BEDJET_SERVICE_UUID, BEDJET_STATUS_UUID);
      if (chr == nullptr) {
        ESP_LOGW(TAG, " No status service found at device, not a BedJet..?");
        break;
      }

      this->char_handle_status_ = chr->handle;
      // We also need to obtain the config descriptor for this handle.
      // Otherwise once we set node_state=Established, the parent will flush all handles/descriptors, and we won't be
      // able to look it up.
      auto *descr = chr->get_descriptor(0x2902);//this->parent_->get_config_descriptor(this->char_handle_status_);
      if (descr == nullptr) {
        ESP_LOGW(TAG, "No config descriptor for status handle 0x%x. Will not be able to receive status notifications",
                 this->char_handle_status_);
      } else if (descr->uuid.get_uuid().len != ESP_UUID_LEN_16 ||
                 descr->uuid.get_uuid().uuid.uuid16 != ESP_GATT_UUID_CHAR_CLIENT_CONFIG) {
        ESP_LOGW(TAG, "Config descriptor 0x%x (uuid %s) is not a client config char uuid", this->char_handle_status_,
                 descr->uuid.to_string().c_str());
      } else {
        this->config_descr_status_ = descr->handle;
      }

      
      //this->write_notify_config_descriptor_(true);
      //write_bedjet_packet_();
      ESP_LOGD(TAG, "Services complete: obtained char handles. 0x%x %s ", this->char_handle_status_, descr->uuid.to_string().c_str());
      

      
      this->set_notify_(true);
      this->write_notify_config_descriptor_(true);

      //this->write_notify_config_descriptor_(true);
      this->node_state = espbt::ClientState::ESTABLISHED;

      break;
    }
/*    case ESP_GATTC_WRITE_DESCR_EVT: {
      if (param->write.status != ESP_GATT_OK) {
        // ESP_GATT_INVALID_ATTR_LEN
        ESP_LOGW(TAG, "Error writing descr at handle 0x%04d, status=%d", param->write.handle, param->write.status);
        break;
      }
      // [16:44:44][V][bedjet:279]: [JOENJET] Register for notify event success: h=0x002a s=0
      // This might be the enable-notify descriptor? (or disable-notify)
      ESP_LOGV(TAG, "Write to handle 0x%04x status=%d", param->write.handle,
               param->write.status);


      break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT: {
      ESP_LOGW(TAG, "Writing char at handle 0x%04d, status=%d", param->write.handle, param->write.status);
      if (param->write.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Error writing char at handle 0x%04d, status=%d", param->write.handle, param->write.status);
        break;
      }
      if (param->write.handle == this->char_handle_cmd_) {
        if (this->force_refresh_) {
          // Command write was successful. Publish the pending state, hoping that notify will kick in.
          //this->publish_state();
        }
      }
      break;
    }
    case ESP_GATTC_READ_CHAR_EVT: {
      ESP_LOGW(TAG, "Reading char at handle %d, status=%d", param->read.handle, param->read.status);
      if (param->read.conn_id != this->parent_->conn_id)
        break;
      if (param->read.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Error reading char at handle %d, status=%d", param->read.handle, param->read.status);
        break;
      }
      if (param->read.handle == this->char_handle_status_) {
        ESP_LOGW(TAG, "Read len %d", param->read.value_len);
        // This is the additional packet that doesn't fit in the notify packet.
        //this->codec_->decode_extra(param->read.value, param->read.value_len);
      } 
      break;
    }*/
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {

      ESP_LOGW(TAG, "ESP_GATTC_REG_FOR_NOTIFY_EVT");
      // This event means that ESP received the request to enable notifications on the client side. But we also have to
      // tell the server that we want it to send notifications. Normally BLEClient parent would handle this
      // automatically, but as soon as we set our status to Established, the parent is going to purge all the
      // service/char/descriptor handles, and then get_config_descriptor() won't work anymore. There's no way to disable
      // the BLEClient parent behavior, so our only option is to write the handle anyway, and hope a double-write
      // doesn't break anything.

      if (param->reg_for_notify.handle != this->char_handle_status_) {
        ESP_LOGW(TAG, "Register for notify on unexpected handle 0x%04x, expecting 0x%04x", param->reg_for_notify.handle, this->char_handle_status_);
        break;
      }

      
      this->last_notify_ = 0;
      this->force_refresh_ = true;
      write_bedjet_packet_();
      break;
    }
    case ESP_GATTC_UNREG_FOR_NOTIFY_EVT: {
      // This event is not handled by the parent BLEClient, so we need to do this either way.
      if (param->unreg_for_notify.handle != this->char_handle_status_) {
        ESP_LOGW(TAG, "Unregister for notify on unexpected handle 0x%04x, expecting 0x%04x", param->unreg_for_notify.handle, this->char_handle_status_);
        break;
      }

      this->write_notify_config_descriptor_(false);
      this->last_notify_ = 0;
      // Now we wait until the next update() poll to re-register notify...
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      ESP_LOGW(TAG, "NOTIFY data %s, len %d", param->notify.value, param->notify.value_len);
      if (param->notify.handle != this->char_handle_status_) {
        ESP_LOGW(TAG, " Unexpected notify handle, wanted %04X, got %04X", this->char_handle_status_, param->notify.handle);
        break;
      }
      
      // FIXME: notify events come in every ~200-300 ms, which is too fast to be helpful. So we
      //  throttle the updates to once every MIN_NOTIFY_THROTTLE (5 seconds).
      //  Another idea would be to keep notify off by default, and use update() as an opportunity to turn on
      //  notify to get enough data to update status, then turn off notify again.


      auto status = esp_ble_gattc_read_char(this->parent_->gattc_if, this->parent_->conn_id,
                                                this->char_handle_status_, ESP_GATT_AUTH_REQ_NONE);
          if (status) {
            ESP_LOGI(TAG, " Unable to read extended status packet");
          }


 /*      uint32_t now = millis();
      auto delta = now - this->last_notify_;

      if (this->last_notify_ == 0 || delta > MIN_NOTIFY_THROTTLE || this->force_refresh_) {

        this->last_notify_ = now;



       if (needs_extra) {
          // this means the packet was partial, so read the status characteristic to get the second part.
          auto status = esp_ble_gattc_read_char(this->parent_->gattc_if, this->parent_->conn_id,
                                                this->char_handle_status_, ESP_GATT_AUTH_REQ_NONE);
          if (status) {
            ESP_LOGI(TAG, "[%s] Unable to read extended status packet", this->get_name().c_str());
          }
        }

        if (this->force_refresh_) {
          // If we requested an immediate update, do that now.
          this->update();
          this->force_refresh_ = false;
        }
      }
      */

      break;
    }
    default:
      ESP_LOGVV(TAG, "gattc unhandled event: enum=%d", event);
      break;
  }
}

/** Reimplementation of BLEClient.gattc_event_handler() for ESP_GATTC_REG_FOR_NOTIFY_EVT.
 *
 * This is a copy of ble_client's automatic handling of `ESP_GATTC_REG_FOR_NOTIFY_EVT`, in order
 * to undo the same on unregister. It also allows us to maintain the config descriptor separately,
 * since the parent BLEClient is going to purge all descriptors once we set our connection status
 * to `Established`.
 */
uint8_t Bedjet::write_notify_config_descriptor_(bool enable) {
  auto handle = this->char_handle_status_;
  if (handle == 0) {
    ESP_LOGW(TAG, "No descriptor found for notify of handle 0x%x", this->char_handle_status_);
    return -1;
  }

  // NOTE: BLEClient uses `uint8_t*` of length 1, but BLE spec requires 16 bits.
  uint8_t notify_en[] = {0, 0};
  notify_en[0] = enable;
  auto status =
      esp_ble_gattc_write_char(this->parent_->gattc_if, this->parent_->conn_id, handle, sizeof(notify_en),
                                     &notify_en[0], ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  if (status) {
    ESP_LOGW(TAG, "esp_ble_gattc_write_char_descr error, status=%d", status);
    return status;
  }
  ESP_LOGD(TAG, "wrote notify=%s to status config 0x%04x", enable ? "true" : "false",
           handle);
  return ESP_GATT_OK;
}


/** Writes one BedjetPacket to the BLE client on the BEDJET_COMMAND_UUID. */
uint8_t Bedjet::write_bedjet_packet_(){//(BedjetPacket *pkt) {
  if (this->node_state != espbt::ClientState::ESTABLISHED) {
    if (!this->parent_->enabled) {
      ESP_LOGI(TAG, "Cannot write packet: Not connected, enabled=false");
    } else {
      ESP_LOGW(TAG, "Cannot write packet: Not connected");
    }
    return -1;
  }
  uint8_t cmd[] = { 0xBE, 0xEF, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x5A };
  ESP_LOGW(TAG, "Writing command %s %i", cmd, sizeof(cmd));
  
  auto status = esp_ble_gattc_write_char(this->parent_->gattc_if, this->parent_->conn_id, this->char_handle_cmd_,
                                         sizeof(cmd), cmd, ESP_GATT_WRITE_TYPE_NO_RSP,
                                         ESP_GATT_AUTH_REQ_NONE);

  return status;
}

/** Configures the local ESP BLE client to register (`true`) or unregister (`false`) for status notifications. */
uint8_t Bedjet::set_notify_(const bool enable) {
  uint8_t status;
  if (enable) {
    status = esp_ble_gattc_register_for_notify(this->parent_->gattc_if, this->parent_->remote_bda,
                                               this->char_handle_status_);
    if (status) {
      ESP_LOGW(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", status);
    }
  } else {
    status = esp_ble_gattc_unregister_for_notify(this->parent_->gattc_if, this->parent_->remote_bda,
                                                 this->char_handle_status_);
    if (status) {
      ESP_LOGW(TAG, "esp_ble_gattc_unregister_for_notify failed, status=%d", status);
    }
  }
  ESP_LOGW(TAG, "set_notify: enable=%d; result=%d", enable, status);
  return status;
}


BedJet::update() {
  if (this->node_state != esp32_ble_tracker::ClientState::ESTABLISHED) {
    if (!parent()->enabled) {
      ESP_LOGW(TAG, "Reconnecting to device");
      parent()->set_enabled(true);
      parent()->connect();
    } else {
      ESP_LOGW(TAG, "Connection in progress");
    }
  }



/*    uint32_t now = millis();
    uint32_t diff = now - this->last_notify_;

    if (this->last_notify_ == 0) {
      // This means we're connected and haven't received a notification, so it likely means that the BedJet is off.
      // However, it could also mean that it's running, but failing to send notifications.
      // We can try to unregister for notifications now, and then re-register, hoping to clear it up...
      // But how do we know for sure which state we're in, and how do we actually clear out the buggy state?

      ESP_LOGI(TAG, "Still waiting for first GATT notify event.");
      this->set_notify_(false);
    } else if (diff > NOTIFY_WARN_THRESHOLD) {
      ESP_LOGW(TAG, "Last GATT notify was %d seconds ago.", diff / 1000);
    }

    if (this->timeout_ > 0 && diff > this->timeout_ && this->parent()->enabled) {
      ESP_LOGW(TAG, "Timed out after %d sec. Retrying...", this->timeout_);
      this->parent()->set_enabled(false);
      this->parent()->set_enabled(true);
    }*/
  
}

}  // namespace bedjet
}  // namespace esphome

#endif
