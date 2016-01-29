#ifndef __MULTI_ROOM_SCENE_HPP__
#define __MULTI_ROOM_SCENE_HPP__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace ui;
//using namespace CocosDenshion;

class multi_room_scene : public cocos2d::Layer {
public:
  
  static cocos2d::Scene* createScene();

 
  virtual bool init();
  virtual void update(float dt);
    
  void replace_multi_play_scene();
  void replace_multi_lobby_scene();

  // a selector callback
  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Vec2 center_;


  TextField* textField;

  Button* start_button;
  Button* ready_button;

  Button* back_button;

  bool is_loading;

  CREATE_FUNC(multi_room_scene);
};

#endif
