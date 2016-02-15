#ifndef __MULTI_ROOM_SCENE_HPP__
#define __MULTI_ROOM_SCENE_HPP__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "network/HttpClient.h"

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


  struct user_profile {
    Sprite* img;
    Label* name_font;
    Label* score_font;
    Label* score_front_font;
    Label* win_count_font;
    Label* win_font;

    Label* lose_count_font;
    Label* lose_font;
    Label* ranking_font;
    Label* ranking_front_font;
    Texture2D texture;

    user_profile() {
      img = nullptr;
      name_font = nullptr;
      score_font = nullptr;
      score_front_font = nullptr;
      win_count_font = nullptr;
      win_font = nullptr;
      lose_count_font = nullptr;
      lose_font = nullptr;
      ranking_font = nullptr;
      ranking_front_font = nullptr;
    };

    ~user_profile() {};
  };

  user_profile master_profile;
  user_profile opponent_profile;

  Sprite* master_profile_background;
  Sprite* opponent_profile_background;

  Label* earn_score_font;
  Label* lose_score_font;

  Button* kick_button;

  void create_master_profile(std::string facebookid, std::string name, int score, int win_count, int lose_count, int ranking);
  void create_opponent_profile(std::string facebookid, std::string name, int score, int win_count, int lose_count, int ranking);

  void start_img_req(std::string id="100005347304902", bool is_master = true);
  void on_request_master_img_completed(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response);
  void on_request_opponent_img_completed(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response);

  bool is_master_img_requesting;
  bool is_opponent_img_requesting;


  CREATE_FUNC(multi_room_scene);
};

#endif
