#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "../prana_ble_child.h"
#include "../prana_ble_const.h"

#ifdef USE_ESP32

namespace esphome {
namespace prana_ble {

class PranaBLESensors : public PranaBLEClient, public PollingComponent {
 public:
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  /* BedJetClient status update */
  void on_status(const PranaStatusPacket *data) override;
  void on_bedjet_state(bool is_ready) override{};
  std::string describe() override;

  void set_temp_in(sensor::Sensor *temp_in) { temp_in_ = temp_in; }
  void set_temp_out(sensor::Sensor *temp_out) { temp_out_ = temp_out; }
  void set_humidity(sensor::Sensor *humidity) { humidity_ = humidity_; }
  void set_pressure(sensor::Sensor *pressure) { pressure_ = pressure; }
  void set_tvoc(sensor::Sensor *tvoc) { tvoc_ = tvoc; }
  void set_co2(sensor::Sensor *co2) { co2_ = co2; }
  void set_voltage(sensor::Sensor *voltage) { voltage_ = voltage; }
  void set_frequency(sensor::Sensor *frequency) { frequency_ = frequency; }

 protected:
  sensor::Sensor *temp_in_{nullptr};
  sensor::Sensor *temp_out_{nullptr};
  sensor::Sensor *humidity_{nullptr};
  sensor::Sensor *pressure_{nullptr};
  sensor::Sensor *tvoc_{nullptr};
  sensor::Sensor *co2_{nullptr};
  sensor::Sensor *voltage_{nullptr};
  sensor::Sensor *frequency_{nullptr};

 private:
  bool update_status_();
};

}  // namespace bedjet
}  // namespace esphome

#endif
