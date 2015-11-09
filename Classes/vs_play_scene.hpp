#ifndef __VS_PLAY_SCENE_HPP__
#define __VS_PLAY_SCENE_HPP__

#include "cocos2d.h"
#include "json11.hpp"

USING_NS_CC;
using namespace json11;

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

  void handle_sound(sound_type type);
  Vec2 center_;

  CREATE_FUNC(vs_play_scene);
};

#endif

