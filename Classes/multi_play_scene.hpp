#ifndef __MULTI_PLAY_SCENE_HPP__
#define __MULTI_PLAY_SCENE_HPP__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "user_info.hpp"

USING_NS_CC;
using namespace ui;
//using namespace CocosDenshion;

class multi_play_scene : public cocos2d::Layer {
public:
  
  static cocos2d::Scene* createScene();

 
  virtual bool init();
  virtual void update(float dt);
    
  void replace_multi_lobby_scene();
  void replace_multi_room_scene();
  void loading_first_stage();
  void loading_next_stage();

  Vec2 change_device_to_img_pos(float x, float y);
  void check_user_input(float x, float y);

  bool is_found_point(int index, stage& game_stage);
  int check_point(float x, float y);
  bool is_point_in_area(float ux, float uy, float xc, float yc, float r=45.0f);

  void check_point_req(int index);

  void action_correct(Vec2 point);
  void action_other_correct(Vec2 point);
  void action_incorrect(float x, float y);
  

  void victory_game_end();
  void defeat_game_end();
  void win_stage_end();
  void lose_stage_end();


  void found_point(bool found_point, Vec2 point);

  void set_point_index(Vec2 point);

  Vec2 change_img_to_device_pos(bool is_left, float x, float y);

  void open_block();
  void close_block();

  void create_stage_status();


  // a selector callback
  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Size visible_size;
  Vec2 origin;
  Vec2 center;

  int stage_count;
  int max_stage_count;
  int point_count;
  int max_point_count;

  bool is_playing;
  bool is_end_game;

  Sprite* left_block;
  Sprite* right_block;

  Label* stage_count_font;
  Label* max_stage_count_font;
  Label* point_count_font;
  Label* max_point_count_font;


  Sprite* left_img;
  Sprite* right_img;

  std::vector<stage> stages;

  std::vector<Sprite*> correct_spots;
  std::vector<Sprite*> other_correct_spots;

  TextField* textField;

  CREATE_FUNC(multi_play_scene);
};

#endif
