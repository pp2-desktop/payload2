#ifndef __ASSETS_SCENE_HPP__
#define __ASSETS_SCENE_HPP__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "extensions/cocos-ext.h"

USING_NS_CC;
using namespace ui;
USING_NS_CC_EXT;
//using namespace CocosDenshion;

class assets_scene : public cocos2d::Layer {
public:
  
  static cocos2d::Scene* createScene();


  virtual bool init();
  virtual void update(float dt);

  void replace_lobby_scene();


  Vec2 center_;
  TextField* textField;

  CREATE_FUNC(assets_scene);


  cocos2d::extension::AssetsManagerEx* _am;
  cocos2d::extension::EventListenerAssetsManagerEx* _amListener;
};

#endif

