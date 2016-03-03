#ifndef __SETTING_SCENE_HPP__
#define __SETTING_SCENE_HPP__

#include <map>
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "extensions/cocos-ext.h"

USING_NS_CC;
using namespace ui;
USING_NS_CC_EXT;

class setting_scene : public cocos2d::Layer {
public:

  static cocos2d::Scene* createScene();

  virtual bool init();
  virtual void update(float dt);

  void create_ui_top();

  void replace_lobby_scene();

  void menuCloseCallback(cocos2d::Ref* pSender);

  void handle_payload(float dt);

  void create_ui_buttons();
  void value_background_sound_changed(Ref* sender, Control::EventType controlEvent);
  void value_effect_sound_changed(Ref* sender, Control::EventType controlEvent);

  void create_reset_sp();

  Vec2 center_;

  Button* back_button;
  Button* reset_sp_button;
  Label* reset_sp_font;

  CREATE_FUNC(setting_scene);

  cocos2d::ui::ScrollView* scrollView;

};

#endif
