#include "SimpleAudioEngine.h"
#include "multi_play_scene.hpp"
#include "multi_lobby_scene.hpp"
#include "multi_room_scene.hpp"
#include "connection.hpp"
#include "assets_scene.hpp"
#include "resource_md.hpp"
#include "single_play_info.hpp"

using namespace CocosDenshion;

Scene* multi_play_scene::createScene() {

  auto scene = Scene::create();
  auto layer = multi_play_scene::create();

  scene->addChild(layer);

  return scene;
}

// on "init" you need to initialize your instance
bool multi_play_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  visible_size = Director::getInstance()->getVisibleSize();
  origin = Director::getInstance()->getVisibleOrigin();
  center = Vec2(visible_size.width/2 + origin.x, visible_size.height/2 + origin.y);

  stage_count = 0;
  max_stage_count = user_info::get().room_info_.stages.size();

  is_end_game = false;

  /*
  if(user_info::get().room_info_.is_master) {
    auto debug_font = Label::createWithTTF("방장 플레이", "fonts/nanumb.ttf", 40);
    debug_font->setPosition(center);
    this->addChild(debug_font, 0);
  } else {
    auto debug_font = Label::createWithTTF("상대 플레이", "fonts/nanumb.ttf", 40);
    debug_font->setPosition(center);
    this->addChild(debug_font, 0);
  }
  */

  stages = user_info::get().room_info_.stages;
  loading_first_stage();


  auto ui_top_bg = Sprite::create("ui/top23.png");
  ui_top_bg->setPosition(Vec2(center.x, center.y + _play_screen_y/2 - _offset_y+0));
  this->addChild(ui_top_bg, 1);
  create_stage_status();

  // 화면 가릴것 2개 로딩하기
  left_block = Sprite::create("ui/hide1.png");
  left_block->setPosition(Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y));
  this->addChild(left_block, 1);

  right_block = Sprite::create("ui/hide1.png");
  right_block->setPosition(Vec2((visible_size.width/2)+(visible_size.width/2/2) + origin.x + _offset_x, visible_size.height/2 + origin.y  - _offset_y));
  this->addChild(right_block, 1);


  auto input_listener = EventListenerTouchOneByOne::create();
  input_listener->setSwallowTouches(true);
 
  input_listener->onTouchBegan = [=](Touch* touch, Event* event) {
    if(!is_playing) return false;

    CCPoint touchLocation = touch->getLocationInView();
    touchLocation = cocos2d::CCDirector::sharedDirector()->convertToGL(touchLocation);
    this->check_user_input(touchLocation.x, touchLocation.y);

    return true;
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(input_listener, this);
  
  this->scheduleUpdate();

  return true;
}

void multi_play_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }
  
  //CCLOG("update");
}

void multi_play_scene::replace_multi_lobby_scene() {
  auto multi_lobby_scene = multi_lobby_scene::createScene();
  Director::getInstance()->replaceScene(multi_lobby_scene);
}

void multi_play_scene::replace_multi_room_scene() {
  auto multi_room_scene = multi_room_scene::createScene();
  Director::getInstance()->replaceScene(multi_room_scene);
}

