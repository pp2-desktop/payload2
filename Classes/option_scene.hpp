#ifndef __OPTION_SCENE_HPP__
#define __OPTION_SCENE_HPP__

#include <map>
#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace ui;

class option_scene : public cocos2d::Layer {
public:

  static cocos2d::Scene* createScene();

  virtual bool init();
  virtual void update(float dt);
    
  void create_top_ui();

  void replace_lobby_scene();

  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  Vec2 center_;

  Button* back_button;

  CREATE_FUNC(option_scene);

  cocos2d::ui::ScrollView* scrollView;

};

#endif

