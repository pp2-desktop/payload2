#include "SimpleAudioEngine.h"
#include "multi_play_scene.hpp"
#include "multi_lobby_scene.hpp"
#include "multi_room_scene.hpp"
#include "connection.hpp"
#include "assets_scene.hpp"
#include "resource_md.hpp"
#include "single_play_info.hpp"
#include "lobby_scene.hpp"

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

  auto audio = SimpleAudioEngine::getInstance();
  audio->playBackgroundMusic("sound/besound_acousticbreeze.mp3", true);

  stage_count = 0;
  max_stage_count = user_info::get().room_info_.stages.size();

  is_end_game = false;
  img_complete_cnt = 0;

  master_score = 0;
  opponent_score = 0;

  is_incorrect_action = false;
  is_perfect_stage = true;
  perfect_stage_cnt = 0;
  is_leave_user = false;

  resource_status_font = Label::createWithTTF("이미지 다운로드 중", "fonts/nanumb.ttf", 50);
  resource_status_font->setPosition(Vec2(center.x, center.y));
  resource_status_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(resource_status_font, 3);

  auto scaleTo = ScaleTo::create(1.1f, 1.1f);
  resource_status_font->runAction(scaleTo);
  auto delay = DelayTime::create(0.25f);
  auto scaleTo2 = ScaleTo::create(1.0f, 1.0f);
  auto seq = Sequence::create(scaleTo, delay, scaleTo2,
			       delay->clone(), nullptr);
  resource_status_font->runAction(RepeatForever::create(seq));
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
  loading_first_stage2();

  auto ui_top_bg = Sprite::create("ui/multi_play_top24.png");
  ui_top_bg->setPosition(Vec2(center.x, center.y + _play_screen_y/2 - _offset_y+0));
  this->addChild(ui_top_bg, 1);
  create_stage_status();


  // 화면 가릴것 2개 로딩하기
  left_block = Sprite::create("ui/hide1.png");
  left_block->setPosition(Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y));
  this->addChild(left_block, 2);

  right_block = Sprite::create("ui/hide1.png");
  right_block->setPosition(Vec2((visible_size.width/2)+(visible_size.width/2/2) + origin.x + _offset_x, visible_size.height/2 + origin.y  - _offset_y));
  this->addChild(right_block, 2);


  auto input_listener = EventListenerTouchOneByOne::create();
  input_listener->setSwallowTouches(true);
 
  input_listener->onTouchBegan = [=](Touch* touch, Event* event) {
    if(!is_playing || is_incorrect_action) return false;

    CCPoint touchLocation = touch->getLocationInView();
    touchLocation = cocos2d::CCDirector::sharedDirector()->convertToGL(touchLocation);
    this->check_user_input(touchLocation.x, touchLocation.y);

    return true;
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(input_listener, this);
 
  create_leave_user_popup();
  create_connection_popup();

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
      open_connection_popup();

    } else if(type == "update_alive_noti") { 
      CCLOG("[noti] update alive noti");
      connection::get().send2(Json::object {
	  { "type", "update_alive_noti" }
	});
    } else if(type == "start_stage_noti") {
      //resource_status_font->setPosition(Vec2(center.x + 5000.0f, center.y));
       open_block();
       stage_count = payload["stage_count"].number_value();
       stage_count_font->setString(ccsf2("%d", stage_count+1));

       point_count = 0;
       max_point_count = user_info::get().room_info_.stages[stage_count].hidden_points.size();
       point_count_font->setString(ccsf2("%d", point_count));       
       max_point_count_font->setString(ccsf2("%d", max_point_count));

       CCLOG("현재 스테이지: %d", stage_count);
       is_playing = true;
       resource_status_font->setPosition(Vec2(center.x+5000.0f, center.y));
       resource_status_font->setString("이미지 다운로드 중");
       is_perfect_stage = true;

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
          this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::win_stage_end), 1.8f);
        } else {
          // 내가 스테이지 패배
          CCLOG("is game end 스테이지 패배");
          this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::lose_stage_end), 1.8f);
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
          this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::win_stage_end), 1.8f);
        } else {
          // 내가 스테이지 패배
          this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::lose_stage_end), 1.8f);
        }

	if(stage_winner == "master") {
	  master_score++;
	} else {
	  opponent_score++;
	}
      }

    } else if(type == "game_end_noti") {
      CCLOG("상대 유저 나감");
      is_playing = false;
      close_block();
      open_leave_user_popup();
      //this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::replace_multi_lobby_scene), 1.0f);
      //replace_multi_lobby_scene();
      // 상대가 나가면 승리 처리해주고 방으로 이동한다

    } else if(type == "update_game_info_noti") {
      user_info::get().account_info_.score = payload["score"].int_value();
      user_info::get().account_info_.win_count = payload["win_count"].int_value();
      user_info::get().account_info_.lose_count = payload["lose_count"].int_value();
      user_info::get().account_info_.ranking = payload["ranking"].int_value();

    } else {
      CCLOG("[error] handler 없음");
      CCLOG("type: %s", type.c_str());
    }
}

