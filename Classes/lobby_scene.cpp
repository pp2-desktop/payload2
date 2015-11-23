#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "lobby_scene.hpp"
#include "lobby_multi_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"

using namespace ui;
using namespace CocosDenshion;

Scene* lobby_scene::createScene() {

  auto scene = Scene::create();
  auto layer = lobby_scene::create();

  scene->addChild(layer);

  return scene;
}

// on "init" you need to initialize your instance
bool lobby_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }

  // 커넥터 초기화
  if(!connection::get().get_is_connected()) {
    connection::get().create("ws://t.05day.com:8080/echo");
    connection::get().connect();
  }
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);
    
  auto closeItem = MenuItemImage::create(
					 "CloseNormal.png",
					 "CloseSelected.png",
					 CC_CALLBACK_1(lobby_scene::menuCloseCallback, this));
  closeItem->setScale(2.0f, 2.0f);
  closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2-20, origin.y + closeItem->getContentSize().height/2+15));

  // create menu, it's an autorelease object
  auto menu = Menu::create(closeItem, NULL);
  menu->setPosition(Vec2::ZERO);
  this->addChild(menu, 1);

  // 
  auto background = Sprite::create("background/lobby_scene.jpg");
  background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background, 0);

  // 싱글 버튼 추가
  auto single_button = Button::create("ui/normal_btn.png", "ui/pressed_btn.png", "ui/disabled_btn.png");
  single_button->setTitleText("SinglePlay");
  single_button->setTitleFontSize(24);
  single_button->setScale(2.0f, 2.0f);
  single_button->setPosition(Vec2(center_.x+425, center_.y+260));

  single_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
	  handle_sound(sound_type::BUTTON_PRESSED);
	  break;

	case ui::Widget::TouchEventType::ENDED:

	  break;
	default:
	  break;
	}
    });
  this->addChild(single_button);

  // 멀티 버튼 추가
  auto multi_button = Button::create("ui/normal_btn.png", "ui/pressed_btn.png", "ui/disabled_btn.png");
  multi_button->setTitleText("MultiPlay");
  multi_button->setTitleFontSize(24);
  multi_button->setScale(2.0f, 2.0f);
  multi_button->setPosition(Vec2(center_.x+425, center_.y+150));
  multi_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      auto scene = lobby_multi_scene::createScene();

      switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
	  handle_sound(sound_type::BUTTON_PRESSED);
	  break;
	case ui::Widget::TouchEventType::ENDED:
	  Director::getInstance()->replaceScene(TransitionFade::create(1, scene, Color3B(0,255,255)));
	  //std::cout << "Button 1 clicked" << std::endl;
	  break;
	default:
	  break;
	}
    });
  this->addChild(multi_button);



  /*
  ActionInterval* lens = Lens3D::create(1, Size(32,24), Vec2(100,180), 150);
  ActionInterval* waves = Waves3D::create(1, Size(15,10), 18, 15);
  auto nodeGrid = NodeGrid::create();
  nodeGrid->addChild(background);
  nodeGrid->runAction(Sequence::create(waves, lens, NULL));
  this->addChild(nodeGrid);
  */


  
  this->scheduleUpdate();
    
  return true;
}

void lobby_scene::menuCloseCallback(Ref* pSender) {
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}

void lobby_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }
  
  //CCLOG("update");
}

void lobby_scene::handle_payload(float dt) {
    Json payload = connection::get().q.front();
    connection::get().q.pop_front();
    std::string type = payload["type"].string_value();

    if(type == "connection_notify") {
      CCLOG("[debug] 접속 성공");
      connection::get().send2(Json::object {
	  { "type", "login_req" }
	});

    } else if(type == "disconnection_notify") {
      CCLOG("[debug] 접속 큰킴");

    } else if(type == "login_res") {
      user_info::get().uid = payload["uid"].string_value();
      CCLOG("uid: %s", user_info::get().uid.c_str());
    } else {
      CCLOG("[error] handler 없음");
    }
}

void lobby_scene::handle_sound(sound_type type) {
  auto audio = SimpleAudioEngine::getInstance();

  if(type == sound_type::BUTTON_PRESSED) {
  audio->playEffect("sound/button_pressed.mp3", false, 1.0f, 1.0f, 1.0f);
  }
}
