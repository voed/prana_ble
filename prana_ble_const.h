#pragma once

#include <set>

namespace esphome {
namespace prana_ble {

static const char *const TAG = "prana_ble";

/// Converts a BedJet fan step to a speed percentage, in the range of 5% to 100%.
inline static uint8_t prana_fan_step_to_speed(const uint8_t fan) {
  //  0 =  5%
  // 19 = 100%
  return 10 * fan + 10;
}
inline static uint8_t prana_fan_speed_to_index(const uint8_t speed) { return speed / 10 - 1; }

static const uint8_t PRANA_MAGIC[] = {0xBE, 0xEF};
static const uint8_t PRANA_CMD_PREFIX = 0x04;
static const uint8_t PRANA_STATE[] = {0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x5A};

struct PranaCmdPacket
{
  uint8_t magic[2];
  uint8_t prefix;
  uint8_t command;

  PranaCmdPacket(uint8_t command)
  {
    std::copy(std::begin(PRANA_MAGIC), std::end(PRANA_MAGIC), std::begin(magic));
    prefix = PRANA_CMD_PREFIX;
    command = command;
  }
} __attribute__((packed));

enum PranaCommand : uint8_t {
  CMD_OFF                 = 0x01,
  CMD_BRIGHTNESS          = 0x02,
  CMD_HEATING             = 0x05,
  CMD_NIGHT_MODE          = 0x06,
  CMD_HIGH_SPEED          = 0x07,
  CMD_FAN_LOCK            = 0x09,
  CMD_FAN_OUT_OFF         = 0x10,
  CMD_FAN_OUT_SPEED_UP    = 0x11,
  CMD_FAN_OUT_SPEED_DOWN  = 0x12,
  CMD_WINTER_MODE         = 0x16,
  CMD_AUTO_MODE           = 0x18,
  CMD_FAN_SPEED_DOWN      = 0x0B,
  CMD_FAN_SPEED_UP        = 0x0C,
  CMD_FAN_IN_SPEED_UP     = 0x0E,
  CMD_FAN_IN_SPEED_DOWN   = 0x0F,
  CMD_FAN_IN_OFF          = 0x0D
};

struct PranaStatusPacket {
  uint8_t magic[2];
  uint8_t command[7];
  uint8_t unused1; // C0
  bool enabled : 8;//[10]
  uint8_t unused2;
  uint8_t brightness : 8;//12
  uint8_t unused3;
  bool heating_on : 8; //14
  uint8_t unused4;
  bool night_mode : 8; //16
  uint8_t unused11;
  uint8_t unknown1[2];
  bool auto_mode : 8; //20
  uint8_t unused5;
  bool fans_locked : 8; //22
  uint8_t unknown2[3];
  uint8_t speed;//26
  uint8_t unused6;
  bool input_enabled : 8;//28
  uint8_t unknown3;
  uint8_t speed_in;//30
  uint8_t unused7;
  bool output_enabled : 8;//32
  uint8_t unused8;
  uint8_t speed_out;//34
  uint8_t unknown4[6];//35-40
  uint8_t unused9;
  bool winter_mode : 8;//42
  uint8_t unknown5[6];//48
  uint8_t temp_in_v1;//49
  uint8_t unknown6;
  short temp_in_v2;//51-52 //short?
  uint8_t unknown7;

/*  struct { //short?
    uint8_t temp_out_v2 : 8;//54 
    uint8_t temp_out_v1 : 8;//55
  } __attribute__((packed)) temp_out;
*/

union
{
  short temp_out_v2;
  struct { //short?
    uint8_t unused11 : 8;//54 
    uint8_t temp_out_v1 : 8;//55
  } __attribute__((packed)) temp_out;
};

  uint8_t unknown8[4];//56-59
  uint8_t humidity;//60
  short co2;//61-62 //short?
  short voc;//63-64
  uint8_t unknown10[13];//65-77
  uint8_t pressure;//78
  uint8_t unknown11[15];//79-93
  uint8_t unused10;//C0
  uint8_t unknown12;//95
  uint8_t voltage;//96
  uint8_t frequency;//97
}  __attribute__((packed));

static const uint8_t PRANA_FAN_SPEED_COUNT = 10;
static const std::string PRANA_FAN_STEP_NAME_STRINGS[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };

}  // namespace prana_ble
}  // namespace esphome
