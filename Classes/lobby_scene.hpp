#ifndef __LOBBY_SCENE_HPP__
#define __LOBBY_SCENE_HPP__

#include "cocos2d.h"
USING_NS_CC;

class lobby_scene : public cocos2d::Layer
{
public:
  // there's no 'id' in cpp, so we recommend returning the class instance pointer
  static cocos2d::Scene* createScene();

  // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
  virtual bool init();
  virtual void update(float dt);
    
  // a selector callback
  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Vec2 center_;
  // implement the "static create()" method manually
  CREATE_FUNC(lobby_scene);
};

#endif

