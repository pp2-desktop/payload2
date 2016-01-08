#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "lobby_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"
#include "single_play_info.hpp"
#include "single_lobby_scene.hpp"
#include "single_play_scene.hpp"
#include "single_play_info.hpp"
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


  //is_start_game = false;
  start_game = step0;

  auto bg = Sprite::create("background/vs_play_scene.png");
  bg->setPosition(Vec2(center_.x, center_.y));
  this->addChild(bg, 0);

  
  read_single_play_json();

  create_top_ui();
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

void single_lobby_scene::create_top_ui() {

  auto ui_top_bg = Sprite::create("ui/top_single_lobby2.png");
  auto y = center_.y + _play_screen_y/2 - _offset_y+0;
  ui_top_bg->setPosition(Vec2(center_.x, center_.y + _play_screen_y/2 - _offset_y+0));
  this->addChild(ui_top_bg, 0);


  auto font_x = 280;
  auto font_y = center_.y + _play_screen_y/2 - _offset_y+0;
  font_y = font_y + 1;

  auto font_size = 30;
  int money = user_info::get().money;
  std::string input = num_to_money(money);

  top_money_font = Label::createWithTTF(input.c_str(), "fonts/nanumb.ttf", font_size);
  top_money_font->setPosition(Vec2(font_x, font_y));
  top_money_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(top_money_font, 0);


  back_button = ui::Button::create();
  back_button->setTouchEnabled(true);
  back_button->ignoreContentAdaptWithSize(false);
  back_button->setContentSize(Size(64, 64));
  back_button->loadTextures("ui/back2.png", "ui/back2.png");

  back_button->setPosition(Vec2(40, y));

  back_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {

	if(start_game == step1 || start_game == step2) return;

        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);

	auto scaleTo = ScaleTo::create(0.1f, 1.5f);
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	back_button->runAction(seq2);

        this->scheduleOnce(SEL_SCHEDULE(&single_lobby_scene::replace_lobby_scene), 0.2f); 
      }
    });
     
  this->addChild(back_button, 0);
}

void single_lobby_scene::create_menu() {

  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();  

  auto scroll_frame_width = 1200;  // L 117, R 117 = 1334
  auto scroll_frame_height = 500;  // T 50, B 50 = 750

  // 스크롤할 전체 뷰의 사이즈
  //Size scollFrameSize = Size(visibleSize.width/1.2, visibleSize.height/2);
  Size scollFrameSize = Size(scroll_frame_width, scroll_frame_height);

  scrollView = cocos2d::ui::ScrollView::create();
  scrollView->setContentSize(scollFrameSize);
  scrollView->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
  scrollView->setBackGroundColor(Color3B(200, 200, 200));
  scrollView->setOpacity(100);

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

  scrollView->setSwallowTouches(false);

  auto input_listener = EventListenerTouchOneByOne::create();
  //input_listener->setSwallowTouches(true);
  input_listener->onTouchBegan = [=](Touch* touch, Event* event) {
    CCPoint touchLocation = touch->getLocationInView();
    touchedLocation = cocos2d::CCDirector::sharedDirector()->convertToGL(touchLocation);
    CCLOG("x: %f", touchedLocation.x);
    CCLOG("y: %f", touchedLocation.y);

    if(start_game == step1) {
      start_game = step2;
      float y = center_.y + _play_screen_y/2 - _offset_y+0;
      start_action(Vec2(150, y), Vec2(touchedLocation.x, touchedLocation.y));
      scrollView->setTouchEnabled(false);
    }

    return true;
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(input_listener, this);

  /*
  scrollView->setTouchEnabled(true);
  scrollView->addTouchEventListener(CC_CALLBACK_2(single_lobby_scene::touchEvent2, this));
  */


    

  auto left_start_offset = item_full_size_width/2 + 50;
  // 안에 들어갈 오브젝트들
  auto last_x = 0;

  for(auto i=0; i<max_item_cnt; i++) {

    auto theme = themes[i];
    auto item_img = "img/themes/" + theme + ".jpg";
    auto item = Sprite::create(item_img);

    auto bg_item = Sprite::create("img/themes/bg_theme.png");


    if(i == 0) {
      last_x = 50 + item_full_size_width/2;
      bg_item->setPosition(Point(last_x, scollFrameSize.height /2));
      item->setPosition(Point(last_x, scollFrameSize.height /2));
    } else {
      last_x = last_x + item_full_size_width + 50;
      bg_item->setPosition(Point(last_x, scollFrameSize.height /2));
      item->setPosition(Point(last_x, scollFrameSize.height /2));
    }

    scrollView->addChild(bg_item, 0);
    scrollView->addChild(item, 0);


    // 타이틀
    auto title_img = "img/themes/title_"  + theme + ".png";
    auto title_item = Sprite::create(title_img);

    if(i == 0) {
      title_item->setPosition(Point(last_x, scollFrameSize.height /2 + 200));
    } else {
      title_item->setPosition(Point(last_x, scollFrameSize.height /2 + 200));
    }

    //title_item->setOpacity(98);
    scrollView->addChild(title_item, 0);


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
    item_button->setScale(0.5f);
    //item_button->setScaleY(0.8f);
    item_button->ignoreContentAdaptWithSize(false);
    item_button->setContentSize(Size(427, 167));
    item_button->loadTextures("ui/start_button.png", "ui/start_button.png");

    
    if(i == 0) {
      item_button->setPosition(Point(last_x, scollFrameSize.height /2-135));
    } else {
      item_button->setPosition(Point(last_x, scollFrameSize.height /2-135));
    }

    auto index = i;

    //Vec2 pos(static_cast<float>(last_x), static_cast<float>(scroll_frame_height/2-135));
    item_button->setSwallowTouches(false);
    item_button->addTouchEventListener([&, theme, index](Ref* sender, Widget::TouchEventType type) {

	if(type == ui::Widget::TouchEventType::BEGAN) {

	  if(start_game == step2) {
	    return;
	  }
	  
	  auto audio = SimpleAudioEngine::getInstance();
	  audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);
	  play_info_md::get().playing_theme = theme;	  

	  // single_play_scene 교체
	  auto scaleTo = ScaleTo::create(0.1f, 0.7f);
	  auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	  auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	  
	  //start_buttons[index]->runAction(seq2);


	  auto i = 0;
	  for(auto button : start_buttons) {
	    if(index == i) {
	      Vec2 to = button->getPosition();
	      button->runAction(seq2);
	      if(start_game == step0) {

		int money = user_info::get().money;
		if(playing_game_cost <= money) {
		  user_info::get().money -= playing_game_cost;
		  money = user_info::get().money;
		  std::string input = num_to_money(money);
		  top_money_font->setString(input.c_str());
		  start_game = step1;
		} else {
		  CCLOG("돈이 부족함");
		}
	      }
	      //float y = center_.y + _play_screen_y/2 - _offset_y+0;
	      //start_action(Vec2(150, y), Vec2(touchedLocation.x, touchedLocation.y));
	      break;
	    }
	    i++;
	  }

	  this->scheduleOnce(SEL_SCHEDULE(&single_lobby_scene::replace_single_play_scene), 2.0f);
	}
      });

    start_buttons.pushBack(item_button);
    scrollView->addChild(item_button);


    // 스테이지 현황
    auto percentage_label = Label::createWithTTF(ccsf2("%d %%", progression), "fonts/nanumb.ttf", 30);
    percentage_label->setColor( Color3B( 255, 255, 255) );
    percentage_label->setPosition(Point(last_x, scollFrameSize.height /2-32));
    scrollView->addChild(percentage_label, 0);
   
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

void single_lobby_scene::replace_lobby_scene() {
  auto lobby_scene = lobby_scene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.0f, lobby_scene, Color3B(0,255,255)));
}

