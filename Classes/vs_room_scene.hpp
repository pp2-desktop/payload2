#ifndef __VS_ROOM_SCENE_HPP__
#define __VS_ROOM_SCENE_HPP__

#include "cocos2d.h"
USING_NS_CC;

class vs_room_scene : public cocos2d::Layer
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
    
  CREATE_FUNC(vs_room_scene);
};

#endif
