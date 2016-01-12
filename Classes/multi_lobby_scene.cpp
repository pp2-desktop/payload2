#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "multi_lobby_scene.hpp"
#include "lobby_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"
#include "json11.hpp"

using namespace ui;
using namespace CocosDenshion;
using namespace json11;

Scene* multi_lobby_scene::createScene() {

  auto scene = Scene::create();
  auto layer = multi_lobby_scene::create();

  scene->addChild(layer);

  return scene;
}

bool multi_lobby_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }

  auto audio = SimpleAudioEngine::getInstance();
  audio->playBackgroundMusic("sound/bg1.mp3", true);
  audio->setBackgroundMusicVolume(0.5f);
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);

  create_ui();
  
  this->scheduleUpdate();
    
  return true;
}

void create_ui_buttons() {

  quick_join_button = ui::Button::create();
  quick_join_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  quick_join_button->loadTextures("ui/resume_button.png", "ui/presume_button.png");
  quick_join_button->ignoreContentAdaptWithSize(false);
  quick_join_button->setContentSize(Size(64, 64));
  quick_join_button->setPosition(Vec2(center_.x, center_.y+20.0f));
  quick_join_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.2f, 1.3f);
	quick_join_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	quick_join_button->runAction(scaleTo2);

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	quick_join_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(quick_join_button, 0);

}

void multi_lobby_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }
  
}

void multi_lobby_scene::handle_payload(float dt) {
    Json payload = connection::get().q.front();
    connection::get().q.pop_front();
    std::string type = payload["type"].string_value();

    if(type == "connection_notify") {
      CCLOG("[debug] 접속 성공");
      connection::get().send2(Json::object {
	  { "type", "login_req" }
	});

    }  else {
      CCLOG("[error] handler 없음");
    }
}
