#ifndef __SINGLE_PLAY_SCENE_HPP__
#define __SINGLE_PLAY_SCENE_HPP__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
USING_NS_CC;
using namespace cocos2d::ui;

class single_play_scene : public cocos2d::Layer {
public:

  static cocos2d::Scene* createScene();


  virtual bool init();
  virtual void update(float dt);

  void create_ready(float move_to_sec, float offset, std::string img);
  void create_go();
  void ready_go();
  void create_timer();
  void update_timer();
  void increase_timer(int sec);
  CCProgressTimer* progressTimeBar_;

  void check_end_play();
  void check_tmp_timer();

  Vec2 change_device_to_img_pos(float x, float y);
  void check_user_input(float x, float y);
  void render_found_spot(float x, float y);

  void action_correct(int index);
  void action_incorrect(float x, float y);
  Vec2 change_img_to_device_pos(bool is_left, float x, float y);
  void check_win_play();
  void create_stage_status();
    
  // a selector callback
  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Size visible_size;
  Vec2 origin;
  Vec2 center;
  CREATE_FUNC(single_play_scene);

  Button* pause_button;
  int current_stage;
  int max_stage_cnt;
  int max_spot_cnt;
  int find_spot_cnt;


  Label* top_left_stage_font;
  Label* top_right_stage_font;
  Label* top_left_spot_font;
  Label* top_right_spot_font;
};

#endif
