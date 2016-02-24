#include "option_scene.hpp"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "lobby_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"
#include "json11.hpp"
#include "single_play_info.hpp"

using namespace ui;
using namespace CocosDenshion;
using namespace json11;

Scene* option_scene::createScene() {

  auto scene = Scene::create();
  auto layer = option_scene::create();

  scene->addChild(layer);

  return scene;
}

bool option_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }

  auto audio = SimpleAudioEngine::getInstance();
  audio->playBackgroundMusic("sound/bg1.mp3", true);
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);
    

  auto bg = Sprite::create("background/vs_play_scene.png");
  bg->setPosition(Vec2(center_.x, center_.y));
  this->addChild(bg, 0);

  create_top_ui();
  
  this->scheduleUpdate();
    
  return true;
}

void option_scene::create_top_ui() {

  auto ui_top_bg = Sprite::create("ui/top_single_lobby2.png");
  auto y = center_.y + _play_screen_y/2 - _offset_y+0;
  ui_top_bg->setPosition(Vec2(center_.x, center_.y + _play_screen_y/2 - _offset_y+0));
  this->addChild(ui_top_bg, 0);

  auto font_x = 280;
  auto font_y = center_.y + _play_screen_y/2 - _offset_y+0;
  font_y = font_y + 1;
  auto font_size = 30;

  back_button = ui::Button::create();
  back_button->setTouchEnabled(true);
  back_button->ignoreContentAdaptWithSize(false);
  back_button->setContentSize(Size(128, 128));
  back_button->setScale(0.5f);
  back_button->loadTextures("ui/back2.png", "ui/back2.png");

  back_button->setPosition(Vec2(40, y));

  back_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
       if(type == ui::Widget::TouchEventType::BEGAN) {

        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);
	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	back_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	back_button->runAction(scaleTo2);
        this->scheduleOnce(SEL_SCHEDULE(&option_scene::replace_lobby_scene), 0.2f); 
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	back_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(back_button, 0);
}

void option_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }
  
}

void option_scene::replace_lobby_scene() {
  auto lobby_scene = lobby_scene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.0f, lobby_scene, Color3B(0,255,255)));
}

void option_scene::handle_payload(float dt) {
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
