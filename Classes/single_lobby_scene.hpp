#ifndef __SINGLE_LOBBY_SCENE_HPP__
#define __SINGLE_LOBBY_SCENE_HPP__

#include "cocos2d.h"
#include "network/HttpClient.h"
#include <map>
USING_NS_CC;
using namespace cocos2d::network;

class single_lobby_scene : public cocos2d::Layer {
public:

  static cocos2d::Scene* createScene();


  virtual bool init();
  virtual void update(float dt);
    
  void read_single_play_json();
  void parsing_json(std::string read_data);
  void create_menu();

  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Vec2 center_;


  //void req_play_info();
  //void handle_req_play_info(HttpClient *sender, HttpResponse *response);

  CREATE_FUNC(single_lobby_scene);

  std::vector<std::string> themes;
  Label* coin_font;
  Label* bonus;
  Label* bonus_game_time;
  //std::map<std::string, theme_info> theme_infos;
};

#endif

