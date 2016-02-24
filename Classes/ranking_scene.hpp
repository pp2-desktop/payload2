#ifndef __RANKING_SCENE_HPP__
#define __RANKING_SCENE_HPP__

#include <map>
#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace ui;

struct ranking_info {
  std::string name;
  int win_count;
  int lose_count;
  int score;

  Sprite* sprite_ptr;
};

class ranking_scene : public cocos2d::Layer {
public:

  static cocos2d::Scene* createScene();

  virtual bool init();
  virtual void update(float dt);
    
  void create_ui_top();
  void create_ui_ranking_info();
  void create_ui_game_info();

  void create_connection_popup();
  void open_connection_popup();
  void close_connection_popup();

  void create_loading_status_font();
  

  void replace_lobby_scene();

  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Vec2 center_;

  Button* back_button;

  CREATE_FUNC(ranking_scene);

  cocos2d::ui::ScrollView* scrollView;

  std::vector<ranking_info> ranking_infos_;

  Button* connection_confirm_button;
  Sprite* connection_background_popup;
  Label* connection_noti_font;

  Label* loading_status_font;
  
};

#endif

