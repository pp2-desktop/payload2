#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "lobby_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"

#include "single_lobby_scene.hpp"
#include "single_play_scene.hpp"

using namespace ui;
using namespace CocosDenshion;

Scene* single_lobby_scene::createScene() {

  auto scene = Scene::create();
  auto layer = single_lobby_scene::create();

  scene->addChild(layer);

  return scene;
}


bool single_lobby_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);
    
  auto closeItem = MenuItemImage::create(
					 "CloseNormal.png",
					 "CloseSelected.png",
					 CC_CALLBACK_1(single_lobby_scene::menuCloseCallback, this));

  closeItem->setScale(2.0f, 2.0f);
  closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2-20, origin.y + closeItem->getContentSize().height/2+15));

  auto menu = Menu::create(closeItem, NULL);
  menu->setPosition(Vec2::ZERO);
  this->addChild(menu, 1);


  
  req_play_info();


  
  this->scheduleUpdate();
    
  return true;
}

void single_lobby_scene::req_play_info() {

  HttpRequest* request = new HttpRequest();
 

  request->setUrl("http://httpbin.org/get");
  request->setRequestType(HttpRequest::Type::GET);
  request->setResponseCallback(this, httpresponse_selector(single_lobby_scene::handle_req_play_info));
  //request->setTag("Get test");
 
  HttpClient::getInstance()->send(request);
  request->release();

}

void single_lobby_scene::handle_req_play_info(HttpClient *sender, HttpResponse *response) {
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();  

  long statusCode = response->getResponseCode(); 
  CCLOG("response code: %ld", statusCode);
 
  if (!response->isSucceed()) {
    CCLOG("response failed");
    CCLOG("error buffer: %s", response->getErrorBuffer());
    return;
  }

  auto scroll_frame_width = 1200;  // L 117, R 117 = 1334
  auto scroll_frame_height = 550;  // T 50, B 50 = 750

  // 스크롤할 전체 뷰의 사이즈
  //Size scollFrameSize = Size(visibleSize.width/1.2, visibleSize.height/2);
  Size scollFrameSize = Size(scroll_frame_width, scroll_frame_height);
  auto scrollView = cocos2d::ui::ScrollView::create();
  scrollView->setContentSize(scollFrameSize);
  scrollView->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
  scrollView->setBackGroundColor(Color3B(200, 200, 200));
  

  // 
  auto cheight = (visibleSize.height - scollFrameSize.height) / 2;
  scrollView->setPosition(Point(67, cheight));
  //scrollView->setPosition(Point(0, cheight));


  scrollView->setDirection(cocos2d::ui::ScrollView::Direction::HORIZONTAL);
  scrollView->setBounceEnabled(true);
  scrollView->setTouchEnabled(true);


  // 스크롤 프레임 사이즈는 변화 없음 containersize 변화 있음!! 
  auto max_item_cnt = 10;
  auto item_full_size_width = 300;

  auto containerSize = Size( (max_item_cnt * (item_full_size_width+50)) + 50, scollFrameSize.height);
  scrollView->setInnerContainerSize(containerSize);
  this->addChild(scrollView);


  auto left_start_offset = item_full_size_width/2 + 50;
  // 안에 들어갈 오브젝트들
  auto last_x = 0;

  for(auto i=0; i<max_item_cnt; i++) {
    
    auto item = Sprite::create("img/boracay.jpg");

    if(i == 0) {
      last_x = 50 + item_full_size_width/2;
      item->setPosition(Point(last_x, scollFrameSize.height /2));
    } else {
      last_x = last_x + item_full_size_width + 50;
      CCLOG("last_x: %d", last_x);
      item->setPosition(Point(last_x, scollFrameSize.height /2));
    }

    scrollView->addChild(item, 0);

    auto item_button = ui::Button::create();
    item_button->setTouchEnabled(true);
    item_button->ignoreContentAdaptWithSize(false);
    item_button->setContentSize(Size(250, 100));
    item_button->loadTextures("ui/pressed_item.png", "ui/pressed_item.png");

    
    if(i == 0) {
      item_button->setPosition(Point(last_x, scollFrameSize.height /2-135));
    } else {
      CCLOG("last_x: %d", last_x);
      item_button->setPosition(Point(last_x, scollFrameSize.height /2-135));
    }

    item_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {

	if(type == ui::Widget::TouchEventType::BEGAN) {

	  auto single_play_scene = single_play_scene::createScene();
	  Director::getInstance()->replaceScene(TransitionFade::create(0.5f, single_play_scene, Color3B(0,255,255)));
	}
     
      });
    scrollView->addChild(item_button);
  }
  
}

void single_lobby_scene::menuCloseCallback(Ref* pSender) {
  CCLOG("xxxxxxxxxxX");
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}

void single_lobby_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }
  
}

void single_lobby_scene::handle_payload(float dt) {
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
