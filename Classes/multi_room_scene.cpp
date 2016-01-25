#include "SimpleAudioEngine.h"
#include "multi_room_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"
#include "multi_lobby_scene.hpp"
#include "assets_scene.hpp"
#include "resource_md.hpp"
//#include "single_play_scene.hpp"
using namespace CocosDenshion;

Scene* multi_room_scene::createScene() {

  auto scene = Scene::create();
  auto layer = multi_room_scene::create();

  scene->addChild(layer);

  return scene;
}

// on "init" you need to initialize your instance
bool multi_room_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);
    
  //
  /* 
  auto background = Sprite::create("background/lobby_scene.png");
  background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background, 0);
  */


  start_button = nullptr;
  ready_button = nullptr;

  if(user_info::get().room_info_.is_master) {
    start_button = ui::Button::create();
    start_button->setTouchEnabled(true);
    start_button->ignoreContentAdaptWithSize(false);
    start_button->setContentSize(Size(200.0f, 200.0f));
    start_button->loadTextures("ui/sp_button.png", "ui/sp_button.png");
    start_button->setEnabled(false);

    start_button->setPosition(Vec2(1100.0f, center_.y-200.0f));

    start_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
	if(type == ui::Widget::TouchEventType::BEGAN) {
	  auto audio = SimpleAudioEngine::getInstance();
	  audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);

	  auto scaleTo = ScaleTo::create(0.1f, 1.3f);
	  auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	  auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	  start_button->runAction(seq2);

	  Json payload = Json::object {
	    { "type", "start_game_req" }
	  };

	  connection::get().send2(payload);
	}
      });
     
    this->addChild(start_button, 0);
  } else {
    ready_button = ui::Button::create();
    ready_button->setTouchEnabled(true);
    ready_button->ignoreContentAdaptWithSize(false);
    ready_button->setContentSize(Size(200.0f, 200.0f));
    ready_button->loadTextures("ui/mp_button.png", "ui/mp_button.png");

    ready_button->setPosition(Vec2(1100.0f, center_.y-200.0f));

    ready_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
	if(type == ui::Widget::TouchEventType::BEGAN) {
	  auto audio = SimpleAudioEngine::getInstance();
	  audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);

	  auto scaleTo = ScaleTo::create(0.1f, 1.3f);
	  auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	  auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	  ready_button->runAction(seq2);

	  Json payload = Json::object {
	    { "type", "ready_noti" }
	  };

	  connection::get().send2(payload);
        
	}
      });
     
    this->addChild(ready_button, 0);
  }

  
  this->scheduleUpdate();
    
  return true;
}

void multi_room_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }
  
  //CCLOG("update");
}

void multi_room_scene::replace_multi_play_scene() {
  /*
  auto single_lobby_scene = single_lobby_scene::createScene();
  Director::getInstance()->replaceScene(single_lobby_scene);
  */
}

void multi_room_scene::replace_multi_lobby_scene() {
  auto multi_lobby_scene = multi_lobby_scene::createScene();
  Director::getInstance()->replaceScene(multi_lobby_scene);
}

void multi_room_scene::handle_payload(float dt) {
    Json payload = connection::get().q.front();
    connection::get().q.pop_front();
    std::string type = payload["type"].string_value();

    if(type == "connection_notify") {
      CCLOG("[debug] 접속 성공");
     

    } else if(type == "disconnection_notify") {
      CCLOG("[debug] 접속 큰킴");
      
    } else if(type == "start_game_res") {
      // 둘다 다른 씬으로 넘어감
   
    } else if(type == "ready_noti") {
      // ready 누르고 나면 무조건 대기중
    } else {
      CCLOG("[error] handler 없음");
    }
}