void multi_play_scene::loading_first_stage() {
  auto img = stages[0].img;

  left_img = Sprite::create("img_dummy/" + img + "_left.jpg");
  left_img->setPosition(Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y));
  this->addChild(left_img, 0);

  right_img = Sprite::create("img_dummy/" + img + "_right.jpg");
  right_img->setPosition(Vec2( (visible_size.width/2)+(visible_size.width/2/2) + origin.x + _offset_x, visible_size.height/2 + origin.y  - _offset_y));
  this->addChild(right_img, 0);

 connection::get().send2(Json::object {
      { "type", "ready_stage_noti" }
   });
}

void multi_play_scene::loading_next_stage() {
  this->removeChild(left_img);
  this->removeChild(right_img);

  for(auto& sprite : correct_spots) {
    this->removeChild(sprite);
  }

  for(auto& sprite : other_correct_spots) {
    this->removeChild(sprite);
  }

  auto img = stages[stage_count+1].img;
  left_img = Sprite::create("img_dummy/" + img + "_left.jpg");
  left_img->setPosition(Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y));
  this->addChild(left_img, 0);

  right_img = Sprite::create("img_dummy/" + img + "_right.jpg");
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

  correct_spots.push_back(left_spot);
  correct_spots.push_back(right_spot);
}

void multi_play_scene::action_other_correct(Vec2 point) {
  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/multi_other_point_noti.wav", false, 1.0f, 1.0f, 1.0f);

  auto circle_animation = Animation::create();
  circle_animation->setDelayPerUnit(0.1f);
  circle_animation->addSpriteFrameWithFileName("animation/incorrects/circle0.png");
  circle_animation->addSpriteFrameWithFileName("animation/incorrects/circle1.png");
  circle_animation->addSpriteFrameWithFileName("animation/incorrects/circle2.png");
  circle_animation->addSpriteFrameWithFileName("animation/incorrects/circle3.png");
  circle_animation->addSpriteFrameWithFileName("animation/incorrects/circle4.png");

  Vec2 left_pos = change_img_to_device_pos(true, point.x, point.y);
  auto left_spot = CCSprite::create("animation/corrects/circle0.png");
  left_spot->setPosition(Vec2(left_pos.x, left_pos.y));
  left_spot->setScale(0.5f);
  //left_spot->setColor(Color3B(255, 0, 0)); 

  left_spot->runAction(Animate::create(circle_animation));
  this->addChild(left_spot, 0);


  Vec2 right_pos = change_img_to_device_pos(false, point.x, point.y);
  auto right_spot = CCSprite::create("animation/corrects/circle0.png");
  right_spot->setPosition(Vec2(right_pos.x, right_pos.y));
  right_spot->setScale(0.5f);
  //right_spot->setColor(Color3B(255, 0, 0)); 

  right_spot->runAction(Animate::create(circle_animation));
  this->addChild(right_spot, 0);

  other_correct_spots.push_back(left_spot);
  other_correct_spots.push_back(right_spot);

  is_perfect_stage = false;
}

