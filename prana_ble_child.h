#pragma once

#include "esphome/core/helpers.h"
#include "prana_ble_hub.h"
#include "prana_ble_const.h"
namespace esphome {
namespace prana_ble {

// Forward declare BedJetHub
class PranaBLEHub;

class PranaBLEClient : public Parented<PranaBLEHub> {
 public:
  virtual void on_status(const PranaStatusPacket *data) = 0;
  virtual void on_bedjet_state(bool is_ready) = 0;

 protected:
  friend PranaBLEHub;
  virtual std::string describe() = 0;
};

}  // namespace bedjet
}  // namespace esphome
