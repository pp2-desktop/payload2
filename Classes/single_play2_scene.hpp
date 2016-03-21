#ifndef __SINGLE_PLAY2_SCENE_HPP__
#define __SINGLE_PLAY2_SCENE_HPP__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "network/HttpClient.h"
#include "user_info.hpp"
#if (CC_TARGET_PLATFORM != CC_PLATFORM_LINUX)
#include "Sdkbox/Sdkbox.h"
#include "PluginIAP/PluginIAP.h"
#endif

USING_NS_CC;
using namespace ui;
//using namespace CocosDenshion;

#if (CC_TARGET_PLATFORM != CC_PLATFORM_LINUX)
class single_play2_scene : public cocos2d::Layer, public sdkbox::IAPListener {
#else
class single_play2_scene : public cocos2d::Layer {
#endif
public:
  static cocos2d::Scene* createScene();

  // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
  virtual bool init();
  virtual void update(float dt);

  // a selector callback
  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  void create_ui_top();
  void create_ui_timer();
  void update_timer();
  void increase_timer(int percentage);

  void create_ready(float move_to_sec, float offset, std::string img);
  void create_go();
  void ready_go();

  void create_block();
  void open_block();
  void close_block();

  void create_status_font();

  void start_get_img(bool is_left, std::string img);
  void on_request_left_img_completed(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response);
  void on_request_right_img_completed(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response);
  void loading_stage(std::string img);
  void destroy_stage();

  void start_game();
  void win_game();
  void end_game();
  void complete_stages();

  void check_end_play();

  void check_user_input(float x, float y);
  int check_point(float x, float y);
  bool is_point_in_area(float ux, float uy, float xc, float yc, float r=60.0f);
  void check_point_req(int index);
  void action_correct(Vec2 point);
  void action_incorrect(float x, float y);
  void action_hint(Vec2 point);

  void action_win_game();
  void release_incorrect_action();
  bool is_found_point(int index, stage& game_stage);
  Vec2 change_img_to_device_pos(bool is_left, float x, float y);
  Vec2 change_device_to_img_pos(float x, float y);

  void replace_single_play2_scene();
  void replace_lobby_scene();

  void create_pause_popup();
  void open_pause_popup();
  void close_pause_popup();

  void create_game_end_popup();
  void open_game_end_popup();
  void close_game_end_popup();

  void create_complete_popup();
  void open_complete_popup();
  void close_complete_popup();

  void create_connection_popup();
  void open_connection_popup();
  void close_connection_popup();

  void create_stage_status();

  void set_is_pause_false();
  void set_is_store_on_false();

  void clear_hint();

  void create_store_popup();
  void open_store_popup();
  void close_store_popup();

  void create_iap_popup();
  void open_iap_popup();
  void close_iap_popup();

  void add_hint_item(int hint_count);
    
  // sdkbox for iap

  virtual void onInitialized(bool ok) override;
  virtual void onSuccess(sdkbox::Product const& p) override;
  virtual void onFailure(sdkbox::Product const& p, const std::string &msg) override;
  virtual void onCanceled(sdkbox::Product const& p) override;
  virtual void onRestored(sdkbox::Product const& p) override;
  virtual void onProductRequestSuccess(std::vector<sdkbox::Product> const &products) override;
  virtual void onProductRequestFailure(const std::string &msg) override;
  void updateIAP(const std::vector<sdkbox::Product>& products);
  void onRestoreComplete(bool ok, const std::string &msg);

  Size visible_size;
  Vec2 origin;
  Vec2 center;


  Button* pause_button;
  Button* hint_button;
  Sprite* hint;
  Label* hint_status_font;
  Button* add_hint_button;

  ProgressTimer* progressTimeBar_;
  Sprite* timeBar;

  Sprite* left_block;
  Sprite* right_block;

  int img_complete_cnt;
  Texture2D left_texture;
  Texture2D right_texture;

  Label* resource_status_font;

  Sprite* left_img;
  Sprite* right_img;

  std::vector<Sprite*> correct_spots;
  std::set<int> hint_indexs;

  stage game_stage;

  bool is_end_play;
  bool is_incorrect_action;
  bool is_playing;
  bool is_hint_on;

  bool is_pause;

  int create_stage_status_cnt;

  Button* connection_confirm_button;
  Sprite* connection_background_popup;
  Label* connection_noti_font;

  Sprite* pause_background;
  Button* resume_button;
  Button* back_button;

  Button* complete_confirm_button;
  Sprite* complete_background_popup;
  Label* complete_noti_font;

  Sprite* game_end_background;
  Button* retry_button;

  Label* stage_cnt_font;
  Label* max_stage_cnt_font;
  Label* point_cnt_font;
  Label* max_point_cnt_font;
  
  int stage_cnt;
  int max_stage_cnt;
  int point_cnt;
  int max_point_cnt;

  bool is_hurry_up;

  Sprite* store_background;
  Button* hint10_button;
  Button* hint25_button;
  Button* hint99_button;
  Button* close_store_button;
  bool is_store_on;
  bool is_iap_on;

  Button* iap_confirm_button;
  Sprite* iap_background_popup;
  Label* iap_noti_font;

  CREATE_FUNC(single_play2_scene);
};

#endif
