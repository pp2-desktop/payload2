#ifndef __SINGLE_PLAY_INFO_H__
#define __SINGLE_PLAY_INFO_H__

#include <map>
#include "cocos2d.h"
//#include "json11.hpp"

USING_NS_CC;
//using namespace json11;
#define ccsf2(...) CCString::createWithFormat(__VA_ARGS__)->getCString()

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
  bool is_spot_info_in_area(float ux, float uy, float xc, float yc, float r=45.0f);
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

struct play_info_detail {
  int max_stage_cnt;
  int current_stage;
  std::vector<play_info> play_infos;
};

/*
class play_info_md2 {
public:
  std::map<std::string, play_info_detail> play_infos;
  std::string current_theme;

  static play_info_md2& get() {
    static play_info_md2 obj;
    return obj;
  }

};
*/

struct stage_info {
  std::vector<Vec2> spots;
  std::string img;
  int time;
};

struct user_played_info {
  int clear_stage;
  int max_stage_cnt;
  std::vector<stage_info> stage_infos;
};

class single_play2_info {
public:
  single_play2_info();
  ~single_play2_info();

  void reset();
  int get_stage_cnt();
  void set_stage_cnt(int stage_cnt);
  bool increase_stage_cnt();
  int get_max_stage_cnt();
  void set_max_stage_cnt(int max_stage_cnt);

  int max_stage_cnt_;
  int stage_cnt_;
};

class play_info_md {
public:

  static play_info_md& get() {
    static play_info_md obj;
    return obj;
  }

  std::string playing_theme;

  stage_info get_stage_info(std::string theme, int clear_stage);
  int increase_clear_stage(std::string theme);
  
  std::map<std::string, user_played_info> user_played_infos;

  single_play2_info single_play2_info_;
};


#endif
