#include "SimpleAudioEngine.h"
#include "lobby_scene.hpp"
#include "lobby_multi_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"
#include "single_lobby_scene.hpp"
#include "multi_lobby_scene.hpp"
#include "assets_scene.hpp"
#include "resource_md.hpp"
//#include "single_play_scene.hpp"
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

  // 파싱 데이터
  /*
  CCUserDefault *def=CCUserDefault::sharedUserDefault();
  def-> setIntegerForKey("high_score", 2000);
  def->flush();
  // int high_score=def->getIntegerForKey("high_score");
  //setStringForKey
  CCLOG("high score: %d", high_score);
  */
  auto audio = SimpleAudioEngine::getInstance();
  audio->playBackgroundMusic("sound/bg2.mp3", true);
  audio->setBackgroundMusicVolume(0.5f);
  
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

  /*
  auto background_all = Sprite::create("background/all.jpg");
  background_all->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background_all, 0);
  */

  // 
  auto background = Sprite::create("background/lobby_scene.png");
  //auto background = Sprite::create(resource_md::get().path + "right_2.jpg");
  background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background, 0);

  // 싱글 버튼 추가
  /*
  auto single_button = Button::create("ui/normal_btn.png", "ui/pressed_btn.png", "ui/disabled_btn.png");
  single_button->setTitleText("SinglePlay");
  single_button->setTitleFontSize(24);
  single_button->setScale(2.0f, 2.0f);
  single_button->setPosition(Vec2(center_.x+425, center_.y+260));
  single_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {

      if(type == ui::Widget::TouchEventType::BEGAN) {

	auto audio = SimpleAudioEngine::getInstance();
	audio->playEffect("sound/pressing.wav", false, 1.0f, 1.0f, 1.0f);

	auto single_lobby_scene = single_lobby_scene::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(0.5f, single_lobby_scene, Color3B(0,255,255)));
      }
     
    });
  this->addChild(single_button);
  */

  // 멀티 버튼 추가
  /*
  auto multi_button = Button::create("ui/normal_btn.png", "ui/pressed_btn.png", "ui/disabled_btn.png");
  multi_button->setTitleText("MultiPlay");
  multi_button->setTitleFontSize(24);
  multi_button->setScale(2.0f, 2.0f);
  multi_button->setPosition(Vec2(center_.x+425, center_.y+150));
  multi_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      auto scene = lobby_multi_scene::createScene();
      auto audio = SimpleAudioEngine::getInstance();

      switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
	  audio->playEffect("sound/pressing.wav", false, 1.0f, 1.0f, 1.0f);
	  break;
	case ui::Widget::TouchEventType::ENDED:

	  if(resource_md::get().get_is_resource_load()) {
	    Director::getInstance()->replaceScene(TransitionFade::create(1, scene, Color3B(0,255,255)));
	  } else {
	    CCLOG("fail to downalod resource");
	    CCLOG("please check network status and try again");
	  }

	  break;
	default:
	  break;
	}
    });
  this->addChild(multi_button);
  */


  

  sp_button = ui::Button::create();
  sp_button->setTouchEnabled(true);
  sp_button->setScale(3.0f);
  sp_button->ignoreContentAdaptWithSize(false);
  sp_button->setContentSize(Size(64, 64));
  sp_button->loadTextures("ui/sp_button.png", "ui/sp_button.png");

  sp_button->setPosition(Vec2(222, center_.y));

  sp_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);

	auto scaleTo = ScaleTo::create(0.1f, 3.3f);
	auto scaleTo2 = ScaleTo::create(0.1f, 3.0f);
	auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	sp_button->runAction(seq2);

        this->scheduleOnce(SEL_SCHEDULE(&lobby_scene::replace_single_lobby_scene), 0.2f); 
      }
    });
     
  this->addChild(sp_button, 0);


  mp_button = ui::Button::create();
  mp_button->setTouchEnabled(true);
  mp_button->setScale(3.0f);
  mp_button->ignoreContentAdaptWithSize(false);
  mp_button->setContentSize(Size(64, 64));
  mp_button->loadTextures("ui/mp_button.png", "ui/mp_button.png");

  mp_button->setPosition(Vec2(222*2, center_.y));

  mp_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 3.3f);
	auto scaleTo2 = ScaleTo::create(0.1f, 3.0f);
	auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	mp_button->runAction(seq2);

        this->scheduleOnce(SEL_SCHEDULE(&lobby_scene::replace_multi_lobby_scene), 0.2f); 
      }
    });
     
  this->addChild(mp_button, 0);


  ranking_button = ui::Button::create();
  ranking_button->setTouchEnabled(true);
  ranking_button->setScale(3.0f);
  ranking_button->ignoreContentAdaptWithSize(false);
  ranking_button->setContentSize(Size(64, 64));
  ranking_button->loadTextures("ui/ranking_button.png", "ui/ranking_button.png");

  ranking_button->setPosition(Vec2(center_.x, center_.y));

  ranking_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 3.3f);
	auto scaleTo2 = ScaleTo::create(0.1f, 3.0f);
	auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	ranking_button->runAction(seq2);
      }
    });
     
  this->addChild(ranking_button, 0);


  setting_button = ui::Button::create();
  setting_button->setTouchEnabled(true);
  setting_button->setScale(3.0f);
  setting_button->ignoreContentAdaptWithSize(false);
  setting_button->setContentSize(Size(64, 64));
  setting_button->loadTextures("ui/setting_button.png", "ui/setting_button.png");

  setting_button->setPosition(Vec2(889, center_.y));

  setting_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 3.3f);
	auto scaleTo2 = ScaleTo::create(0.1f, 3.0f);
	auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	setting_button->runAction(seq2);
      }
    });
     
  this->addChild(setting_button, 0);


  quit_button = ui::Button::create();
  quit_button->setTouchEnabled(true);
  quit_button->setScale(3.0f);
  quit_button->ignoreContentAdaptWithSize(false);
  quit_button->setContentSize(Size(64, 64));
  quit_button->loadTextures("ui/quit_button.png", "ui/quit_button.png");

  quit_button->setPosition(Vec2(1111, center_.y));

  quit_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 3.3f);
	auto scaleTo2 = ScaleTo::create(0.1f, 3.0f);
	auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	quit_button->runAction(seq2);
      }
    });
     
  this->addChild(quit_button, 0);


  /* 닉네임 인풋 받는 부분
  textField = TextField::create("bbbb","Arial", 40);
  textField->setMaxLength(10);
  textField->setMaxLengthEnabled(true);
  textField->setPosition(Vec2(center_.x, center_.y));
  textField->addEventListener([&](Ref* sender,ui::TextField::EventType event) {
      CCLOG("%s", textField->getString().c_str());
    });

  this->addChild(textField, 2);
  */
 

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

void lobby_scene::replace_single_lobby_scene() {
  auto single_lobby_scene = single_lobby_scene::createScene();
  Director::getInstance()->replaceScene(single_lobby_scene);
}

void lobby_scene::replace_multi_lobby_scene() {
  auto multi_lobby_scene = multi_lobby_scene::createScene();
  Director::getInstance()->replaceScene(multi_lobby_scene);
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
  audio->playEffect("sound/pressing.wav", false, 1.0f, 1.0f, 1.0f);
  }
}
