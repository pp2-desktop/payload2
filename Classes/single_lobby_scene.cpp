#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "lobby_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"
#include "single_play_info.hpp"
#include "single_lobby_scene.hpp"
#include "single_play_scene.hpp"
#include "json11.hpp"

using namespace ui;
using namespace CocosDenshion;
using namespace json11;

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


  
  read_single_play_json();

  create_menu();
  //req_play_info();


  
  this->scheduleUpdate();
    
  return true;
}

void single_lobby_scene::read_single_play_json() {
  std::string fileName = CCFileUtils::sharedFileUtils()->fullPathForFilename("config/single_play.json");        
  //auto bufferSize = 0; // android
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
  auto bufferSize = 0; 
#else 
  long bufferSize = 0; 
#endif

  //long bufferSize = 0; // linux
  unsigned char* json = CCFileUtils::sharedFileUtils()->getFileData(fileName.c_str(), "rb", &bufferSize );
  std::string read_data((const char*)json, bufferSize);
  parsing_json(read_data);
}

void single_lobby_scene::parsing_json(std::string read_data) {
  string err;
  auto json = Json::parse(read_data, err);

  if (!err.empty()) {
    CCLOG("[error] fail to parse singl_play.json");
  } else {
    for(auto &theme : json["themes"].array_items()) {

      user_played_info _user_played_info;

      std::string item_key = theme.string_value();

      themes.push_back(item_key);

      auto max_stage_cnt = 0;

      for(auto& item : json[item_key].array_items()) {

        max_stage_cnt++;

	stage_info si;

	CCLOG("img: %s", item["img"].string_value().c_str());
	CCLOG("time: %d", item["time"].int_value());

	si.img = item["img"].string_value();
	si.time = item["time"].int_value();

	for(auto& spot : item["spots"].array_items()) {
	  CCLOG("x: %d", spot["x"].int_value());
	  CCLOG("y: %d", spot["y"].int_value());
	  float x = static_cast<float>(spot["x"].int_value());
	  float y = static_cast<float>(spot["y"].int_value());

	  si.spots.push_back(Vec2(x, y));
	}

	_user_played_info.stage_infos.push_back(si);
      }

      _user_played_info.max_stage_cnt = max_stage_cnt;

      // data 가져오기
      CCUserDefault *def=CCUserDefault::sharedUserDefault();
      _user_played_info.clear_stage = def->getIntegerForKey(item_key.c_str());

      CCLOG("max_stage_cnt: %d",  _user_played_info.max_stage_cnt);

      play_info_md::get().user_played_infos[item_key] = _user_played_info;

      //

    }
  }

}

