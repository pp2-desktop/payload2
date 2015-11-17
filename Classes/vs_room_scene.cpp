#include "vs_room_scene.hpp"
#include "lobby_multi_scene.hpp"
#include "vs_play_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"

Scene* vs_room_scene::createScene()
{
  // 'scene' is an autorelease object
  auto scene = Scene::create();
    
  // 'layer' is an autorelease object
  auto layer = vs_room_scene::create();

  // add layer as a child to scene
  scene->addChild(layer);

  // return the scene
  return scene;
}

// on "init" you need to initialize your instance
bool vs_room_scene::init()
{
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }

  is_next_scene = false;
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);

  // 백그라운드
  auto background = Sprite::create("background/vs_room_scene.jpg");
  background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background, 0);

    
  // 돌아가기 버튼 추가
  auto back_button = Button::create("ui/b1.png", "ui/b2.png", "ui/b2.png");
  back_button->setScale(1.2f, 1.2f);
  back_button->setPosition(Vec2(65, visibleSize.height-40));
  back_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {

      switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
	  break;

	case ui::Widget::TouchEventType::ENDED:
	  connection::get().send2( Json::object {
	      { "type", "leave_room_req" }
	    });
	  break;

	default:
	  break;
	}
    });
  this->addChild(back_button);

  // xx
  prepare_button = Button::create("ui/normal_btn.png", "ui/pressed_btn.png", "ui/disabled_btn.png");
  if(user_info::get().room_info_ptr->is_master_) {
    prepare_button->setTitleText("Wait");
    prepare_button->setEnabled(false);
    //prepare_button->set
  } else {
    prepare_button->setTitleText("Ready");
  }
  prepare_button->setTitleFontSize(24);
  prepare_button->setScale(2.0f, 2.0f);
  prepare_button->setPosition(Vec2(center_.x+430, center_.y-280));
  prepare_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
	  break;
	case ui::Widget::TouchEventType::ENDED:
	  if(user_info::get().room_info_ptr->is_master_) {
	    connection::get().send2(Json::object {
	      { "type", "start_vs_game_req" }
	      });
	    prepare_button->setEnabled(true);
	  } else {
	    connection::get().send2(Json::object {
		{ "type", "opponent_ready_notify" }
	      });
	  }
	  break;
	default:
	  break;
	}
    });
  this->addChild(prepare_button);

  this->scheduleUpdate();   

  return true;
}

void vs_room_scene::update(float dt) {

  if(!connection::get().q.empty() && !is_next_scene) {
    handle_payload(dt);
  } 
  
  // next
  

  //CCLOG("update");
}

void vs_room_scene::handle_payload(float dt) {

  Json payload = connection::get().q.front();
  connection::get().q.pop_front();

  std::string type = payload["type"].string_value();

  if (type == "connection_notify") {

  } else if (type == "disconnection_notify") {
    CCLOG("[debug] 접속 큰킴");
    prepare_button->setTitleText("Disconnecton");
    user_info::get().destroy_room(); 
    // after reconnect prev scene

  } else if (type == "leave_room_res") {
    user_info::get().destroy_room();
    auto scene = lobby_multi_scene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(1, scene, Color3B(255,0,255)));

  } else if (type == "join_opponent_notify") {
    std::string uid = payload["uid"].string_value();
    join_opponent_notify(uid);

  } else if (type == "master_leave_notify") {
    master_leave_notify();
    
  } else if (type == "opponent_leave_notify") {
    std::string uid = payload["uid"].string_value();
    opponent_leave_notify(uid);

  } else if (type == "opponent_ready_notify") {
    opponent_ready_notify();
  } else if (type == "start_vs_game_res") {

    CCLOG("게임 시작");
    is_next_scene = true;
    auto scene2 = vs_play_scene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(1, scene2, Color3B(125,125,0)));
  } else if (type == "update_alive_res") {
    connection::get().send2(Json::object {
	{ "type", "update_alive_req" }
      });
    CCLOG("[debug] update_alive_req 보냄");
  } else if (type == "kick_user_notify") {
    CCLOG("[debug] 킥 당함");
    user_info::get().destroy_room();
    auto scene = lobby_multi_scene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(1, scene, Color3B(255,0,255)));

  } else {
    CCLOG("[error] vs_room_scene handler 없음");
  }
}

void vs_room_scene::join_opponent_notify(std::string uid) {
  CCLOG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
  CCLOG("상대 들어옴");
  CCLOG("들어온 유저 uid:  %s", uid.c_str());
  prepare_button->setEnabled(false);
  prepare_button->setTitleText("Start");
}

void vs_room_scene::master_leave_notify() {
  CCLOG("방장이 떠나서 방장됨");
  prepare_button->setEnabled(false);
  prepare_button->setTitleText("Wait");
}

void vs_room_scene::opponent_leave_notify(std::string uid) {
  CCLOG("떠난 유저 uid:  %s", uid.c_str());
  prepare_button->setEnabled(false);
  prepare_button->setTitleText("Wait");
}

void vs_room_scene::opponent_ready_notify() {
  CCLOG("상대편 준비 완료");
  prepare_button->setEnabled(true);
  prepare_button->setTitleText("Start");
}
