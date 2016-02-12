#ifndef __MULTI_LOBBY_SCENE_HPP__
#define __MULTI_LOBBY_SCENE_HPP__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace ui;
//using namespace CocosDenshion;

struct chat_msg {
  std::string name;
  std::string msg;
  chat_msg() {}
  ~chat_msg() {}
};

struct player {
  std::string name;
  int score;
};

struct room {
  int id;
  std::string title;
  std::string password;
  std::vector<player> players;
  bool is_full;
  Label* label_ptr;
  Sprite* sprite_ptr;
  Button* button_ptr;

  room() : label_ptr(nullptr), sprite_ptr(nullptr), button_ptr(nullptr) { is_full = false; }
  ~room() {}
};

class multi_lobby_scene : public cocos2d::Layer {
public:

  static cocos2d::Scene* createScene();

  // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
  virtual bool init();
  virtual void update(float dt);

  void create_ui_buttons();
  void create_ui_room_info();
  void resize_ui_room_info();
  void create_ui_chat_info();
  void resize_ui_chat_info();

  void replace_lobby_scene();

  void create_room_req(std::string title, std::string password="");
  void join_room_req(int rid);

  void create_ui_top();

  void add_room(int rid, std::string title, std::string password);
  void remove_room(int rid);

  std::string get_quick_room_title();

  void create_connection_popup();
  void open_connection_popup();
  void close_connection_popup();

  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Vec2 center_;


  cocos2d::ui::ScrollView* scrollView;
  cocos2d::ui::ScrollView* ChatScrollView;
  std::vector<Label*> chat_fonts;

  Button* back_button;
  Button* quick_join_button;
  Button* create_room_button;


  std::vector<room> rooms;
  std::deque<chat_msg> chat_msgs;
  void dummy_data();
  TextField* textField;
  Button* send_button;

  bool is_requesting;
  bool is_quick_requesting;

  Button* connection_confirm_button;
  Button* connection_retry_button;
  Button* connection_cancel_button;
  Sprite* connection_background_popup;
  Label* connection_noti_font;

  CREATE_FUNC(multi_lobby_scene);
};

#endif
