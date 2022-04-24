#include "prana_ble.h"

#ifdef USE_ESP32

namespace esphome {
namespace prana_ble {



void PranaBLE::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                        esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      if (param->open.status == ESP_GATT_OK) {
        ESP_LOGI(TAG, "Connected successfully!");
      }
      break;
    }

    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGW(TAG, "Disconnected!");
      break;
    }

    case ESP_GATTC_SEARCH_CMPL_EVT: {
      ESP_LOGW(TAG, "ESP_GATTC_SEARCH_CMPL_EVT");
      this->char_handle_ = 0;
      auto *chr = this->parent()->get_characteristic(service_uuid_, sensors_char_uuid_);
      if (chr == nullptr) {
        ESP_LOGW(TAG, "No sensor read characteristic found at service %s char %s", service_uuid_.to_string().c_str(),
                 sensors_char_uuid_.to_string().c_str());
        break;
      }
      this->char_handle_ = chr->handle;
      set_notify_(true);
      
      
      write_notify_message_();
      this->update();
      this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;

      
      //request_read_values_();
      break;
    }

    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      ESP_LOGW(TAG, "REG_FOR_NOTIFY");


      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      ESP_LOGW(TAG, "NOTIFY len %d", param->notify.value_len);
      if(param->notify.value_len > 0)
        ESP_LOGW(TAG, "NOTIFY 0 %d", param->notify.value[0]);
      
      if(param->notify.value_len > 10)
        ESP_LOGW(TAG, "NOTIFY 0 %d", param->notify.value[10]);
      if (param->notify.is_notify){
          ESP_LOGW(TAG, "ESP_GATTC_NOTIFY_EVT, receive notify value:");
      }else{
          ESP_LOGW(TAG, "ESP_GATTC_NOTIFY_EVT, receive indicate value:");
      }

      break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT: {
      ESP_LOGW(TAG, "Write char at handle %d, status=%d", param->write.handle, param->write.status);
      
    }
    case ESP_GATTC_READ_CHAR_EVT: {
      ESP_LOGW(TAG, "Reading char at handle %d, status=%d", param->read.handle, param->read.status);
      if(param->read.value_len > 0)
        ESP_LOGW(TAG, "Data %d, len %d", param->read.value[0], param->read.value_len);
      if (param->read.conn_id != this->parent()->conn_id)
        break;
      ESP_LOGW(TAG, "Reading char at handle %d, status=%d", param->read.handle, param->read.status);
      if (param->read.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Data %d, len %d", param->read.value[10], param->read.value_len);
        break;
      }


      if (param->read.handle == this->char_handle_) {
        read_sensors_(param->read.value, param->read.value_len);
      }
      break;
    }

    default:
      break;
  }
}

void PranaBLE::read_sensors_(uint8_t *value, uint16_t value_len) {


    ESP_LOGD(TAG, "Value len: %d", value_len);
    /*ESP_LOGD(TAG, "is_on: %d", value[10]);
    ESP_LOGD(TAG, "brightness: %d", value[10]);
    ESP_LOGD(TAG, "speed_in: %d", value[30] / 10);
    ESP_LOGD(TAG, "speed_out: %d", value[34] / 10);*/

  // Example data
  // [13:08:47][D][radon_eye_rd200:107]: result bytes: 5010 85EBB940 00000000 00000000 2200 2500 0000
  /*ESP_LOGV(TAG, "result bytes: %02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X %02X%02X %02X%02X",
           value[0], value[1], value[2], value[3], value[4], value[5], value[6], value[7], value[8], value[9],
           value[10], value[11], value[12], value[13], value[14], value[15], value[16], value[17], value[18],
           value[19]);*/

 /* if (value[0] != 0x50) {
    // This isn't a sensor reading.
    return;
  }

  // Convert from pCi/L to Bq/m³
  constexpr float convert_to_bwpm3 = 37.0;

  RadonValue radon_value;
  radon_value.chars[0] = value[2];
  radon_value.chars[1] = value[3];
  radon_value.chars[2] = value[4];
  radon_value.chars[3] = value[5];
  float radon_now = radon_value.number * convert_to_bwpm3;
  if (is_valid_radon_value_(radon_now)) {
    radon_sensor_->publish_state(radon_now);
  }

  radon_value.chars[0] = value[6];
  radon_value.chars[1] = value[7];
  radon_value.chars[2] = value[8];
  radon_value.chars[3] = value[9];
  float radon_day = radon_value.number * convert_to_bwpm3;

  radon_value.chars[0] = value[10];
  radon_value.chars[1] = value[11];
  radon_value.chars[2] = value[12];
  radon_value.chars[3] = value[13];
  float radon_month = radon_value.number * convert_to_bwpm3;

  if (is_valid_radon_value_(radon_month)) {
    ESP_LOGV(TAG, "Radon Long Term based on month");
    radon_long_term_sensor_->publish_state(radon_month);
  } else if (is_valid_radon_value_(radon_day)) {
    ESP_LOGV(TAG, "Radon Long Term based on day");
    radon_long_term_sensor_->publish_state(radon_day);
  }

  ESP_LOGV(TAG, "  Measurements (Bq/m³) now: %0.03f, day: %0.03f, month: %0.03f", radon_now, radon_day, radon_month);

  ESP_LOGV(TAG, "  Measurements (pCi/L) now: %0.03f, day: %0.03f, month: %0.03f", radon_now / convert_to_bwpm3,
           radon_day / convert_to_bwpm3, radon_month / convert_to_bwpm3);

  // This instance must not stay connected
  // so other clients can connect to it (e.g. the
  // mobile app).*/
  //parent()->set_enabled(false);
}

