#ifndef __SINGLE_LOBBY_SCENE_HPP__
#define __SINGLE_LOBBY_SCENE_HPP__

#include <map>
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "network/HttpClient.h"

USING_NS_CC;
using namespace ui;
using namespace cocos2d::network;

class single_lobby_scene : public cocos2d::Layer {
public:

  static cocos2d::Scene* createScene();


  virtual bool init();
  virtual void update(float dt);
    
  void read_single_play_json();
  void parsing_json(std::string read_data);

  void create_top_ui();
  void create_menu();

  void replace_lobby_scene();
  void replace_single_play_scene();

  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Vec2 center_;

  Button* back_button;
  //Button* item_button;
  //std::vector<Button*> start_buttons;
  Vector<Button*> start_buttons;

  //void req_play_info();
  //void handle_req_play_info(HttpClient *sender, HttpResponse *response);

  CREATE_FUNC(single_lobby_scene);

  std::vector<std::string> themes;
  Label* top_money_font;
  Label* bonus;
  Label* bonus_game_time;
  //std::map<std::string, theme_info> theme_infos;
  
  enum LOBBY_STATUS { step0, step1, step2 };
  LOBBY_STATUS start_game;
  void start_action(Vec2 from, Vec2 to);

  CCPoint touchedLocation;

  cocos2d::ui::ScrollView* scrollView;

  void do_demo();
  Vec2 at;
  
  //void touchEvent2(Ref *pSender, Widget::TouchEventType type);
  
};

#endif