void multi_play_scene::action_incorrect(float x, float y) {
  is_incorrect_action = true;
  this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::release_incorrect_action), 0.75f);
  
  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/incorrect2.wav", false, 1.0f, 1.0f, 1.0f);

  auto incorrect2 = Sprite::create("ui/incorrect2.png");
  incorrect2->setScale(0.5f);
  incorrect2->setPosition(Vec2(x,y));
  this->addChild(incorrect2);
  auto fadeOut = FadeOut::create(0.75f);
  incorrect2->runAction(fadeOut);
  
  auto moveBy0 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy1 = MoveBy::create(0.05f, Vec2(20, 0));
  auto moveBy2 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy3 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy4 = MoveBy::create(0.05f, Vec2(20, 0));
  auto moveBy5 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy6 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy7 = MoveBy::create(0.05f, Vec2(20, 0));
  auto moveBy8 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy9 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy10 = MoveBy::create(0.05f, Vec2(20, 0));
  auto moveBy11 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy12 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy13 = MoveBy::create(0.05f, Vec2(20, 0));
  auto moveBy14 = MoveBy::create(0.05f, Vec2(-10, 0));

  auto seq = Sequence::create(moveBy0, moveBy1, moveBy2, moveBy3, moveBy4, moveBy5, moveBy6, moveBy7, moveBy8, moveBy9, moveBy10, moveBy11, moveBy12, moveBy13, moveBy14, nullptr);

  if(x < visible_size.width / 2.0f) {
    left_img->runAction(seq);
  } else {
    right_img->runAction(seq);
  }
}

void multi_play_scene::victory_game_end() {
  CCLOG("게임에서 승리");  
  auto background = Sprite::create("background/victory2.jpg");
  background->setPosition(Vec2(center));
  this->addChild(background, 1);
  create_game_result(true);

  auto font_size = 38;
  auto x = 290;

  auto earn_score_font = Label::createWithTTF(ccsf2("%d 점", user_info::get().account_info_.earn_score), "fonts/nanumb.ttf", font_size);
  earn_score_font->setPosition(Vec2(x, center.y + 129));
  earn_score_font->setColor( Color3B( 255, 255, 255) );
  earn_score_font->setAnchorPoint(ccp(0,0.5f));
  this->addChild(earn_score_font, 1);

  auto perfect_score_font = Label::createWithTTF(ccsf2("2점  x  %d", perfect_stage_cnt), "fonts/nanumb.ttf", font_size);
  perfect_score_font->setPosition(Vec2(x, center.y + 73));
  perfect_score_font->setColor( Color3B( 255, 255, 255) );
  perfect_score_font->setAnchorPoint(ccp(0,0.5f));
  this->addChild(perfect_score_font, 1);

  font_size = 40;

  auto score_font = Label::createWithTTF(ccsf2("%d 점", user_info::get().account_info_.score), "fonts/nanumb.ttf", font_size);
  score_font->setPosition(Vec2(x, center.y - 23));
  score_font->setColor( Color3B( 255, 255, 255) );
  score_font->setAnchorPoint(ccp(0,0.5f));
  this->addChild(score_font, 1);

  open_block();
}

