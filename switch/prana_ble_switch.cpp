#include "prana_ble_switch.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32

namespace esphome {
namespace prana_ble {

using namespace esphome::fan;

void PranaBLESwitch::dump_config() { LOG_FAN("", "Prana BLE Fan", this); }
std::string PranaBLESwitch::describe() { return "Prana BLE Fan"; }

void PranaBLESwitch::write_state(bool state) {
  if (!this->parent_->is_connected()) {
    ESP_LOGW(TAG, "Not connected, cannot handle control call yet.");
    return;
  }
  this->publish_state(state);
}

void PranaBLESwitch::on_status(const PranaStatusPacket *data) {
  ESP_LOGV(TAG, "[%s] Handling on_status with data=%p", this->get_name().c_str(), (void *) data);
}

/** Attempts to update the fan device from the last received BedjetStatusPacket.
 *
 * This will be called from #on_status() when the parent dispatches new status packets,
 * and from #update() when the polling interval is triggered.
 *
 * @return `true` if the status has been applied; `false` if there is nothing to apply.
 */
bool PranaBLESwitch::update_status_() {
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

void PranaBLESwitch::update() {
  ESP_LOGD(TAG, "[%s] update()", this->get_name().c_str());
  // TODO: if the hub component is already polling, do we also need to include polling?
  //  We're already going to get on_status() at the hub's polling interval.
  auto result = this->update_status_();
  ESP_LOGD(TAG, "[%s] update_status result=%s", this->get_name().c_str(), result ? "true" : "false");
}


}  // namespace bedjet
}  // namespace esphome

#endif
