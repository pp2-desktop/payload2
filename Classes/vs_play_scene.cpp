#include "vs_play_scene.hpp"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "connection.hpp"
#include "user_info.hpp"

using namespace ui;
using namespace CocosDenshion;

Scene* vs_play_scene::createScene()
{

  auto scene = Scene::create();
   
  auto layer = vs_play_scene::create();

  scene->addChild(layer);

  return scene;
}


bool vs_play_scene::init()
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

  // 백그라운드
  auto background = Sprite::create("background/vs_play_scene.png");
  background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background, 0);

  // 현재 스테이지
  stage_cnt_ = 0;


  // 터치 이벤트
  auto listener1 = EventListenerTouchOneByOne::create();
  listener1->onTouchBegan = [=](Touch* touch, Event* event) {
    CCLOG("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    CCPoint touchLocation = touch->getLocationInView();
    touchLocation = cocos2d::CCDirector::sharedDirector()->convertToGL(touchLocation);

    // gl to 0,0
    touchLocation.x = touchLocation.x - visibleSize.width/2;
    touchLocation.y = touchLocation.y - visibleSize.height/2;
    CCLOG("x : %f, y: %f", touchLocation.x, touchLocation.y);

    return true; 
  };
  /*
  listener1->onTouchMoved = [](Touch* touch, Event* event) {
  };

  listener1->onTouchEnded = [=](Touch* touch, Event* event) {
  };
  */
  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener1, this);
  
  offset_x = 2.0f;
  offset_y = 50.0f;
  // xx

  
  connection::get().send2(Json::object {
      { "type", "round_info_req" }
    });

  this->scheduleUpdate();

  return true;
}

void vs_play_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  } 


  //CCLOG("update");
}

void vs_play_scene::handle_payload(float dt) {

  Json payload = connection::get().q.front();
  connection::get().q.pop_front();

  std::string type = payload["type"].string_value();
  CCLOG("type: %s", type.c_str());
  

  if (type == "connection_notify") {

  } else if (type == "disconnection_notify") {
    CCLOG("[debug] 접속 큰킴");
    //user_info::get().destroy_room(); 
    // after reconnect prev scene

  } else if (type == "round_info_res") {
    round_info_res(payload["round_infos"]);

  } else if (type == "start_round_res" ) {
    start_round_res(payload);
  } else if (type == "end_round_res") {
    end_round_res(payload);
  } else if (type == "find_spot_res" ) {

    // check who's?

    // end of game? next round?

  } else {
    CCLOG("[error] vs_play_scene handler 없음");
  }

}

void vs_play_scene::round_info_res(Json round_infos) {
  round_infos_.clear();
  for (auto &r : round_infos.array_items()) {

    round_info r_info;
    auto img0 = r["img0"].string_value();
    auto img1 = r["img1"].string_value();

    auto points = r["point_infos"];
    auto size = points.array_items().size();
    std::vector<Vec2> spots;

    CCLOG("size: %d",size);

    for(unsigned i=0; i<size;) {
      auto x = points.array_items()[i];
      auto y = points.array_items()[i+1];
      Vec2 v(x.int_value(), y.int_value());
      spots.push_back(v);
      i = i+2;
    }

    r_info.left_img = img0;
    r_info.right_img = img1;
    r_info.spots = spots;
    
    CCLOG("spot count: %d", r_info.spots.size());
    round_infos_.push_back(r_info);
  }

  max_stage_cnt_ = round_infos_.size();
  
  pre_loading_resources();
  
  connection::get().send2(Json::object {
      { "type", "start_round_req" }
    });

  CCLOG("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
}

 void vs_play_scene::pre_loading_resources() {
   //텍스쳐, 오디오 기타 리소스 프리 로딩
   Size visibleSize = Director::getInstance()->getVisibleSize();
   Vec2 origin = Director::getInstance()->getVisibleOrigin();

   auto left_img = Sprite::create("img/" + round_infos_[stage_cnt_].left_img);
   left_img->setPosition(Vec2((visibleSize.width/2)/2 + origin.x - offset_x, visibleSize.height/2 + origin.y - offset_y));
   this->addChild(left_img, 1);

   auto right_img = Sprite::create("img/" + round_infos_[stage_cnt_].right_img);
   right_img->setPosition(Vec2( (visibleSize.width/2)+(visibleSize.width/2/2) + origin.x + offset_x, visibleSize.height/2 + origin.y  - offset_y));
   this->addChild(right_img, 1);
}

// 다음 라운드 시작하라는 의미 두명한테서 다 받을때까지 기다림
void vs_play_scene::start_round_res(Json payload) {
  stage_cnt_ = payload["stage_cnt"].int_value();
  // open 커튼
}

void vs_play_scene::end_round_res(Json payload) {
  // 다음 라운드 있는지 is_next_round = false
  auto stage_cnt = payload["stage_cnt"].int_value();
  auto winner = payload["winner_type"].int_value();
  auto is_next = payload["is_next"].bool_value();
  
  round_infos_[stage_cnt].winner = static_cast<VS_PLAY_WINNER_TYPE>(winner); 
  // close 커튼
}

// 한 라운드 끝날때 마다 연출(커튼을 친다든지 기타)
void vs_play_scene::score_vs_round(Json payload) {

}

// 경기 끝나도 마지막 연출 결과보여주기
void vs_play_scene::score_vs_play(Json payload) {

}

void vs_play_scene::end_vs_play_res(Json payload) {

}

void vs_play_scene::handle_sound(sound_type type) {

  auto audio = SimpleAudioEngine::getInstance();
  if(type == sound_type::BUTTON_PRESSED) {
    audio->playEffect("sound/button_pressed.mp3", false, 1.0f, 1.0f, 1.0f);
  }
}


//void vs_play_scene::check_find_spot(
