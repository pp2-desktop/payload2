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
  
  void create_connection_popup();
  void open_connection_popup();
  void close_connection_popup();

  void create_destroy_popup();
  void open_destroy_popup();
  void close_destroy_popup();

  // a selector callback
  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Vec2 center_;


  TextField* textField;

  Button* start_button;
  Button* ready_button;

  Button* back_button;

  bool is_loading;

  Button* connection_confirm_button;
  Button* connection_retry_button;
  Button* connection_cancel_button;
  Sprite* connection_background_popup;
  Label* connection_noti_font;

  Button* destory_confirm_button;
  Sprite* destory_background_popup;
  Label* destory_noti_font;

  CREATE_FUNC(multi_room_scene);
};

#endif