void multi_play_scene::handle_payload(float dt) {
    Json payload = connection::get().q.front();
    connection::get().q.pop_front();
    std::string type = payload["type"].string_value();

    if(type == "disconnection_notify") {
      CCLOG("[debug] 접속 큰킴");
     

    } else if(type == "start_stage_noti") {
       open_block();
       stage_count = payload["stage_count"].number_value();
       stage_count_font->setString(ccsf2("%d", stage_count+1));

       point_count = 0;
       max_point_count = user_info::get().room_info_.stages[stage_count].hidden_points.size();
       point_count_font->setString(ccsf2("%d", point_count));       
       max_point_count_font->setString(ccsf2("%d", max_point_count));

       CCLOG("현재 스테이지: %d", stage_count);
       is_playing = true;

    } else if(type == "check_point_res") {

      auto is_game_end = payload["is_game_end"].bool_value();
      auto is_stage_end = payload["is_stage_end"].bool_value();
      auto found_type = payload["found_type"].string_value();
      auto x = static_cast<float>(payload["x"].int_value());
      auto y = static_cast<float>(payload["y"].int_value());
      
      set_point_index(Vec2(x, y));
      
      point_count++;
      point_count_font->setString(ccsf2("%d", point_count));

      if((found_type == "master" && user_info::get().room_info_.is_master) ||
         (found_type == "opponent" && !user_info::get().room_info_.is_master)) {
        action_correct(Vec2(x,y));
      } else {
        action_other_correct(Vec2(x,y));
      }


      if(is_game_end) {
        is_end_game = true;
        is_playing = false;
        auto stage_winner = payload["stage_winner"].string_value();
        if((stage_winner == "master" && user_info::get().room_info_.is_master) ||
           (stage_winner == "opponent" && !user_info::get().room_info_.is_master)) {
          // 내가 스테이지 승리
          CCLOG("is game end 스테이지 승리");
          this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::win_stage_end), 1.5f);
        } else {
          // 내가 스테이지 패배
          CCLOG("is game end 스테이지 패배");
          this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::lose_stage_end), 1.5f);
        }

        auto game_winner = payload["game_winner"].string_value();
        if((game_winner == "master" && user_info::get().room_info_.is_master) ||
           (game_winner == "opponent" && !user_info::get().room_info_.is_master)) {
          // 내가 게임 승리(victory 연출)
          this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::victory_game_end), 4.0f);
        } else {
          // 내가 게임 패배(defeat 연출)
          this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::defeat_game_end), 4.0f);
        }


      } else if(is_stage_end) {
        is_playing = false;
        auto stage_winner = payload["stage_winner"].string_value();
        if((stage_winner == "master" && user_info::get().room_info_.is_master) ||
           (stage_winner == "opponent" && !user_info::get().room_info_.is_master)) {
          // 내가 스테이지 승리
          this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::win_stage_end), 1.5f);
        } else {
          // 내가 스테이지 패배
          this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::lose_stage_end), 1.5f);
        }
      }

    } else {
      CCLOG("[error] handler 없음");
    }
}

void multi_play_scene::loading_first_stage() {

  auto img = stages[0].img;
  auto left_img = Sprite::create("img_dummy/" + img + "_left.jpg");
  left_img->setPosition(Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y));
  this->addChild(left_img, 0);

  auto right_img = Sprite::create("img_dummy/" + img + "_right.jpg");
  right_img->setPosition(Vec2( (visible_size.width/2)+(visible_size.width/2/2) + origin.x + _offset_x, visible_size.height/2 + origin.y  - _offset_y));
  this->addChild(right_img, 0);

 connection::get().send2(Json::object {
      { "type", "ready_stage_noti" }
   });
}

void multi_play_scene::loading_next_stage() {

  auto img = stages[stage_count+1].img;
  auto left_img = Sprite::create("img_dummy/" + img + "_left.jpg");
  left_img->setPosition(Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y));
  this->addChild(left_img, 0);

  auto right_img = Sprite::create("img_dummy/" + img + "_right.jpg");
  right_img->setPosition(Vec2( (visible_size.width/2)+(visible_size.width/2/2) + origin.x + _offset_x, visible_size.height/2 + origin.y  - _offset_y));
  this->addChild(right_img, 0);

 connection::get().send2(Json::object {
      { "type", "ready_stage_noti" }
   });

}

Vec2 multi_play_scene::change_device_to_img_pos(float x, float y) {
  const auto half_width = _play_screen_x / 2;
  const auto offset_height = visible_size.height - _play_screen_y;
  // ui 영역 및 나머지 부분 터치했는지 확인 해야함
  y = y - offset_height/2;
  if(x < half_width) {
    return Vec2(x, y);
  }

  x = x - half_width - _offset_x;
  return Vec2(x, y);
}

void multi_play_scene::check_user_input(float x, float y) {
  Vec2 img_pos = change_device_to_img_pos(x, y);
  CCLOG("x: %f", img_pos.x);
  CCLOG("y: %f", img_pos.y);

  auto index = check_point(img_pos.x, img_pos.y);

  if(index >= 0) {
    CCLOG("맞춤");
    check_point_req(index);
  } else {
    CCLOG("틀림");
    action_incorrect(x, y);
  }
}

bool multi_play_scene::is_found_point(int index, stage& game_stage) {

  for(auto i : game_stage.found_indexs) {
    CCLOG("contained index: %d", i);
  }

  auto it = game_stage.found_indexs.find(index);
  if(it != game_stage.found_indexs.end()) return true;

  return false;
}

int multi_play_scene::check_point(float x, float y) {

  auto& game_stage = stages[stage_count];

  auto index = 0;
  for(auto& point : game_stage.hidden_points) {
    if(is_point_in_area(x, y, point.x, point.y)) {
      if(is_found_point(index, game_stage)) {
        CCLOG("이미 찾은 포인트라서 false 처리");
        return -1;
      }

      return index;
    }
    index++;
  }

  return -1;
}