bool PranaBLE::is_valid_radon_value_(float radon) { return radon > 0.0 and radon < 37000; }

void PranaBLE::update() {
  if (this->node_state != esp32_ble_tracker::ClientState::ESTABLISHED) {
    if (!parent()->enabled) {
      ESP_LOGW(TAG, "Reconnecting to device");
      parent()->set_enabled(true);
      parent()->connect();
    } else {
      ESP_LOGW(TAG, "Connection in progress");
    }
  }
  else
    write_query_message_();
}

uint8_t PranaBLE::set_notify_(const bool enable) {
  uint8_t status;
  if (enable) {
    status = esp_ble_gattc_register_for_notify(this->parent_->gattc_if, this->parent_->remote_bda,
                                               this->char_handle_);
    if (status) {
      ESP_LOGW(TAG, " esp_ble_gattc_register_for_notify failed, status=%d", status);
    }
  } else {
    status = esp_ble_gattc_unregister_for_notify(this->parent_->gattc_if, this->parent_->remote_bda,
                                                 this->char_handle_);
    if (status) {
      ESP_LOGW(TAG, " esp_ble_gattc_unregister_for_notify failed, status=%d",  status);
    }
  }
  ESP_LOGV(TAG, " set_notify: enable=%d; result=%d", enable, status);
  return status;
}

void PranaBLE::write_notify_message_() {
  ESP_LOGW(TAG, "writing 0x01 to write service");
  //uint8_t request[] = { 0xBE, 0xEF, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x5A };
  uint8_t request[] = { 0x01, 0x00 };
  auto status = esp_ble_gattc_write_char(this->parent()->gattc_if, this->parent()->conn_id, this->char_handle_,
                                               sizeof(request), request, ESP_GATT_WRITE_TYPE_NO_RSP,
                                               ESP_GATT_AUTH_REQ_NONE);
  if (status) {
    ESP_LOGW(TAG, "Error sending write request for sensor, status=%d", status);
  }
}

void PranaBLE::write_query_message_() {
  ESP_LOGW(TAG, "writing 0x50 to write service");
  //uint8_t request[] = { 0xBE, 0xEF, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x5A };
  uint8_t request[] = { 0xBE, 0xEF, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x5A }; //{ 0xBE, 0xEF, 0x04, 0x0C };
  auto status = esp_ble_gattc_write_char(this->parent()->gattc_if, this->parent()->conn_id, this->char_handle_,
                                               sizeof(request), request, ESP_GATT_WRITE_TYPE_NO_RSP,
                                               ESP_GATT_AUTH_REQ_NONE);
  if (status) {
    ESP_LOGW(TAG, "Error sending write request for sensor, status=%d", status);
  }
}

void PranaBLE::request_read_values_() {
  auto status = esp_ble_gattc_read_char(this->parent()->gattc_if, this->parent()->conn_id, this->char_handle_,
                                        ESP_GATT_AUTH_REQ_NONE);
  if (status) {
    ESP_LOGW(TAG, "Error sending read request for sensor, status=%d", status);
  }
}

void PranaBLE::dump_config() {
  ESP_LOGW(TAG, "Prana BLE");
}




PranaBLE::PranaBLE()
    : PollingComponent(10000),
      service_uuid_(esp32_ble_tracker::ESPBTUUID::from_raw(SERVICE_UUID)),
      sensors_char_uuid_(esp32_ble_tracker::ESPBTUUID::from_raw(RW_CHARACTERISTIC_UUID)) {}

}  // namespace prana_ble
}  // namespace esphome

#endif  // USE_ESP32