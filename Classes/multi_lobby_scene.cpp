#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "multi_lobby_scene.hpp"
#include "lobby_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"
#include "json11.hpp"
#include "single_play_info.hpp"

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
  //audio->playBackgroundMusic("sound/bg1.mp3", true);
  //audio->setBackgroundMusicVolume(0.5f);
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);

  auto background = Sprite::create("background/lobby_scene.png");
  //auto background = Sprite::create(resource_md::get().path + "right_2.jpg");
  background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background, 0);

  create_ui_buttons();
  create_ui_room_info();
  create_ui_chat_info();
  
  this->scheduleUpdate();
    
  return true;
}

void multi_lobby_scene::create_ui_buttons() {

  auto y = center_.y + _play_screen_y/2 - _offset_y+0;

  back_button = ui::Button::create();
  back_button->setTouchEnabled(true);
  back_button->ignoreContentAdaptWithSize(false);
  back_button->setContentSize(Size(64, 64));
  back_button->loadTextures("ui/back2.png", "ui/back2.png");

  back_button->setPosition(Vec2(40, y));

  back_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);
	auto scaleTo = ScaleTo::create(0.1f, 1.5f);
	back_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);;
	back_button->runAction(scaleTo2);
        this->scheduleOnce(SEL_SCHEDULE(&multi_lobby_scene::replace_lobby_scene), 0.2f); 

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);;
	back_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(back_button, 0);


  quick_join_button = ui::Button::create();
  quick_join_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  quick_join_button->loadTextures("ui/quick_join_button.png", "ui/quick_join_button.png");
  quick_join_button->ignoreContentAdaptWithSize(false);
  quick_join_button->setContentSize(Size(221, 120));
  quick_join_button->setPosition(Vec2(1190, center_.y-300));
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

void multi_lobby_scene::create_ui_room_info() {
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();  

  auto scroll_frame_width = 740; 
  auto scroll_frame_height = 650; 

  Size scollFrameSize = Size(scroll_frame_width, scroll_frame_height);

  scrollView = cocos2d::ui::ScrollView::create();
  scrollView->setContentSize(scollFrameSize);
  scrollView->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
  scrollView->setBackGroundColor(Color3B(200, 200, 200));
  scrollView->setOpacity(80);

  // 
  auto cheight = ((visibleSize.height - scollFrameSize.height) / 2) - 30.0f;
  scrollView->setPosition(Point(25, cheight));
  scrollView->setDirection(cocos2d::ui::ScrollView::Direction::VERTICAL);
  scrollView->setBounceEnabled(true);
  scrollView->setTouchEnabled(true);

  auto containerSize = Size(scollFrameSize.width*2, scollFrameSize.height*2);
  scrollView->setInnerContainerSize(containerSize);


  auto tmp = Sprite::create("ui/prestart2_button.png");
  tmp->setPosition(Vec2(0, 0));
  scrollView->addChild(tmp, 1);
 
  this->addChild(scrollView);
}

void multi_lobby_scene::create_ui_chat_info() {

  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();  

  auto scroll_frame_width = 520;
  auto scroll_frame_height = 520; 

  Size scollFrameSize = Size(scroll_frame_width, scroll_frame_height);

  ChatScrollView = cocos2d::ui::ScrollView::create();
  ChatScrollView->setContentSize(scollFrameSize);
  ChatScrollView->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
  ChatScrollView->setBackGroundColor(Color3B(200, 200, 200));
  ChatScrollView->setOpacity(80);

  ChatScrollView->setPosition(Point(center_.x + 122, center_.y - 225));
  ChatScrollView->setDirection(cocos2d::ui::ScrollView::Direction::VERTICAL);
  ChatScrollView->setBounceEnabled(true);
  ChatScrollView->setTouchEnabled(true);

  auto containerSize = Size(scollFrameSize.width*2, scollFrameSize.height*2);
  ChatScrollView->setInnerContainerSize(containerSize);
 
  this->addChild(ChatScrollView);
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

void multi_lobby_scene::replace_lobby_scene() {
  auto lobby_scene = lobby_scene::createScene();
  Director::getInstance()->replaceScene(lobby_scene);
}