void multi_play_scene::defeat_game_end() {
  CCLOG("게임에서 패배");
  auto background = Sprite::create("background/defeat2.jpg");
  background->setPosition(Vec2(center));
  this->addChild(background, 1);
  create_game_result(false);

  auto font_size = 38;
  auto x = 290;

  auto lose_score_font = Label::createWithTTF(ccsf2("- %d 점", user_info::get().account_info_.lose_score), "fonts/nanumb.ttf", font_size);
  lose_score_font->setPosition(Vec2(x, center.y + 103));
  lose_score_font->setColor( Color3B( 255, 255, 255) );
  lose_score_font->setAnchorPoint(ccp(0,0.5f));
  this->addChild(lose_score_font, 1);

  auto perfect_score_font = Label::createWithTTF(ccsf2("2점  x  %d", perfect_stage_cnt), "fonts/nanumb.ttf", font_size);
  perfect_score_font->setPosition(Vec2(x, center.y + 27));
  perfect_score_font->setColor( Color3B( 255, 255, 255) );
  perfect_score_font->setAnchorPoint(ccp(0,0.5f));
  this->addChild(perfect_score_font, 1);

  font_size = 40;
  
  auto score_font = Label::createWithTTF(ccsf2("%d 점", user_info::get().account_info_.score), "fonts/nanumb.ttf", font_size);
  score_font->setPosition(Vec2(x, center.y - 83));
  score_font->setColor( Color3B( 255, 255, 255) );
  score_font->setAnchorPoint(ccp(0,0.5f));
  this->addChild(score_font, 1);

  open_block();
}

void multi_play_scene::create_game_result(bool is_victory) {  
  result_confirm_button = ui::Button::create();
  result_confirm_button->setTouchEnabled(true);
  result_confirm_button->ignoreContentAdaptWithSize(false);
  result_confirm_button->setContentSize(Size(286.0f, 126.0f));
  if(is_victory) {
    result_confirm_button->loadTextures("ui/confirm2_button.png", "ui/confirm2_button.png");
  } else {
    result_confirm_button->loadTextures("ui/confirm_button.png", "ui/confirm_button.png");
  }
  result_confirm_button->setPosition(Vec2(center.x, center.y - 200.0f));

  result_confirm_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 1.1f);
        result_confirm_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
        result_confirm_button->runAction(scaleTo2);

	if(is_leave_user) {
	  replace_multi_lobby_scene();
	} else {
	  replace_multi_room_scene();
	}

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 1.0f);
        result_confirm_button->runAction(scaleTo);
      }
    });
     
  this->addChild(result_confirm_button, 1);
}

void multi_play_scene::win_stage_end() {
  close_block();
  user_info::get().room_info_.stages[stage_count].is_win = true;

  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/YouWin.wav", false, 1.0f, 1.0f, 1.0f);

  auto youwin = Sprite::create("ui/youwin.png");
  youwin->setScale(2.0f);
  youwin->setPosition(Vec2(visible_size.width + 100.0f, center.y));
  this->addChild(youwin, 2);

  if(!is_perfect_stage) {
    auto moveTo = MoveTo::create(1.2f, Vec2(center.x, center.y));
    auto fadeOut = FadeOut::create(1.5f);
    auto seq = Sequence::create(moveTo, fadeOut, nullptr);
    youwin->runAction(seq);
  } else {
    perfect_stage_cnt++;
    auto moveTo = MoveTo::create(0.8f, Vec2(center.x, center.y));
    auto fadeOut = FadeOut::create(0.5f);
    auto seq = Sequence::create(moveTo, fadeOut, nullptr);
    youwin->runAction(seq);
    this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::perfect_action), 0.8f);
  }

  // 문이 닫히면서 연출
 
  // 스테이지 종료 noti하고
  // 다음 씬준비
  if(!is_end_game) {
    this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::loading_next_stage2), 2.0f);
  }
}

