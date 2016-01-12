#ifndef __MULTI_LOBBY_SCENE_HPP__
#define __MULTI_LOBBY_SCENE_HPP__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace ui;
//using namespace CocosDenshion;

class multi_lobby_scene : public cocos2d::Layer {
public:

  static cocos2d::Scene* createScene();

  // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
  virtual bool init();
  virtual void update(float dt);

  void create_ui_buttons();
  void replace_lobby_scene();

  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Vec2 center_;

 
  Button* back_button;
  Button* quick_join_button;
  Button* create_room_button;

  CREATE_FUNC(multi_lobby_scene);
};

#endif
