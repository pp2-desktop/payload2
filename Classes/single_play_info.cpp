#include "single_play_info.hpp"

spot_info::spot_info() {
  pos = Vec2(0, 0);
  is_find = false;
}

spot_info::~spot_info() {
  pos = Vec2(0, 0);
  is_find = false;
}

play_info::play_info() {
  play_time_sec = 30;
}

play_info::~play_info() {

}

bool play_info::check_find_spot(spot_info& si) {

  return true;
}
