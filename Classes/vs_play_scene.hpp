#ifndef __VS_PLAY_SCENE_HPP__
#define __VS_PLAY_SCENE_HPP__

#include "cocos2d.h"
#include "json11.hpp"

USING_NS_CC;
using namespace json11;

enum VS_PLAY_WINNER_TYPE { MASTER, OPPONENT, UNKNOWN };

struct round_info {
  std::string left_img;
  std::string right_img;
  std::vector<Vec2> spots; 
  std::vector<bool> find_spots; 
  VS_PLAY_WINNER_TYPE winner;
};

class vs_play_scene : public cocos2d::Layer {
public:
  enum class sound_type {
    BUTTON_PRESSED = 0,
    END
  };

  static cocos2d::Scene* createScene();

  virtual bool init();
  virtual void update(float dt);


  void handle_payload(float dt);
  void round_info_res(Json round_infos);
  void start_round_res(Json payload);
  void end_round_res(Json payload);
  void score_vs_round(Json payload);
  void score_vs_play(Json payload);
  void end_vs_play_res(Json payload);
  bool check_spot(float x, float y);
  bool is_point_in_circle(float xa, float ya, float xc, float yc, float r);

  void handle_sound(sound_type type);
  Vec2 center_;

  std::vector<round_info> round_infos_;

  unsigned int stage_cnt_;
  unsigned int max_stage_cnt_ ;
  
  float offset_y;
  float offset_x;

  void pre_loading_resources();

  CREATE_FUNC(vs_play_scene);
};

#endif

