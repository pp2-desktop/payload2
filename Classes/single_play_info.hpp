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

  //bool check_find_spot(spot_info& si);
  bool add_spot_info(float x, float y);

  int check_spot_info(float x, float y);
  bool is_spot_info_in_area(float ux, float uy, float xc, float yc, float r=35.0f);
  spot_info get_spot_info(int index);

  play_info();
  ~play_info();

  void reset() {
    spot_infos.clear();
  }

  static play_info& get() {
    static play_info obj;
    return obj;
  }
};

class play_info_md {
public:
  std::string theme;
  int max_stage_cnt;
  int current_stage;
  std::vector<play_info> play_infos;

  void reset() {
    max_stage_cnt = 0;
    current_stage = 0;
    play_infos.clear();
  }

  bool complete_stage(int stage);

  void set_theme(std::string);
  std::string get_theme();

  play_info& get_play_info(int index);

  static play_info_md& get() {
    static play_info_md obj;
    return obj;
  }

};

#endif