void multi_play_scene::lose_stage_end() {
  close_block();
  user_info::get().room_info_.stages[stage_count].is_win = false;
  // 문이 닫히면서 연출
  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/YouFailed.wav", false, 1.0f, 1.0f, 1.0f);

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
    this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::loading_next_stage2), 2.0f);
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

  auto ui_offset_x = 170;
  auto font_size = 30;
  
  auto font_x = visible_size.width/4 + ui_offset_x;
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

  auto name_left_font = Label::createWithTTF(user_info::get().account_info_.get_name(), "fonts/nanumb.ttf", font_size);
  name_left_font->setPosition(Vec2(15, font_y));
  name_left_font->setColor( Color3B( 255, 255, 255) );
  name_left_font->setAnchorPoint(ccp(0,0.5f));
  this->addChild(name_left_font, 1);

  master_score_img = Sprite::create("ui/score0.png");
  master_score_img->setPosition(Vec2(name_left_font->getPosition().x + name_left_font->getContentSize().width + (master_score_img->getContentSize().width/2.0f) + 10.0f, font_y));
  this->addChild(master_score_img, 1);

  auto name_right_font = Label::createWithTTF(user_info::get().account_info_.get_other_name(), "fonts/nanumb.ttf", font_size);
  auto x = visible_size.width - name_right_font->getContentSize().width;
  name_right_font->setPosition(Vec2(x-15, font_y));
  name_right_font->setColor( Color3B( 255, 255, 255) );
  name_right_font->setAnchorPoint(ccp(0,0.5f)); 
  this->addChild(name_right_font, 1);

  opponent_score_img = Sprite::create("ui/score0.png");
  opponent_score_img->setPosition(Vec2(x - 50.0f, font_y));
  this->addChild(opponent_score_img, 1);


  if(!(user_info::get().room_info_.is_master)) {
    name_left_font->setString(user_info::get().account_info_.get_other_name());
    name_right_font->setString(user_info::get().account_info_.get_name());
  }

}

void multi_play_scene::create_connection_popup() {
  auto offset = 5000.0f;
  connection_background_popup = Sprite::create("ui/background_popup.png");
  connection_background_popup->setScale(2.0f);
  connection_background_popup->setPosition(Vec2(center.x + offset, center.y));
  this->addChild(connection_background_popup, 2);

  connection_noti_font = Label::createWithTTF("네트워크 불안정 상태로 서버와 접속 끊김", "fonts/nanumb.ttf", 40);
  connection_noti_font->setPosition(Vec2(center.x + offset, center.y));
  connection_noti_font->setColor(Color3B( 110, 110, 110));
  this->addChild(connection_noti_font, 2);

  connection_confirm_button = ui::Button::create();
  connection_confirm_button->setTouchEnabled(true);
  connection_confirm_button->ignoreContentAdaptWithSize(false);
  connection_confirm_button->setContentSize(Size(286.0f, 126.0f));
  connection_confirm_button->loadTextures("ui/confirm_button.png", "ui/confirm_button.png");
  connection_confirm_button->setPosition(Vec2(center.x + offset, center.y));

  connection_confirm_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 1.1f);
        connection_confirm_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
        connection_confirm_button->runAction(scaleTo2);
        if(!connection::get().get_is_connected()) {
          connection::get().create("ws://t.05day.com:8080/echo");
          connection::get().connect();
        }
        auto lobby_scene = lobby_scene::createScene();
        Director::getInstance()->replaceScene(lobby_scene);
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 1.0f);
        connection_confirm_button->runAction(scaleTo);
      }
    });
     
  this->addChild(connection_confirm_button, 2);
}

void multi_play_scene::open_connection_popup() {
  connection_background_popup->setPosition(Vec2(center));
  connection_noti_font->setPosition(Vec2(center.x, center.y + 60.0f));
  connection_confirm_button->setPosition(Vec2(center.x, center.y - 100.0f));
  resource_status_font->setPosition(Vec2(center.x+5000.0f, center.y));
}

void multi_play_scene::close_connection_popup() {
  auto offset = 5000.0f;
  connection_background_popup->setPosition(Vec2(center.x + offset, center.y));
  connection_noti_font->setPosition(Vec2(center.x + offset, center.y + 60.0f));
  connection_confirm_button->setPosition(Vec2(center.x + offset, center.y - 100.0f));
}

