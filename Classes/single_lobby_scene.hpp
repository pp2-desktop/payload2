#ifndef __SINGLE_LOBBY_SCENE_HPP__
#define __SINGLE_LOBBY_SCENE_HPP__

#include "cocos2d.h"
USING_NS_CC;

class single_lobby_scene : public cocos2d::Layer {
public:

  static cocos2d::Scene* createScene();


  virtual bool init();
  virtual void update(float dt);
    

  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Vec2 center_;

  CREATE_FUNC(single_lobby_scene);
};

#endif