void single_lobby_scene::create_menu() {

  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();  

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
  auto max_item_cnt = themes.size();
  //  width: 300, height: 400
  auto item_full_size_width = 300;

  auto containerSize = Size( (max_item_cnt * (item_full_size_width+50)) + 50, scollFrameSize.height);
  scrollView->setInnerContainerSize(containerSize);
  this->addChild(scrollView);

  auto left_start_offset = item_full_size_width/2 + 50;
  // 안에 들어갈 오브젝트들
  auto last_x = 0;

  for(auto i=0; i<max_item_cnt; i++) {

    auto theme = themes[i];
    auto item_img = "img/themes/" + theme + ".jpg";
    auto item = Sprite::create(item_img);


    if(i == 0) {
      last_x = 50 + item_full_size_width/2;
      item->setPosition(Point(last_x, scollFrameSize.height /2));
    } else {
      last_x = last_x + item_full_size_width + 50;
      item->setPosition(Point(last_x, scollFrameSize.height /2));
    }

    scrollView->addChild(item, 0);


    // 타이틀
    auto title_img = "img/themes/title_"  + theme + ".png";
    auto title_item = Sprite::create(title_img);

    if(i == 0) {
      title_item->setPosition(Point(last_x, scollFrameSize.height /2 + 180));
    } else {
      title_item->setPosition(Point(last_x, scollFrameSize.height /2 + 180));
    }

    scrollView->addChild(title_item, 0);


    // progressin

    //auto progress_label = Label::createWithTTF("Progress", "fonts/nanumb.ttf", 35);
    /*
    auto progress_label = Label::createWithTTF("진행 상태", "fonts/nanumb.ttf", 35);
    progress_label->setPosition(Point(last_x, scollFrameSize.height /2 + 45));
    progress_label->setColor( Color3B( 255, 125, 0) );
    progress_label->enableShadow();
    //progress_label->enableOutline(Color4B::WHITE, 2);
    scrollView->addChild(progress_label, 0);
    */


    // 진행상황
    auto timeBar = CCSprite::create("ui/timebar2.png");
 
    auto timeOutline = CCSprite::create("ui/timeoutline2.png");
    auto progressTimeBar = CCProgressTimer::create(timeBar);

    if(i == 0) {
      timeOutline->setPosition(Point(last_x, scollFrameSize.height /2));
      progressTimeBar->setPosition(Point(last_x, scollFrameSize.height /2));
    } else {
      timeOutline->setPosition(Point(last_x, scollFrameSize.height /2));
      progressTimeBar->setPosition(Point(last_x, scollFrameSize.height /2));
    }

    timeOutline->setScale(0.4f);
    timeOutline->setVisible(true);
    scrollView->addChild(timeOutline, 0);

    progressTimeBar->setScale(0.4f);
    //progressTimeBar_->setScaleX(0.6f);
    //progressTimeBar_->setScaleY(0.9f);
    //progressTimeBar_->setMidpoint(ccp(0, 0.5f));
    progressTimeBar->setMidpoint(ccp(0, 1.0f));
    progressTimeBar->setBarChangeRate(ccp(1, 0));
    progressTimeBar->setType(kCCProgressTimerTypeBar);
    auto progression = 0;
    if(!play_info_md::get().user_played_infos[theme].clear_stage == 0) {
      auto r = static_cast<float>(play_info_md::get().user_played_infos[theme].clear_stage) / static_cast<float>(play_info_md::get().user_played_infos[theme].max_stage_cnt);
      CCLOG("%f", r);
      progression = static_cast<int>(r * 100.0f);
      CCLOG("%d", progression);
    }
    progressTimeBar->setPercentage(progression);
    scrollView->addChild(progressTimeBar, 0);
    


    // 시작 버튼
    auto item_button = ui::Button::create();
    item_button->setTouchEnabled(true);
    item_button->setScaleY(0.8f);
    item_button->ignoreContentAdaptWithSize(false);
    item_button->setContentSize(Size(250, 100));
    item_button->loadTextures("ui/pressed_item.png", "ui/pressed_item.png");

    
    if(i == 0) {
      item_button->setPosition(Point(last_x, scollFrameSize.height /2-135));
    } else {
      item_button->setPosition(Point(last_x, scollFrameSize.height /2-135));
    }
    
    item_button->addTouchEventListener([&, theme](Ref* sender, Widget::TouchEventType type) {

	if(type == ui::Widget::TouchEventType::BEGAN) {

	  play_info_md::get().playing_theme = theme;
	  // single_play_scene 교체
	  auto single_play_scene = single_play_scene::createScene();
	  Director::getInstance()->replaceScene(TransitionFade::create(0.5f, single_play_scene, Color3B(0,255,255)));
	}
     
      });
    scrollView->addChild(item_button);


    // 스테이지 현황
    auto clear_stage =  play_info_md::get().user_played_infos[theme].clear_stage;
    auto clear_stage_label = Label::createWithTTF(ccsf2("%d", clear_stage), "fonts/nanumb.ttf", 35);
    clear_stage_label->setPosition(Point(last_x-20, scollFrameSize.height /2-32));
    clear_stage_label->setColor( Color3B( 255, 255, 255) );
    scrollView->addChild(clear_stage_label, 1);

    auto slash_label = Label::createWithTTF("/", "fonts/nanumb.ttf", 30);
    slash_label->setPosition(Point(last_x, scollFrameSize.height /2-32));
    slash_label->setColor( Color3B( 255, 255, 255) );
    scrollView->addChild(slash_label, 1);

    auto max_stage_cnt =  play_info_md::get().user_played_infos[theme].max_stage_cnt;
    auto max_stage_cnt_label = Label::createWithTTF(ccsf2("%d", max_stage_cnt), "fonts/nanumb.ttf", 35);
    max_stage_cnt_label->setPosition(Point(last_x+20, scollFrameSize.height /2-32));
    max_stage_cnt_label->setColor( Color3B( 255, 255, 255) );
    scrollView->addChild(max_stage_cnt_label, 1);

    CCLOG("max_stage_cnt: %d", play_info_md::get().user_played_infos[theme].max_stage_cnt);
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


/*
    for(auto &theme : json["themes"].array_items()) {
      CCLOG("%s", theme.string_value().c_str());
      std::string item_key = theme.string_value();

      for(auto& item : json[item_key].array_items()) {
	CCLOG("img: %s", item["img"].string_value().c_str());
	CCLOG("time: %d", item["time"].int_value());

	for(auto& spot : item["spots"].array_items()) {
	  CCLOG("x: %d", spot["x"].int_value());
	  CCLOG("y: %d", spot["y"].int_value());
	}
      }
    }
*/
