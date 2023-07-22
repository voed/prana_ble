#include "prana_ble_sensors.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32

namespace esphome {
namespace prana_ble {


void PranaBLESensors::dump_config() { ESP_LOGCONFIG("", "Prana BLE Sensors", this); }
std::string PranaBLESensors::describe() { return "Prana BLE Sensors"; }


void PranaBLESensors::on_status(const PranaStatusPacket *data) {
  ESP_LOGV(TAG, " Handling on_status with data=%p",  (void *) data);
  if(data == nullptr)
    return;
  
  //TODO: add v2 support
  if(this->temp_in_ != nullptr && is_temp_valid(data->temp_in_v1))
    this->temp_in_->publish_state(data->temp_in_v1);

  if(this->temp_out_ != nullptr && is_temp_valid(data->temp_out_v1))
    this->temp_out_->publish_state(data->temp_out_v1);

  if(this->humidity_ != nullptr && data->humidity < 100)
    this->humidity_->publish_state(data->humidity);

  //TODO: add validation
  if(this->pressure_ != nullptr)
    this->pressure_->publish_state(data->pressure);

  if(this->tvoc_ != nullptr)
    this->tvoc_->publish_state(data->tvoc);

  if(this->co2_ != nullptr)
    this->co2_->publish_state(data->co2);

  if(this->voltage_ != nullptr)
    this->voltage_->publish_state(data->voltage);
    
  if(this->frequency_ != nullptr)
    this->frequency_->publish_state(data->frequency);
}

bool is_temp_valid(int temp)
{
  //TODO: test this
  if(abs(temp) > 50)
    return false;

  return true;
}
/** Attempts to update the fan device from the last received BedjetStatusPacket.
 *
 * This will be called from #on_status() when the parent dispatches new status packets,
 * and from #update() when the polling interval is triggered.
 *
 * @return `true` if the status has been applied; `false` if there is nothing to apply.
 */
bool PranaBLESensors::update_status_() {
  if (!this->parent_->is_connected())
    return false;
  if (!this->parent_->has_status())
    return false;
  auto *status = this->parent_->get_status_packet();

  if (status == nullptr)
    return false;

  this->on_status(status);
  return true;
}

void PranaBLESensors::update() {
  // TODO: if the hub component is already polling, do we also need to include polling?
  //  We're already going to get on_status() at the hub's polling interval.
  auto result = this->update_status_();
  ESP_LOGD(TAG, " update_status result=%s", result ? "true" : "false");
}


}  // namespace bedjet
}  // namespace esphome

#endif
