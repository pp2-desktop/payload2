#ifndef __SINGLE_PLAY_SCENE_HPP__
#define __SINGLE_PLAY_SCENE_HPP__

#include "cocos2d.h"
USING_NS_CC;

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
    
  // a selector callback
  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Size visible_size;
  Vec2 origin;
  Vec2 center;
  CREATE_FUNC(single_play_scene);
};

#endif

