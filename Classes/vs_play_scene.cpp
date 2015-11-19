#include "vs_play_scene.hpp"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "connection.hpp"
#include "user_info.hpp"
#include "lobby_scene.hpp"
#include <thread>
#include <chrono>
#include <cmath>

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


  // 리소스 로딩(스프라이트 및 에니메이션)
  correct_animation = CCAnimation::create();
  correct_animation->setDelayPerUnit(0.2f);
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct1.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct2.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct3.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct4.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct5.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct6.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct7.png");

  //correct_animate = CCAnimate::create(correct_animation);

  //correct->runAction(CCAnimate::create(correct_animation));
  /*
  correct = Sprite::create();
  correct->setScale(0.2,0.2);
  correct->setPosition(285, 407);
  correct->runAction(correct_animate);
  this->addChild(correct,2);
  */



  // 현재 스테이지
  stage_cnt_ = 0;


  // 터치 이벤트
  auto listener1 = EventListenerTouchOneByOne::create();
  listener1->onTouchBegan = [=](Touch* touch, Event* event) {
    CCLOG("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    CCPoint touchLocation = touch->getLocationInView();
    touchLocation = cocos2d::CCDirector::sharedDirector()->convertToGL(touchLocation);

    touchLocation.y = std::abs(touchLocation.y - 750.0f);
    CCLOG("x : %f, y: %f", touchLocation.x, touchLocation.y);
    auto r = this->check_spot(touchLocation.x, touchLocation.y);
    if(std::get<0>(r)) {
      auto i = std::get<1>(r);
      this->round_infos_[stage_cnt_].find_spots[i] = true;

      connection::get().send2(Json::object {
	  { "type", "find_spot_req" },
	  { "stage", static_cast<int>(stage_cnt_) },
	  { "index", i}
	});
    } else {
      this->touch_incorrect_spot();
    }

    // gl to 0,0
    //touchLocation.x = touchLocation.x - visibleSize.width/2;
    //touchLocation.y = touchLocation.y - visibleSize.height/2;
    //CCLOG("x : %f, y: %f", touchLocation.x, touchLocation.y);

    return true; 
  };
  /*
  listener1->onTouchMoved = [](Touch* touch, Event* event) {
  };

  listener1->onTouchEnded = [=](Touch* touch, Event* event) {
  };
  */
  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener1, this);
  
  // 664, 676
  offset_x = 2.0f;  // => 1334
  offset_y = 37.0f; // => 37*2 + 676 = 750
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
    user_info::get().destroy_room(); 
    //user_info::get().destroy_room(); 
    // after reconnect prev scene

  } else if (type == "round_info_res") {
    round_info_res(payload["round_infos"]);

  } else if (type == "start_round_res" ) {
    start_round_res(payload);
  } else if (type == "end_round_res") {
    end_round_res(payload);
  } else if (type == "update_alive_res") {
   connection::get().send2(Json::object {
	{ "type", "update_alive_req" }
      });
    CCLOG("[debug] update_alive_req 보냄");

  } else if (type == "kick_user_notify") {
    CCLOG("[debug] 킥 당함");
    user_info::get().destroy_room();
    auto scene = lobby_scene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(1, scene, Color3B(255,0,255)));

  } else if (type == "find_spot_res") {
    auto stage_cnt = payload["round_cnt"].int_value();
    auto index = payload["index"].int_value();
    auto winner_type = static_cast<VS_PLAY_WINNER_TYPE >(payload["winner_type"].int_value());
    auto is_end_round = payload["is_end_round"].bool_value();
    auto is_end_vs_play = payload["is_end_vs_play"].bool_value();

    CCLOG("stage_cnt: %d :", stage_cnt);
    CCLOG("index %d : ", index);
    CCLOG("winner_type %d :", winner_type);
    CCLOG("is_end_round %d :", is_end_round);
    CCLOG("is_end_vs_play %d :", is_end_vs_play);

    auto founder = static_cast<VS_PLAY_WINNER_TYPE>(winner_type);
    VS_PLAY_WINNER_TYPE my_winner_type = MASTER;
    if(!user_info::get().room_info_ptr->is_master_) {
      my_winner_type = OPPONENT;
    }

    if(founder == my_winner_type) {
      found_spot(true, stage_cnt, index);
    } else {
      found_spot(false, stage_cnt, index);
    }

    // 1. 라운드 종료와


    // 2. 경기가 다 끝났는지 까지 체크함
    
    
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

    for(unsigned i=0; i<size; i++) {
      r_info.find_spots.push_back(false);
    }
    
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

std::tuple<bool, int> vs_play_scene::check_spot(float x, float y) {
  //(visibleSize.width/2) + x + offset_x * 2.0f;
  Size visibleSize = Director::getInstance()->getVisibleSize();
  // x가 2개가 되야함
  y = y - offset_y*2;
  Vec2 other_point(0.0f, y);

  if(visibleSize.width/2 + offset_x * 2.0f <= x) {
    other_point.x = x - (visibleSize.width/2 + offset_x * 2.0f);
  } else {
    other_point.x = x;
  }

  CCLOG("other_point x : %f, other_point y: %f", other_point.x, other_point.y);

  for(unsigned i=0; i<round_infos_[stage_cnt_].spots.size(); i++) {
    if(!round_infos_[stage_cnt_].find_spots[i]) {
      bool r = is_point_in_circle(other_point.x, other_point.y, round_infos_[stage_cnt_].spots[i].x, round_infos_[stage_cnt_].spots[i].y, 25.0f);
      if(r) {
	CCLOG("충돌");
	return std::make_tuple<bool, int>(true, i);
      } else {
	CCLOG("충돌 안함");
      }
    }
  }

  return std::make_tuple<bool, int>(false, -1);
}

bool vs_play_scene::is_point_in_circle(float xa, float ya, float xc, float yc, float r) {
   return ((xa-xc)*(xa-xc) + (ya-yc)*(ya-yc)) < r*r;
}

void vs_play_scene::found_spot(bool is_myself, int stage_cnt, int index) {
  if(is_myself) {
    Vec2 img_pos = round_infos_[stage_cnt_].spots[index];
    Vec2 play_pos = change_coordinate_from_img_to_play(img_pos.x, img_pos.y);
    //CCLOG("x: %f", play_pos.x); CCLOG("y: %f", play_pos.y);
    add_correct_action(play_pos.x, play_pos.y);
  } else {
    
  }
}

Vec2 vs_play_scene::change_coordinate_from_img_to_play(float x, float y) {
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 r(x, 0);
  r.y = std::abs(y - (750.0f - offset_y*2));
  /*
  if(x > visibleSize.width/2) {
    r.x = x + offset_x * 2.0f;
  }
  */
  return r;
}

void vs_play_scene::touch_incorrect_spot() {
    
}
//http://www.cocos2d-x.org/wiki/Vector%3CT%3E
void vs_play_scene::add_correct_action(float x, float y) {



   correct_animation = CCAnimation::create();
  correct_animation->setDelayPerUnit(0.1f);
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct1.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct2.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct3.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct4.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct5.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct6.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct7.png");



  auto sp0 = Sprite::create();
  vec0->pushBack(sp0);
  
}

void vs_play_scene::destory_correct_spots() {


}

void vs_play_scene::remove_all_correct_actions() {

}

void vs_play_scene::handle_sound(sound_type type) {

  auto audio = SimpleAudioEngine::getInstance();
  if(type == sound_type::BUTTON_PRESSED) {
    audio->playEffect("sound/button_pressed.mp3", false, 1.0f, 1.0f, 1.0f);
  }
}


//void vs_play_scene::check_find_spot(