bool multi_play_scene::is_point_in_area(float ux, float uy, float xc, float yc, float r) {
   return ((ux-xc)*(ux-xc) + (uy-yc)*(uy-yc)) < r*r;
}

void multi_play_scene::check_point_req(int index) {
  Vec2 point = stages[stage_count].hidden_points[index];
  int x = static_cast<int>(point.x);
  int y = static_cast<int>(point.y);
  connection::get().send2(Json::object {
      { "type", "check_point_req" },
      { "stage_count", stage_count },
      { "x", x },
      { "y", y }
   });
}

void multi_play_scene::set_point_index(Vec2 point) {
  auto& game_stage = stages[stage_count];
  auto index = 0;
  for(auto& p : game_stage.hidden_points) {
    if(p == point) {
      game_stage.found_indexs.insert(index);
      return;
    }
    index++;
  }
}

void multi_play_scene::action_correct(Vec2 point) {
 auto audio = SimpleAudioEngine::getInstance();
  srand(time(NULL));
  auto r = rand() % 4;
  if(r == 0) {
    audio->playEffect("sound/great.wav", false, 1.0f, 1.0f, 1.0f);
  } else if(r==1) {
    audio->playEffect("sound/good.wav", false, 1.0f, 1.0f, 1.0f);
  } else if(r==2) {
    audio->playEffect("sound/cool.wav", false, 1.0f, 1.0f, 1.0f);
  } else {
    audio->playEffect("sound/yeah.wav", false, 1.0f, 1.0f, 1.0f);
  }

  auto circle_animation = Animation::create();
  circle_animation->setDelayPerUnit(0.1f);
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle0.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle1.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle2.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle3.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle4.png");

  Vec2 left_pos = change_img_to_device_pos(true, point.x, point.y);
  auto left_spot = CCSprite::create("animation/corrects/circle0.png");
  left_spot->setPosition(Vec2(left_pos.x, left_pos.y));
  left_spot->setScale(0.5f);

  left_spot->runAction(Animate::create(circle_animation));
  this->addChild(left_spot, 0);


  Vec2 right_pos = change_img_to_device_pos(false, point.x, point.y);
  auto right_spot = CCSprite::create("animation/corrects/circle0.png");
  right_spot->setPosition(Vec2(right_pos.x, right_pos.y));
  right_spot->setScale(0.5f);

  right_spot->runAction(Animate::create(circle_animation));
  this->addChild(right_spot, 0);
}

void multi_play_scene::action_other_correct(Vec2 point) {
  auto circle_animation = Animation::create();
  circle_animation->setDelayPerUnit(0.1f);
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle0.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle1.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle2.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle3.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle4.png");

  Vec2 left_pos = change_img_to_device_pos(true, point.x, point.y);
  auto left_spot = CCSprite::create("animation/corrects/circle0.png");
  left_spot->setPosition(Vec2(left_pos.x, left_pos.y));
  left_spot->setScale(0.5f);
  left_spot->setColor(Color3B(255, 0, 0)); 

  left_spot->runAction(Animate::create(circle_animation));
  this->addChild(left_spot, 0);


  Vec2 right_pos = change_img_to_device_pos(false, point.x, point.y);
  auto right_spot = CCSprite::create("animation/corrects/circle0.png");
  right_spot->setPosition(Vec2(right_pos.x, right_pos.y));
  right_spot->setScale(0.5f);
  right_spot->setColor(Color3B(255, 0, 0)); 

  right_spot->runAction(Animate::create(circle_animation));
  this->addChild(right_spot, 0);

}

void multi_play_scene::action_incorrect(float x, float y) {

}

void multi_play_scene::victory_game_end() {
  CCLOG("게임에서 승리");
  replace_multi_room_scene();
}

void multi_play_scene::defeat_game_end() {
  CCLOG("게임에서 패배");
  replace_multi_room_scene();
}

void multi_play_scene::win_stage_end() {
  close_block();
  user_info::get().room_info_.stages[stage_count].is_win = true;

  auto audio = SimpleAudioEngine::getInstance();
  audio->stopBackgroundMusic();

  auto youwin = Sprite::create("ui/youwin.png");
  youwin->setScale(2.0f);
  youwin->setPosition(Vec2(visible_size.width + 100.0f, center.y));
  this->addChild(youwin, 2);

  auto moveTo = MoveTo::create(1.2f, Vec2(center.x, center.y));
  auto fadeOut = FadeOut::create(1.5f);
  auto seq = Sequence::create(moveTo, fadeOut, nullptr);
  youwin->runAction(seq);

  audio->playEffect("sound/YouWin.wav", false, 1.0f, 1.0f, 1.0f);
  // 문이 닫히면서 연출
 
  // 스테이지 종료 noti하고
  // 다음 씬준비
  if(!is_end_game) {
    this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::  loading_next_stage), 2.0f);
  }
}

