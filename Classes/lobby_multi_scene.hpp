#ifndef __LOBBY_MULTI_SCENE_HPP__
#define __LOBBY_MULTI_SCENE_HPP__

#include "cocos2d.h"
#include <atomic>
USING_NS_CC;

class lobby_multi_scene : public cocos2d::Layer
{
public:
  enum class sound_type {
    BUTTON_PRESSED = 0,
    END
  };

  static cocos2d::Scene* createScene();

  // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
  virtual bool init();
  virtual void update(float dt);
    
  // a selector callback
  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);
  void join_room_res(bool result, int rid, bool is_master);

  void handle_sound(sound_type type);
  Vec2 center_;

  std::atomic<bool> is_next_scene;

  CREATE_FUNC(lobby_multi_scene);
};

#endif

