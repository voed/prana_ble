#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/hal.h"
#include "esphome/components/fan/fan.h"
#include "../prana_ble_child.h"
#include "../prana_ble_const.h"

#ifdef USE_ESP32

namespace esphome {
namespace prana_ble {

class PranaBLEFan : public fan::Fan, public PranaBLEClient, public PollingComponent {
 public:
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  /* BedJetClient status update */
  void on_status(const PranaStatusPacket *data) override;
  void on_bedjet_state(bool is_ready) override{};
  std::string describe() override;

  fan::FanTraits get_traits() override { return fan::FanTraits(false, true, false, PRANA_FAN_SPEED_COUNT); }

 protected:
  void control(const fan::FanCall &call) override;

 private:
  void reset_state_();
  bool update_status_();
};

}  // namespace bedjet
}  // namespace esphome

#endif
