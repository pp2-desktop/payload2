#include "SimpleAudioEngine.h"
#include "multi_room_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"
#include "multi_lobby_scene.hpp"
#include "multi_play_scene.hpp"
#include "assets_scene.hpp"
#include "resource_md.hpp"
#include "single_play_info.hpp"
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
  auto background = Sprite::create("background/vs_play_scene.png");
  background->setPosition(Vec2(center_.x, center_.y));
  this->addChild(background, 0);

  is_loading = false;
  start_button = nullptr;
  ready_button = nullptr;

  if(user_info::get().room_info_.is_master) {
    start_button = ui::Button::create();
    start_button->setTouchEnabled(true);
    start_button->ignoreContentAdaptWithSize(false);
    start_button->setContentSize(Size(286.0f, 120.0f));
    start_button->loadTextures("ui/game_start.png", "ui/game_start.png");
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


    auto debug_font = Label::createWithTTF("방장", "fonts/nanumb.ttf", 40);
    debug_font->setPosition(center_);
    this->addChild(debug_font, 0);

  } else {
    ready_button = ui::Button::create();
    ready_button->setTouchEnabled(true);
    ready_button->ignoreContentAdaptWithSize(false);
    ready_button->setContentSize(Size(286.0f, 126.0f));
    ready_button->loadTextures("ui/game_ready.png", "ui/game_ready.png");

    ready_button->setPosition(Vec2(1100.0f, center_.y-200.0f));

    ready_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
	if(type == ui::Widget::TouchEventType::BEGAN) {
	  auto audio = SimpleAudioEngine::getInstance();
	  audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);

	  auto scaleTo = ScaleTo::create(0.1f, 1.3f);
	  auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	  auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	  ready_button->runAction(seq2);
          ready_button->setEnabled(false);

	  Json payload = Json::object {
	    { "type", "ready_game_noti" }
	  };

	  connection::get().send2(payload);
        
	}
      });
     
    this->addChild(ready_button, 0);


    auto debug_font = Label::createWithTTF("상대", "fonts/nanumb.ttf", 40);
    debug_font->setPosition(center_);
    this->addChild(debug_font, 0);
  }

  back_button = ui::Button::create();
  back_button->setTouchEnabled(true);
  back_button->ignoreContentAdaptWithSize(false);
  back_button->setContentSize(Size(128, 128));
  back_button->setScale(0.5f);
  back_button->loadTextures("ui/back2.png", "ui/back2.png");

  auto y = center_.y + _play_screen_y/2 - _offset_y+0;
  back_button->setPosition(Vec2(40, y));

  back_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {

        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);

	auto scaleTo = ScaleTo::create(0.1f, 0.5f);
	back_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {

	if(is_loading) return;

	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	back_button->runAction(seq2);
        this->scheduleOnce(SEL_SCHEDULE(&multi_room_scene::replace_multi_lobby_scene), 0.2f); 

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 0.5f);
	back_button->runAction(scaleTo);
      }

    });
     
  this->addChild(back_button, 0);

  
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
  auto multi_play_scene = multi_play_scene::createScene();
  Director::getInstance()->replaceScene(multi_play_scene);
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
      // 데이터 넘어오니까 먼저 파싱함
      user_info::get().room_info_.stages.clear();

      auto result = payload["result"].bool_value();
      if(result) {

        auto stages = payload["stage_infos"].array_items();
        for(auto _stage : stages) {
          stage game_stage;
          auto img = _stage["img"].string_value();
          game_stage.img = img;

          std::deque<float> points;
          for (auto &k : _stage["points"].array_items()) {
            points.push_back(static_cast<float>(k.int_value()));
          }

          auto size = points.size();
          for(unsigned i=0; i<size;) {
            auto x = points.front();
            points.pop_front();

            auto y = points.front();
            points.pop_front();

            game_stage.hidden_points.push_back(Vec2(x, y));
            i=i+2;
          }
          user_info::get().room_info_.stages.push_back(game_stage);
        }

        // replace scene
        // 5초 후에 시작한다고 3 2 1 액션 주고 씬넘아감
        replace_multi_play_scene();
        //this->scheduleOnce(SEL_SCHEDULE(&multi_room_scene::replace_multi_play_scene), 0.0f);
      } else {

      }

   
    } else if(type == "ready_game_noti") {
      // ready 누르고 나면 무조건 대기중
      start_button->setEnabled(true);
    } else if(type == "join_opponent_noti") {
      
    } else if(type == "leave_opponent_noti") {
      start_button->setEnabled(false);
    } else {
      CCLOG("[error] handler 없음");
    }
}