void single_lobby_scene::replace_single_play_scene() {
  auto single_play_scene = single_play_scene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.0f, single_play_scene, Color3B(0,255,255)));
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

void single_lobby_scene::start_action(Vec2 from, Vec2 to) {

  auto charged_money = CCSprite::create("ui/coin2.png");
  charged_money->setPosition(from);


  ccBezierConfig bezier;
  bezier.controlPoint_1 = Point(from.x, from.y);  // 첫번째 위치
  bezier.controlPoint_2 = Point((to.x - from.x)/2.0f, (to.y - from.y)/2.0f);  // 첫번째 위치
  bezier.endPosition = Point(to.x, to.y);    // 마지막 위치
  auto action = BezierTo::create(0.4f, bezier);    // 시간, BezierConfig
 
  charged_money->runAction(action);
  this->addChild(charged_money, 1.0f);
  
  at = to;
  this->scheduleOnce(SEL_SCHEDULE(&single_lobby_scene::do_demo), 0.4f);

  auto audio = SimpleAudioEngine::getInstance();
  audio->preloadBackgroundMusic("sound/bg0.mp3");
}

void single_lobby_scene::do_demo() {
  //http://cocos2d-x.tistory.com/entry/5-3
  ParticleSystem* particleSys = ParticleMeteor::create();
  particleSys->retain();
   
  particleSys->setTexture(Director::getInstance()->getTextureCache()->addImage("particle/fire.png"));   

  particleSys->setPosition(at.x, at.y);
  this->addChild(particleSys, 50);
  particleSys->setScale(3);                          // 크기
  particleSys->setDuration(0.8f);
    //particleSys->setLife(1.0f);
}

/*
void single_lobby_scene::touchEvent2(Ref *pSender, Widget::TouchEventType type) {

  switch (type)
    {
    case Widget::TouchEventType::BEGAN:

 CCPoint touchLocation = touch->getLocationInView();
      touchedLocation = cocos2d::CCDirector::sharedDirector()->convertToGL(touchLocation);
      CCLOG("x: %f", touchedLocation.x);
      CCLOG("y: %f", touchedLocation.y);

    case Widget::TouchEventType::MOVED:


    case Widget::TouchEventType::ENDED:
      {
      }
      break;

    case Widget::TouchEventType::CANCELED:

    default:
      break;
    }
}
*/

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