void multi_play_scene::create_leave_user_popup() {
  auto offset = 5000.0f;
  leave_user_background_popup = Sprite::create("ui/background_popup.png");
  leave_user_background_popup->setScale(2.0f);
  leave_user_background_popup->setPosition(Vec2(center.x + offset, center.y));
  this->addChild(leave_user_background_popup, 2);

  leave_user_noti_font = Label::createWithTTF("상대가 네트워크 상태 불안정으로 게임을 나감", "fonts/nanumb.ttf", 40);
  leave_user_noti_font->setPosition(Vec2(center.x + offset, center.y));
  leave_user_noti_font->setColor(Color3B( 110, 110, 110));
  this->addChild(leave_user_noti_font, 2);

  leave_user_confirm_button = ui::Button::create();
  leave_user_confirm_button->setTouchEnabled(true);
  leave_user_confirm_button->ignoreContentAdaptWithSize(false);
  leave_user_confirm_button->setContentSize(Size(286.0f, 126.0f));
  leave_user_confirm_button->loadTextures("ui/confirm_button.png", "ui/confirm_button.png");
  leave_user_confirm_button->setPosition(Vec2(center.x + offset, center.y));

  leave_user_confirm_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 1.1f);
	leave_user_confirm_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	leave_user_confirm_button->runAction(scaleTo2);

	close_leave_user_popup();

	is_leave_user = true;
	victory_game_end();
	//this->scheduleOnce(SEL_SCHEDULE(&multi_play_scene::replace_multi_lobby_scene), 1.0f);

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 1.0f);
        leave_user_confirm_button->runAction(scaleTo);
      }
    });
     
  this->addChild(leave_user_confirm_button, 2);
}

void multi_play_scene::open_leave_user_popup() {
  leave_user_background_popup->setPosition(Vec2(center));
  leave_user_noti_font->setPosition(Vec2(center.x, center.y + 60.0f));
  leave_user_confirm_button->setPosition(Vec2(center.x, center.y - 100.0f));
  resource_status_font->setPosition(Vec2(center.x+5000.0f, center.y));
}

void multi_play_scene::close_leave_user_popup() {
  auto offset = 5000.0f;
  leave_user_background_popup->setPosition(Vec2(center.x + offset, center.y));
  leave_user_noti_font->setPosition(Vec2(center.x + offset, center.y + 60.0f));
  leave_user_confirm_button->setPosition(Vec2(center.x + offset, center.y - 100.0f));
}

void multi_play_scene::loading_first_stage2() {
  img_complete_cnt = 0;
  auto img = stages[0].img;
  start_get_img(true, img);
  start_get_img(false, img);
}


void multi_play_scene::loading_next_stage2() {
  resource_status_font->setPosition(Vec2(center.x, center.y));
  this->removeChild(left_img);
  this->removeChild(right_img);

  for(auto& sprite : correct_spots) {
    this->removeChild(sprite);
  }

  for(auto& sprite : other_correct_spots) {
    this->removeChild(sprite);
  }

  img_complete_cnt = 0;
  auto img = stages[stage_count+1].img;
  start_get_img(true, img);
  start_get_img(false, img);
  
  update_master_score_img(master_score);
  update_opponent_score_img(opponent_score);
}

void multi_play_scene::start_get_img(bool is_left, std::string img) {
  cocos2d::network::HttpRequest* request = new (std::nothrow) cocos2d::network::HttpRequest();

  if(is_left) {
    string url = "https://s3.ap-northeast-2.amazonaws.com/payload2/" + img + "_left.jpg";
    request->setUrl(url.c_str());
    request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
    request->setResponseCallback(CC_CALLBACK_2(multi_play_scene::on_request_left_img_completed, this));
    request->setTag("get_left_img");
  } else {
    string url = "https://s3.ap-northeast-2.amazonaws.com/payload2/" + img + "_right.jpg";
    request->setUrl(url.c_str());
    request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
    request->setResponseCallback(CC_CALLBACK_2(multi_play_scene::on_request_right_img_completed, this));
    request->setTag("get_right_img");
  }

  cocos2d::network::HttpClient::getInstance()->send(request);
  request->release();
}

