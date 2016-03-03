#ifndef __LOBBY_SCENE_HPP__
#define __LOBBY_SCENE_HPP__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "network/HttpClient.h"

USING_NS_CC;
using namespace ui;
//using namespace CocosDenshion;

class lobby_scene : public cocos2d::Layer {
public:
  enum class sound_type {
    BUTTON_PRESSED = 0,
    END
  };
  // there's no 'id' in cpp, so we recommend returning the class instance pointer
  static cocos2d::Scene* createScene();

  // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
  virtual bool init();
  virtual void update(float dt);
    
  void replace_single_play2_scene();
  void replace_multi_lobby_scene();
  void replace_ranking_scene();
  void replace_setting_scene();


  void create_guest_account();
  void login_req(std::string uid, std::string name, std::string password);

  void create_multi_popup();
  void open_multi_popup();
  void close_multi_popup();

  void create_connection_popup();
  void open_connection_popup(int type=0);
  void close_connection_popup();

  void create_update_popup();
  void open_update_popup();
  void close_update_popup();

  void create_facebook_popup();
  void open_facebook_popup();
  void close_facebook_popup();

  void facebook_login();
  void guest_login();

  void tmp();
  void onRequestImgCompleted(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response);
  void close_game();

  void create_ui_font();

  void create_complete_popup();
  void open_complete_popup();
  void close_complete_popup();

  // a selector callback
  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  void handle_sound(sound_type type);
  Vec2 center_;

  TextField* textField;

  Button* sp_button;
  Button* mp_button;
  Button* ranking_button;
  Button* setting_button;
  Button* quit_button;

  Button* facebook_login_button;
  Button* guest_login_button;
  Button* close_popup_button;
  Sprite* background_popup;

  Button* connection_confirm_button;
  Button* connection_retry_button;
  Button* connection_cancel_button;
  Sprite* connection_background_popup;
  Label* connection_noti_font;

  Button* update_confirm_button;
  Sprite* update_background_popup;
  Label* update_noti_font;

  Button* facebook_confirm_button;
  Sprite* facebook_background_popup;
  Label* facebook_noti_font;

  bool is_requesting;
  bool is_popup_on;

  Texture2D texture;

  bool is_multi_play;
  int max_stage_cnt;

  Button* complete_confirm_button;
  Sprite* complete_background_popup;
  Label* complete_noti_font;

  Label* sp_status_font;

  CREATE_FUNC(lobby_scene);
};

#endif
