#include "vs_play_scene.hpp"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "connection.hpp"
#include "user_info.hpp"
#include "lobby_scene.hpp"
#include "vs_room_scene.hpp"
#include <thread>
#include <chrono>
#include <cmath>

using namespace ui;
using namespace CocosDenshion;

#define ccsf(...) CCString::createWithFormat(__VA_ARGS__)->getCString()

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


  // 리소스 로딩
  //vec0 = std::make_shared<Vector<Sprite*>>();
  // test버튼 추가 
  /*
  auto test_button = Button::create("ui/normal_btn.png", "ui/pressed_btn.png", "ui/disabled_btn.png");
  test_button->setScale(1.2f, 1.2f);
  test_button->setPosition(Vec2(center_.x+430, center_.y-280));
  test_button->addTouchEventListener([this](Ref* sender, Widget::TouchEventType type) {

      switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
	  break;

	case ui::Widget::TouchEventType::ENDED:
	  //this->destory_round();


	  break;

	default:
	  break;
	}
    });
  this->addChild(test_button, 4);
*/

  offset_x = 2.0f;  // => 1334
  offset_y = 37.0f; // => 37*2 + 676 = 750

  // ui 이미지 로딩
  auto top = Sprite::create("ui/top.png");
  top->setPosition(Vec2(visibleSize.width/2, visibleSize.height-offset_y));
  this->addChild(top, 1);


  auto ui_offset_x = 40;
  auto top_stage_font = Label::createWithTTF("스테이지", "fonts/nanumb.ttf", offset_y-10);
  top_stage_font->setPosition(Vec2(visibleSize.width/2 + ui_offset_x, visibleSize.height-offset_y));
  //top_stage_font->enableShadow();
  top_stage_font->setColor( Color3B( 125, 125, 125) );
  this->addChild(top_stage_font, 1);

  top_left_stage_font = Label::createWithTTF("1", "fonts/nanumb.ttf", offset_y-10);
  top_left_stage_font->setPosition(Vec2(visibleSize.width/2 + ui_offset_x + 75, visibleSize.height-offset_y));
  top_left_stage_font->setColor( Color3B( 125, 125, 125) );
  this->addChild(top_left_stage_font, 1);

  auto top_stage_slash_font = Label::createWithTTF("/", "fonts/nanumb.ttf", offset_y-10);
  top_stage_slash_font->setPosition(Vec2(visibleSize.width/2 + ui_offset_x + 100, visibleSize.height-offset_y));
  top_stage_slash_font->setColor( Color3B( 125, 125, 125) );
  this->addChild(top_stage_slash_font, 1);

  top_right_stage_font = Label::createWithTTF("1", "fonts/nanumb.ttf", offset_y-10);
  top_right_stage_font->setPosition(Vec2(visibleSize.width/2 + ui_offset_x + 125, visibleSize.height-offset_y));
  top_right_stage_font->setColor( Color3B( 125, 125, 125) );
  this->addChild(top_right_stage_font, 1);


  auto top_spot_font = Label::createWithTTF("틀린그림", "fonts/nanumb.ttf", offset_y-10);
  top_spot_font->setPosition(Vec2(visibleSize.width/2 + ui_offset_x + 250, visibleSize.height-offset_y));
  top_spot_font->setColor( Color3B( 125, 125, 125) );
  this->addChild(top_spot_font, 1);

  top_left_spot_font = Label::createWithTTF("0", "fonts/nanumb.ttf", offset_y-10);
  top_left_spot_font->setPosition(Vec2(visibleSize.width/2 + ui_offset_x + 250 + 75, visibleSize.height-offset_y));
  top_left_spot_font->setColor( Color3B( 125, 125, 125) );
  this->addChild(top_left_spot_font, 1);

  auto top_spot_slash_font = Label::createWithTTF("/", "fonts/nanumb.ttf", offset_y-10);
  top_spot_slash_font->setPosition(Vec2(visibleSize.width/2 + ui_offset_x + 350, visibleSize.height-offset_y));
  top_spot_slash_font->setColor( Color3B( 125, 125, 125) );
  this->addChild(top_spot_slash_font, 1);

  top_right_spot_font = Label::createWithTTF("0", "fonts/nanumb.ttf", offset_y-10);
  top_right_spot_font->setPosition(Vec2(visibleSize.width/2 + ui_offset_x + 350 + 25, visibleSize.height-offset_y));
  top_right_spot_font->setColor( Color3B( 125, 125, 125) );
  this->addChild(top_right_spot_font, 1);


  //  332, 1334 / 2  = 667
  left_curtain = Sprite::create("img/left_curtain.jpg");
  left_curtain->setPosition(Vec2(-visibleSize.width/4, visibleSize.height/2 + origin.y));
  this->addChild(left_curtain, 4);

  right_curtain = Sprite::create("img/right_curtain.jpg");
  right_curtain->setPosition(Vec2(visibleSize.width + visibleSize.width/4, visibleSize.height/2 + origin.y));
  this->addChild(right_curtain, 4);

  auto moveby_left_curtain = MoveBy::create(0.1f, Vec2(visibleSize.width/2, 0));
  left_curtain->runAction(moveby_left_curtain);
  auto moveby_right_curtain = MoveBy::create(0.1f, Vec2(-visibleSize.width/2, 0));
  right_curtain->runAction(moveby_right_curtain);

  // 현재 스테이지
  stage_cnt_ = 0;
  unable_touch = false;

  // 터치 이벤트
  auto listener1 = EventListenerTouchOneByOne::create();
  listener1->onTouchBegan = [=](Touch* touch, Event* event) {
    CCLOG("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    CCPoint touchLocation = touch->getLocationInView();
    touchLocation = cocos2d::CCDirector::sharedDirector()->convertToGL(touchLocation);

    if(touchLocation.y > 676) {
      CCLOG("ui영역 input event 발생");
      return true;
    } else if(unable_touch) {
      CCLOG("터치할수 없는 기간");
      return true;
    }

    touchLocation.y = std::abs(touchLocation.y - 750.0f);
    CCLOG("x : %f, y: %f", touchLocation.x, touchLocation.y);
    auto r = this->check_spot(touchLocation.x, touchLocation.y);
    if(std::get<0>(r)) {
      auto i = std::get<1>(r);
      this->round_infos_[stage_cnt_].find_spots[i] = true;

      connection::get().send2(Json::object {
	  { "type", "find_spot_req" },
	  { "round_cnt", static_cast<int>(stage_cnt_) },
	  { "index", i}
	});
    } else {
      if(touchLocation.x < visibleSize.width / 2 ) {
	this->touch_incorrect_spot(true);
      } else {
	this->touch_incorrect_spot(false);
      }
    }

   
    return true; 
  };
  /*
  listener1->onTouchMoved = [](Touch* touch, Event* event) {
  };

  listener1->onTouchEnded = [=](Touch* touch, Event* event) {
  };
  */
  _eventDispatcher->addEventListenerWithSceneGraphPriority(listener1, this);
  

  
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

    CCLOG("stage_cnt: %d", stage_cnt);
    CCLOG("index: %d", index);
    CCLOG("winner_type: %d", winner_type);
    CCLOG("is_end_round: %d", is_end_round);
    CCLOG("is_end_vs_play: %d", is_end_vs_play);

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

    // 마지막 라운드 까지  0, 1, 2, 3, 4 ?? 마지막인데 왜? is_gameplay end 0dla?


    if(is_end_vs_play) {
      // 게임 종료
      auto vs_play_winner_type = static_cast<VS_PLAY_WINNER_TYPE>(payload["vs_play_winner_type"].int_value());
      if(vs_play_winner_type == MASTER) {
	CCLOG("vs play winner is master");
      } else {
	CCLOG("vs play winner is opponent");
      }
      CCLOG("vs_play game end");
      this->scheduleOnce(SEL_SCHEDULE(&vs_play_scene::close_curtain), 1.0f);
      this->scheduleOnce(SEL_SCHEDULE(&vs_play_scene::destroy_vs_play), 2.0f);
    } else if(is_end_round) {
      // 라운드 종료
      auto round_winner_type = static_cast<VS_PLAY_WINNER_TYPE>(payload["round_winner_type"].int_value());
      if(round_winner_type == MASTER) {
	CCLOG("round winner is master");
      } else {
	CCLOG("round winner is opponent");
      }
      CCLOG("round end");
      this->scheduleOnce(SEL_SCHEDULE(&vs_play_scene::close_curtain), 1.0f);
      this->scheduleOnce(SEL_SCHEDULE(&vs_play_scene::destory_round), 2.0f);
    }
    
    
  } else if (type == "other_leave_notify") {
    CCLOG("[debug] 게임중 유저 1명 나감");


  } else if (type == "play_to_lobby_req") { 
    CCLOG("[debug] 게임중 유저 1명 나가서 액션 후 알려줌");


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
    r_info.find_spot_cnt = 0;

    for(unsigned i=0; i<size; i++) {
      r_info.find_spots.push_back(false);
    }
    
    CCLOG("spot count: %d", r_info.spots.size());
    round_infos_.push_back(r_info);
  }

  max_stage_cnt_ = round_infos_.size();
  top_right_stage_font->setString(ccsf(to_string<int>(max_stage_cnt_).c_str()));

  if(max_stage_cnt_ > 1) {
    auto max_spot_cnt = round_infos_[0].spots.size();
    top_right_spot_font->setString(ccsf(to_string<int>(max_spot_cnt).c_str()));    
  }
  
  pre_loading_resources();
  
  connection::get().send2(Json::object {
      { "type", "start_round_req" }, 
      { "round_cnt", 0 }
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
  stage_cnt_ = payload["round_cnt"].int_value();
  CCLOG("stage cnt: %d", stage_cnt_);
  // 시작 사운드
  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/go.wav", false, 1.0f, 1.0f, 1.0f);
  // open 커튼
  open_curtain();
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

  //CCLOG("other_point x : %f, other_point y: %f", other_point.x, other_point.y);
  
  for(unsigned i=0; i<round_infos_[stage_cnt_].spots.size(); i++) {
    if(!round_infos_[stage_cnt_].find_spots[i]) {
      bool r = is_point_in_circle(other_point.x, other_point.y, round_infos_[stage_cnt_].spots[i].x, round_infos_[stage_cnt_].spots[i].y, 25.0f+10.0f);
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

  Vec2 img_pos = round_infos_[stage_cnt_].spots[index];
  Vec2 play_pos = change_coordinate_from_img_to_play(img_pos.x, img_pos.y);

  if(is_myself) {
    add_correct_action(play_pos.x, play_pos.y);
  } else {
    add_other_correct_action(play_pos.x, play_pos.y);    
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

void vs_play_scene::touch_incorrect_spot(bool is_left) {

  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  unable_touch = true;
  
  if(is_left) {
    ActionInterval* lens = Lens3D::create(1, Size(32,24), Vec2(visibleSize.width, 0), 150);
    ActionInterval* waves = Waves3D::create(1, Size(15,10), 18, 15);
    nodeGrid = NodeGrid::create();
   auto left_img = Sprite::create("img/" + round_infos_[stage_cnt_].left_img);
   left_img->setPosition(Vec2((visibleSize.width/2)/2 + origin.x - offset_x, visibleSize.height/2 + origin.y - offset_y));
    nodeGrid->addChild(left_img);
    nodeGrid->runAction(Sequence::create(waves, lens, NULL));

  } else {
    ActionInterval* lens = Lens3D::create(1, Size(32,24), Vec2(0, 0), 150);
    ActionInterval* waves = Waves3D::create(1, Size(15,10), 18, 15);
    nodeGrid = NodeGrid::create();
   auto right_img = Sprite::create("img/" + round_infos_[stage_cnt_].right_img);
   right_img->setPosition(Vec2( (visibleSize.width/2)+(visibleSize.width/2/2) + origin.x + offset_x, visibleSize.height/2 + origin.y  - offset_y));
   nodeGrid->addChild(right_img,2);
   nodeGrid->runAction(Sequence::create(waves, lens, NULL));
  }
  
  nodeGrid->retain();
  this->addChild(nodeGrid, 2);

  this->scheduleOnce(SEL_SCHEDULE(&vs_play_scene::able_touch), 1.0f);
  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/incorrect.mp3", false, 1.0f, 1.0f, 1.0f);    
}

void vs_play_scene::able_touch() {
  unable_touch = false;
  nodeGrid->removeFromParent();
  nodeGrid->release();
}

//http://www.cocos2d-x.org/wiki/Vector%3CT%3E
void vs_play_scene::add_correct_action(float x, float y) {

  // 소리 효과
  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/correct.mp3", false, 1.0f, 1.0f, 1.0f);

  round_infos_[stage_cnt_].find_spot_cnt++;
  top_left_spot_font->setString(ccsf(to_string<int>(round_infos_[stage_cnt_].find_spot_cnt).c_str()));

  Size visibleSize = Director::getInstance()->getVisibleSize();
  float lx = x;
  float rx = x + (offset_x * 2.0f) + (visibleSize.width/2);

  
  auto correct_animation = Animation::create();
  correct_animation->setDelayPerUnit(0.1f);
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct1.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct2.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct3.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct4.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct5.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct6.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct7.png");
  
  auto sp0 = Sprite::create();
  sp0->setScale(0.2,0.2);
  sp0->setPosition(lx, y);
  //sp0->setColor(Color3B(255, 0, 0));
  sp0->runAction(Animate::create(correct_animation));
  this->addChild(sp0, 3);

  auto sp1 = Sprite::create();
  sp1->setScale(0.2,0.2);
  sp1->setPosition(rx, y);
  sp1->runAction(Animate::create(correct_animation));
  this->addChild(sp1, 3);

  // 나중에 제거위해서
  vec0.pushBack(sp0);
  vec0.pushBack(sp1);

  //http://www.programering.com/a/MDM1gTMwATc.html
  //auto sp = sp_vec.at(i);
  //sp_vec.eraseObject(sp1);
  //sp_vec.popBack();
}

void vs_play_scene::add_other_correct_action(float x, float y) {

  // 소리 효과
  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/other_correct.mp3", false, 1.0f, 1.0f, 1.0f);

  Size visibleSize = Director::getInstance()->getVisibleSize();
  float lx = x;
  float rx = x + (offset_x * 2.0f) + (visibleSize.width/2);
  
  auto correct_animation = Animation::create();
  correct_animation->setDelayPerUnit(0.1f);
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct1.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct2.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct3.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct4.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct5.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct6.png");
  correct_animation->addSpriteFrameWithFileName("animation/corrects/correct7.png");
  
  auto sp0 = Sprite::create();
  sp0->setScale(0.2,0.2);
  sp0->setPosition(lx, y);
  sp0->setColor(Color3B(255, 0, 0));
  sp0->runAction(Animate::create(correct_animation));
  this->addChild(sp0, 2);

  auto sp1 = Sprite::create();
  sp1->setScale(0.2,0.2);
  sp1->setPosition(rx, y);
  sp1->setColor(Color3B(255, 0, 0));
  sp1->runAction(Animate::create(correct_animation));
  this->addChild(sp1, 2);

  // 나중에 제거위해서
  vec0.pushBack(sp0);
  vec0.pushBack(sp1);
}

void vs_play_scene::destory_round() {

  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();


  for(auto sp : vec0) {
    this->removeChild(sp);
  } 

  CCLOG("prev vec size: %d", vec0.size());
  vec0.clear();
  CCLOG("after vec size: %d", vec0.size());

  // 다음라운드 준비함


  // 2. 노래 바꾸기

  // 3. 배경 이미지 바꾸기
  stage_cnt_++;
  top_left_stage_font->setString(ccsf(to_string<int>(stage_cnt_+1).c_str()));

  top_left_spot_font->setString(ccsf(to_string<int>(0).c_str()));
  auto max_spot_cnt = round_infos_[stage_cnt_].spots.size();
  top_right_spot_font->setString(ccsf(to_string<int>(max_spot_cnt).c_str()));  


  if(stage_cnt_ < round_infos_.size()) {

    auto left_img = Sprite::create("img/" + round_infos_[stage_cnt_].left_img);
    left_img->setPosition(Vec2((visibleSize.width/2)/2 + origin.x - offset_x, visibleSize.height/2 + origin.y - offset_y));
    this->addChild(left_img, 1);

    auto right_img = Sprite::create("img/" + round_infos_[stage_cnt_].right_img);
    right_img->setPosition(Vec2( (visibleSize.width/2)+(visibleSize.width/2/2) + origin.x + offset_x, visibleSize.height/2 + origin.y  - offset_y));
    this->addChild(right_img, 1);

    // 준비 완료 패킷 보내줌
    int round_cnt = static_cast<int>(stage_cnt_);
    connection::get().send2(Json::object {
	{ "type", "start_round_req" },
	  { "round_cnt", round_cnt }
      });
  }
}

void vs_play_scene::destroy_vs_play() {

  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto scene = vs_room_scene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(1, scene, Color3B(255,0,255)));

}

void vs_play_scene::open_curtain() {
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto moveby_left_curtain = MoveBy::create(1, Vec2(-visibleSize.width/2, 0));
  left_curtain->runAction(moveby_left_curtain);
  auto moveby_right_curtain = MoveBy::create(1, Vec2(+visibleSize.width/2, 0));
  right_curtain->runAction(moveby_right_curtain);
}

void vs_play_scene::close_curtain() {
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto moveby_left_curtain = MoveBy::create(1, Vec2(visibleSize.width/2, 0));
  left_curtain->runAction(moveby_left_curtain);
  auto moveby_right_curtain = MoveBy::create(1, Vec2(-visibleSize.width/2, 0));
  right_curtain->runAction(moveby_right_curtain);
}


//void vs_play_scene::check_find_spot(
/*
sprite->retain();   
scene->removeChild(sprite);
scene->addChild(sprite);
sprite->release();
*/