void multi_play_scene::on_request_left_img_completed(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response) {

  if(!response) {
    return;
  }

  if(!response->isSucceed()) {
    return;
  }

  std::vector<char>* buffer = response->getResponseData();

  Image* image = new Image();
  image->initWithImageData ( reinterpret_cast<const unsigned char*>(&(buffer->front())), buffer->size());
  left_texture.initWithImage(image);

  left_img = Sprite::createWithTexture(&left_texture);
  left_img->setPosition(Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y));
  this->addChild(left_img, 0);
  delete image;

  img_complete_cnt++;
  if(img_complete_cnt > 1) {
    connection::get().send2(Json::object {
        { "type", "ready_stage_noti" }
      });
  }
}

void multi_play_scene::on_request_right_img_completed(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response) {

  if(!response) {
    return;
  }

  if(!response->isSucceed()) {
    return;
  }

  std::vector<char>* buffer = response->getResponseData();

  Image* image = new Image();
  image->initWithImageData ( reinterpret_cast<const unsigned char*>(&(buffer->front())), buffer->size());

  right_texture.initWithImage(image);
    

  right_img = Sprite::createWithTexture(&right_texture);
  right_img->setPosition(Vec2( (visible_size.width/2)+(visible_size.width/2/2) + origin.x + _offset_x, visible_size.height/2 + origin.y  - _offset_y));
  this->addChild(right_img, 0);
  delete image;

  img_complete_cnt++;
  if(img_complete_cnt > 1) {
    CCLOG("ready_stage_noti 보냄");
    connection::get().send2(Json::object {
        { "type", "ready_stage_noti" }
      });
  }

  resource_status_font->setString("상대편 이미지 다운로드 기다리는 중");
}

void multi_play_scene::update_master_score_img(int score) {
  auto pos = master_score_img->getPosition();
  std::string img = "";
  if(score == 0) {
    return;
  } else if(score == 1) {
    img = "ui/score1.png";
  } else if(score == 2) {
    img = "ui/score2.png";
  } else if(score == 3) {
    img = "ui/score3.png";
  } else if(score == 4) {
    img = "ui/score4.png";
  } else {
    img = "ui/score5.png";
  }

  auto tmp = Sprite::create(img);
  tmp->setPosition(pos);
  this->addChild(tmp, 1);
}

void multi_play_scene::update_opponent_score_img(int score) {
  auto pos = opponent_score_img->getPosition();
  std::string img = "";
  if(score == 0) {
    return;
  } else if(score == 1) {
    img = "ui/score1.png";
  } else if(score == 2) {
    img = "ui/score2.png";
  } else if(score == 3) {
    img = "ui/score3.png";
  } else if(score == 4) {
    img = "ui/score4.png";
  } else {
    img = "ui/score5.png";
  }

  auto tmp = Sprite::create(img);
  tmp->setPosition(pos);
  this->addChild(tmp, 1);
}

void multi_play_scene::release_incorrect_action() {
  is_incorrect_action = false;
}

void multi_play_scene::perfect_action() {
  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/multi_perfect.wav", false, 1.0f, 1.0f, 1.0f);

  auto perfect = Sprite::create("ui/perfect.png");
  perfect->setScale(1.5f);
  perfect->setPosition(Vec2(center.x, center.y + 500.0f));
  this->addChild(perfect, 2);

  auto moveTo = MoveTo::create(0.4f, Vec2(center.x, center.y));
  auto fadeOut = FadeOut::create(1.0f);
  auto seq = Sequence::create(moveTo, fadeOut, nullptr);
  perfect->runAction(seq);
}
