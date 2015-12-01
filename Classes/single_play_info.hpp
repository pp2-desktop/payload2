#ifndef __SINGLE_PLAY_INFO_H__
#define __SINGLE_PLAY_INFO_H__

#include "cocos2d.h"
USING_NS_CC;

static auto _offset_x = 2.0f;
static auto _offset_y = 37.0f;

static auto _play_screen_x = 1334;
static auto _play_screen_y = 750;

struct spot_info {
  Vec2 pos;
  bool is_find;
  spot_info();
  ~spot_info();
};

class play_info {
public:
  std::string img;
  std::vector<spot_info> spot_infos;
  float play_time_sec;

  bool check_find_spot(spot_info& si);
  bool add_spot_info(float x, float y);

  bool check_spot_info(float x, float y);
  bool is_spot_info_in_area(float ux, float uy, float xc, float yc, float r=35.0f);

  play_info();
  ~play_info();

  static play_info& get() {
    static play_info obj;
    return obj;
  }
};

#endif
