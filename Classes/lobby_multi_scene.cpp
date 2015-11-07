#include "lobby_multi_scene.hpp"
#include "ui/CocosGUI.h"
#include "lobby_scene.hpp"
#include "vs_room_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"

using namespace ui;

Scene* lobby_multi_scene::createScene()
{
  // 'scene' is an autorelease object
  auto scene = Scene::create();
    
  // 'layer' is an autorelease object
  auto layer = lobby_multi_scene::create();

  // add layer as a child to scene
  scene->addChild(layer);

  // return the scene
  return scene;
}

// on "init" you need to initialize your instance
bool lobby_multi_scene::init()
{
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);

  /////////////////////////////
  // 2. add a menu item with "X" image, which is clicked to quit the program
  //    you may modify it.

  // add a "close" icon to exit the progress. it's an autorelease object
  auto closeItem = MenuItemImage::create(
					 "CloseNormal.png",
					 "CloseSelected.png",
					 CC_CALLBACK_1(lobby_multi_scene::menuCloseCallback, this));
    
  closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2, origin.y + closeItem->getContentSize().height/2));

	

  // 백그라운드
  auto background = Sprite::create("background/lobby_multi_scene.png");
  background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background, 0);

    
  // 돌아가기 버튼 추가
  auto back_button = Button::create("ui/b1.png", "ui/b2.png", "ui/b2.png");
  back_button->setScale(1.2f, 1.2f);
  back_button->setPosition(Vec2(65, visibleSize.height-40));
  //back_button->setPosition(10, visibleSize.height/1);
  back_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      auto scene = lobby_scene::createScene();

      switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
	  break;
	case ui::Widget::TouchEventType::ENDED:
	  Director::getInstance()->replaceScene(TransitionFade::create(1, scene, Color3B(255,0,255)));
	  //std::cout << "Button 1 clicked" << std::endl;
	  break;
	default:
	  break;
	}
    });
  this->addChild(back_button);


  // 뒷배경
  auto left_on = Sprite::create("ui/on.png");
  left_on->setPosition(Vec2(center_.x-280, center_.y));
  this->addChild(left_on, 0);

  auto right_on = Sprite::create("ui/on.png");
  right_on->setPosition(Vec2(center_.x+280, center_.y));
  this->addChild(right_on, 0);

  auto left_label = Label::createWithTTF("VS Mode", "fonts/Marker Felt.ttf", 40);
  left_label->setPosition(Vec2(center_.x-280, center_.y+240));
  this->addChild(left_label, 1);

  auto right_label = Label::createWithTTF("Team Mode", "fonts/Marker Felt.ttf", 40);
  right_label->setPosition(Vec2(center_.x+280, center_.y+240));
  this->addChild(right_label, 1);


  // vs모드 친구초대 버튼 추가
  auto vs_invite_button = Button::create("ui/normal_btn.png", "ui/pressed_btn.png", "ui/disabled_btn.png");
  vs_invite_button->setTitleText("Invite Friend");
  vs_invite_button->setTitleFontSize(24);
  vs_invite_button->setScale(2.0f, 2.0f);
  vs_invite_button->setPosition(Vec2(center_.x-300, center_.y+50));
  vs_invite_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {

      switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:

	  break;
	case ui::Widget::TouchEventType::ENDED:


	  //std::cout << "Button 1 clicked" << std::endl;
	  break;
	default:
	  break;
	}
    });
  this->addChild(vs_invite_button);


  // vs모드 빠른입장 버튼 추가
  auto vs_quick_button = Button::create("ui/normal_btn.png", "ui/pressed_btn.png", "ui/disabled_btn.png");
  vs_quick_button->setTitleText("Quick Play");
  vs_quick_button->setTitleFontSize(24);
  vs_quick_button->setScale(2.0f, 2.0f);
  vs_quick_button->setPosition(Vec2(center_.x-300, center_.y-75));
  vs_quick_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {

      Json payload = Json::object {
	{ "type", "join_room_req" }
      };

      switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
	  break;
	case ui::Widget::TouchEventType::ENDED:
	  connection::get().send2(payload);
	  break;
	default:
	  break;
	}
    });
  this->addChild(vs_quick_button);

    

  this->scheduleUpdate();   

  return true;
}

void lobby_multi_scene::menuCloseCallback(Ref* pSender) {
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}

void lobby_multi_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  } 
  
  // next
  

  //CCLOG("update");
}

void lobby_multi_scene::handle_payload(float dt) {

    Json payload = connection::get().q.front();
    connection::get().q.pop_front();

    std::string type = payload["type"].string_value();

    if(type == "join_room_res") {
      join_room_res(payload["result"].bool_value(), payload["rid"].int_value(), payload["is_master"].bool_value());
    } else {
      CCLOG("[error] handler 없음");
    }
}

void lobby_multi_scene::join_room_res(bool result, int rid, bool is_master) {

  if(!result) {
    CCLOG("[debug] fail");
  }

  if(!user_info::get().create_room(rid, is_master)) {
    CCLOG("[error] 방생성 실패");    
    return;
  } 

  auto scene = vs_room_scene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(1, scene, Color3B(0,125,125)));

  CCLOG("[debug] 방생성 완료");
}