void multi_play_scene::lose_stage_end() {
  close_block();
  user_info::get().room_info_.stages[stage_count].is_win = false;
  // 문이 닫히면서 연출
  auto youfail = Sprite::create("ui/youfail.png");
  youfail->setScale(2.0f);
  youfail->setPosition(Vec2(visible_size.width + 100.0f, center.y));
  this->addChild(youfail, 2);

  auto moveTo = MoveTo::create(1.2f, Vec2(center.x, center.y));
  auto fadeOut = FadeOut::create(1.5f);
  auto seq = Sequence::create(moveTo, fadeOut, nullptr);
  youfail->runAction(seq);
  
  // 스테이지 종료 noti하고
  // 다음 씬준비
  if(!is_end_game) {
    this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::  loading_next_stage), 2.0f);
  }
}

void multi_play_scene::found_point(bool found_point, Vec2 point) {

}

Vec2 multi_play_scene::change_img_to_device_pos(bool is_left, float x, float y) {
  const auto half_width = _play_screen_x / 2;
  const auto offset_height = (visible_size.height - _play_screen_y) / 2;
  y = y + offset_height;

  if(is_left) {
    return Vec2(x, y);
  }
  x = x + half_width + _offset_x;
  return Vec2(x, y);
}

void multi_play_scene::open_block() {
  auto lx = left_block->getPosition().x - left_block->getContentSize().width;
  auto rx = right_block->getPosition().x + right_block->getContentSize().width;

  auto moveTo = MoveTo::create(0.8f, Vec2(lx, left_block->getPosition().y)); 
  left_block->runAction(moveTo);

  auto moveTo2 = MoveTo::create(0.8f, Vec2(rx, right_block->getPosition().y)); 
  right_block->runAction(moveTo2);
}

void multi_play_scene::close_block() {
  auto moveTo = MoveTo::create(0.8f, Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y)); 
  left_block->runAction(moveTo);

  auto moveTo2 = MoveTo::create(0.8f, Vec2((visible_size.width/2)+(visible_size.width/2/2) + origin.x + _offset_x, visible_size.height/2 + origin.y  - _offset_y)); 
  right_block->runAction(moveTo2);
}

void multi_play_scene::create_stage_status() {

  auto ui_offset_x = 70;
  auto font_size = 30;
  
  auto font_x = visible_size.width/2 + ui_offset_x;
  auto font_y = center.y + _play_screen_y/2 - _offset_y+0;
  font_y = font_y + 1;


  stage_count_font = Label::createWithTTF(ccsf2("%d", stage_count+1), "fonts/nanumb.ttf", font_size);
  stage_count_font->setPosition(Vec2(font_x + 80, font_y));
  stage_count_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(stage_count_font, 1);
 
  auto stage_slash_font = Label::createWithTTF("/", "fonts/nanumb.ttf", font_size);
  stage_slash_font->setPosition(Vec2(font_x + 105, font_y));
  stage_slash_font->setColor( Color3B( 225, 225, 225) );
  this->addChild(stage_slash_font, 1);

  max_stage_count_font = Label::createWithTTF(ccsf2("%d", max_stage_count), "fonts/nanumb.ttf", font_size);
  max_stage_count_font->setPosition(Vec2(font_x + 130, font_y));
  max_stage_count_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(max_stage_count_font, 1);
  
  

  point_count_font = Label::createWithTTF("0", "fonts/nanumb.ttf", font_size);
  point_count_font->setPosition(Vec2(font_x + 300, font_y));
  point_count_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(point_count_font, 1);

  auto point_slash_font = Label::createWithTTF("/", "fonts/nanumb.ttf", font_size);
  point_slash_font->setPosition(Vec2(font_x + 325, font_y));
  point_slash_font->setColor( Color3B( 225, 225, 225) );
  this->addChild(point_slash_font, 1);

  max_point_count_font = Label::createWithTTF(ccsf2("%d", max_point_count), "fonts/nanumb.ttf", font_size);
  max_point_count_font->setPosition(Vec2(font_x + 350, font_y));
  max_point_count_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(max_point_count_font, 1); 
}
