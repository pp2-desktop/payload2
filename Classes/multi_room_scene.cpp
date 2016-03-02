#include "SimpleAudioEngine.h"
#include "multi_room_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"
#include "lobby_scene.hpp"
#include "multi_lobby_scene.hpp"
#include "multi_play_scene.hpp"
#include "assets_scene.hpp"
#include "resource_md.hpp"
#include "single_play_info.hpp"
//#include "single_play_scene.hpp"
using namespace CocosDenshion;

Scene* multi_room_scene::createScene() {

  auto scene = Scene::create();
  auto layer = multi_room_scene::create();

  scene->addChild(layer);

  return scene;
}

// on "init" you need to initialize your instance
bool multi_room_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);
    
  if(user_info::get().sound_option_.get_background()) {
    auto audio = SimpleAudioEngine::getInstance();
    audio->playBackgroundMusic("sound/besound_thelounge.mp3", true);
  }
  //
  /* 
  auto background = Sprite::create("background/lobby_scene.png");
  background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background, 0);
  */

  auto background = Sprite::create("background/vs_play_scene.png");
  background->setPosition(Vec2(center_.x, center_.y));
  this->addChild(background, 0);

  auto vs = Sprite::create("ui/vs.png");
  vs->setPosition(Vec2(center_.x, center_.y));
  vs->setScale(0.5f);
  this->addChild(vs, 0);

  master_profile_background = Sprite::create("ui/profile_background.jpg");
  master_profile_background->setPosition(Vec2(Director::getInstance()->getVisibleSize().width / 4 - 25 + 10, center_.y+40));
  master_profile_background->setOpacity(80);
  this->addChild(master_profile_background, 0);

  opponent_profile_background = Sprite::create("ui/profile_background.jpg");
  opponent_profile_background->setPosition(Vec2(center_.x + (Director::getInstance()->getVisibleSize().width), center_.y+40));
  opponent_profile_background->setOpacity(80);
  this->addChild(opponent_profile_background, 0);

  is_loading = false;
  start_button = nullptr;
  ready_button = nullptr;
  is_master_img_requesting = false;
  is_opponent_img_requesting = false;
  is_requesting = false;
  start_button = nullptr;
  ready_button = nullptr;
  request_count = 0;
  opponent_status_font = nullptr;
  is_kick = false;

  if(user_info::get().room_info_.is_master) {
    start_button = ui::Button::create();
    start_button->setTouchEnabled(true);
    start_button->ignoreContentAdaptWithSize(false);
    start_button->setContentSize(Size(282.0f, 120.0f));
    start_button->loadTextures("ui/game_start_disable.png", "ui/game_start_disable.png");
    start_button->setEnabled(false);

    start_button->setPosition(Vec2(1140.0f, center_.y-254.0f));

    start_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
	if(type == ui::Widget::TouchEventType::BEGAN) {

          //if(is_master_img_requesting || is_opponent_img_requesting) return false;

	  auto audio = SimpleAudioEngine::getInstance();
	  audio->playEffect("sound/pressing.mp3");

	  auto scaleTo = ScaleTo::create(0.1f, 1.3f);
	  start_button->runAction(scaleTo);

	} else if(type == ui::Widget::TouchEventType::ENDED) {

          //if(is_master_img_requesting || is_opponent_img_requesting) return false;

	  auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	  start_button->runAction(scaleTo2);
	  Json payload = Json::object {
	    { "type", "start_game_req" }
	  };
	  connection::get().send2(payload);

        } else if(type == ui::Widget::TouchEventType::CANCELED) {
	  auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	  start_button->runAction(scaleTo2);
        }
      });
     
    this->addChild(start_button, 0);

    opponent_status_font = Label::createWithTTF("상대를 기다리는중", "fonts/nanumb.ttf", 45);
    opponent_status_font->setPosition(Vec2(center_.x + Director::getInstance()->getVisibleSize().width/4.0f, center_.y));
    opponent_status_font->setColor( Color3B( 255, 255, 255) );
    this->addChild(opponent_status_font, 3);
    auto scaleTo = ScaleTo::create(1.1f, 1.1f);
    opponent_status_font->runAction(scaleTo);
    auto delay = DelayTime::create(0.25f);
    auto scaleTo2 = ScaleTo::create(1.0f, 1.0f);
    auto seq = Sequence::create(scaleTo, delay, scaleTo2, delay->clone(), nullptr);
    opponent_status_font->runAction(RepeatForever::create(seq));

    connection::get().send2(Json::object {
      { "type", "opponent_info_req" }
    });
  
    connection::get().send2(Json::object {
      { "type", "check_ready_opponent_req" }
    });

    create_master_profile("100005347304902", user_info::get().account_info_.get_name(), user_info::get().account_info_.score, user_info::get().account_info_.win_count, user_info::get().account_info_.lose_count, user_info::get().account_info_.ranking);
    create_opponent_profile("100005347304902", user_info::get().account_info_.get_name(), user_info::get().account_info_.score, user_info::get().account_info_.win_count, user_info::get().account_info_.lose_count, user_info::get().account_info_.ranking);

  } else {

    connection::get().send2(Json::object {
      { "type", "master_info_req" }
    });

    ready_button = ui::Button::create();
    ready_button->setTouchEnabled(true);
    ready_button->ignoreContentAdaptWithSize(false);
    ready_button->setContentSize(Size(282.0f, 126.0f));
    ready_button->setEnabled(true);
    ready_button->loadTextures("ui/game_ready.png", "ui/game_ready.png");
    ready_button->setPosition(Vec2(1140.0f, center_.y-254.0f));

    ready_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
	if(type == ui::Widget::TouchEventType::BEGAN) {
	  auto audio = SimpleAudioEngine::getInstance();
	  audio->playEffect("sound/pressing.mp3");

	  auto scaleTo = ScaleTo::create(0.1f, 1.3f);
	  ready_button->runAction(scaleTo);

	} else if(type == ui::Widget::TouchEventType::ENDED) {

	  auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	  ready_button->runAction(scaleTo2);
          ready_button->setEnabled(false);
          ready_button->loadTextures("ui/game_ready_disable.png", "ui/game_ready_disable.png");
	  Json payload = Json::object {
	    { "type", "ready_game_noti" }
	  };
	  connection::get().send2(payload);

          // 5초 안에 방장이 게임을 시작안하면 로딩을 풀어주고 방에서 나갈수 있게 해줌

        } else if(type == ui::Widget::TouchEventType::CANCELED) {
	  auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	  ready_button->runAction(scaleTo2);
        }
      });
     
    this->addChild(ready_button, 0);

    create_opponent_profile("100005347304902", user_info::get().account_info_.get_name(), user_info::get().account_info_.score, user_info::get().account_info_.win_count, user_info::get().account_info_.lose_count, user_info::get().account_info_.ranking);

    auto moveTo = MoveTo::create(0.5f, Vec2(center_.x + (Director::getInstance()->getVisibleSize().width / 4) + 25, center_.y+40));
    opponent_profile_background->runAction(moveTo);
  }

  back_button = ui::Button::create();
  back_button->setTouchEnabled(true);
  back_button->ignoreContentAdaptWithSize(false);
  back_button->setContentSize(Size(128, 128));
  back_button->setScale(0.5f);
  back_button->loadTextures("ui/back2.png", "ui/back2.png");

  auto y = center_.y + _play_screen_y/2 - _offset_y+0;
  back_button->setPosition(Vec2(40, y));

  back_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {

	//if(is_master_img_requesting || is_opponent_img_requesting) return false;
	if(is_kick) return false;

        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");

	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	back_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {

	//if(is_master_img_requesting || is_opponent_img_requesting) return false;
	if(is_kick) return false;

	  Json payload = Json::object {
	    { "type", "leave_room_req" }
	  };
	  connection::get().send2(payload);
	
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	back_button->runAction(scaleTo2);

        //this->scheduleOnce(SEL_SCHEDULE(&multi_room_scene::replace_multi_lobby_scene), 0.2f); 

      } else if(type == ui::Widget::TouchEventType::CANCELED) {

	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	back_button->runAction(scaleTo2);
      }

    });
     
  this->addChild(back_button, 0);

  /*
  auto debug_font = Label::createWithTTF("vs", "fonts/nanumb.ttf", 100);
  debug_font->setPosition(center_);
  debug_font->setColor(Color3B( 255, 0, 0));
  this->addChild(debug_font, 0);
  */

  create_connection_popup();
  create_destroy_popup();
  
  this->scheduleUpdate();

  return true;
}

void multi_room_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }
  
  //CCLOG("update");
}

void multi_room_scene::replace_multi_play_scene() {
  for(auto& kv : requests) {
    auto req = kv.second;
    req->setResponseCallback(nullptr);
  }
  auto multi_play_scene = multi_play_scene::createScene();
  Director::getInstance()->replaceScene(multi_play_scene);
}

void multi_room_scene::replace_multi_lobby_scene() {
  for(auto& kv : requests) {
    auto req = kv.second;
    req->setResponseCallback(nullptr);
  }
  auto multi_lobby_scene = multi_lobby_scene::createScene();
  Director::getInstance()->replaceScene(multi_lobby_scene);
}

void multi_room_scene::handle_payload(float dt) {
    Json payload = connection::get().q.front();
    connection::get().q.pop_front();
    std::string type = payload["type"].string_value();

    if(type == "connection_notify") {
      CCLOG("[debug] 접속 성공");
     

    } else if(type == "disconnection_notify") {
      CCLOG("[debug] 접속 큰킴");
      open_connection_popup();
      
    } else if(type == "update_alive_noti") { 
      CCLOG("[noti] update alive noti");
      connection::get().send2(Json::object {
	  { "type", "update_alive_noti" }
	});
    } else if(type == "start_game_res") {
      // 둘다 다른 씬으로 넘어감
      // 데이터 넘어오니까 먼저 파싱함
      user_info::get().room_info_.stages.clear();

      auto result = payload["result"].bool_value();
      if(result) {

        auto stages = payload["stage_infos"].array_items();
        for(auto _stage : stages) {
          stage game_stage;
          auto img = _stage["img"].string_value();
          game_stage.img = img;

          std::deque<float> points;
          for (auto &k : _stage["points"].array_items()) {
            points.push_back(static_cast<float>(k.int_value()));
          }

          auto size = points.size();
          for(unsigned i=0; i<size;) {
            auto x = points.front();
            points.pop_front();

            auto y = points.front();
            points.pop_front();

            game_stage.hidden_points.push_back(Vec2(x, y));
            i=i+2;
          }
          user_info::get().room_info_.stages.push_back(game_stage);
        }

        // replace scene
        // 5초 후에 시작한다고 3 2 1 액션 주고 씬넘아감
        replace_multi_play_scene();
        //this->scheduleOnce(SEL_SCHEDULE(&multi_room_scene::replace_multi_play_scene), 0.0f);
      } else {

      }

   
    } else if(type == "ready_game_noti") {
      // ready 누르고 나면 무조건 대기중
      auto audio = SimpleAudioEngine::getInstance();
      audio->playEffect("sound/multi_lobby_ready.wav");
      start_button->loadTextures("ui/game_start.png", "ui/game_start.png");
      start_button->setEnabled(true);

    } else if(type == "room_destroy_noti") {
      CCLOG("방장이 나감");
      open_destroy_popup();
      
    } else if(type == "join_opponent_noti") {
      CCLOG("상대측이 들어옴");
      auto audio = SimpleAudioEngine::getInstance();
      audio->playEffect("sound/join_user.mp3");
      if(opponent_status_font) {
	opponent_status_font->setPosition(center_.x + 5000.0f, center_.y);
      }
      std::string name = payload["name"].string_value();
      std::string facebookid = payload["facebookid"].string_value();
      if(facebookid == "") facebookid = "100005347304902";
      auto score = payload["score"].int_value();
      auto win_count = payload["win_count"].int_value();
      auto lose_count = payload["lose_count"].int_value();
      auto ranking = payload["ranking"].int_value();
      create_opponent_profile(facebookid, name, score, win_count, lose_count, ranking);

      user_info::get().account_info_.set_other_name(name);
      user_info::get().account_info_.earn_score = payload["earn_score"].int_value();
      user_info::get().account_info_.lose_score = payload["lose_score"].int_value();

      auto moveTo = MoveTo::create(0.5f, Vec2(center_.x + (Director::getInstance()->getVisibleSize().width / 4) + 25, center_.y+40));
      opponent_profile_background->runAction(moveTo);

    } else if(type == "leave_opponent_noti") {
      CCLOG("상대측이 나감");
      auto audio = SimpleAudioEngine::getInstance();
      audio->playEffect("sound/leave_user.wav");
      if(opponent_status_font) {
	opponent_status_font->setPosition(Vec2(center_.x + Director::getInstance()->getVisibleSize().width/4.0f, center_.y));
      }
      start_button->setEnabled(false);
      start_button->loadTextures("ui/game_start_disable.png", "ui/game_start_disable.png");

      auto moveTo = MoveTo::create(0.4f, Vec2(center_.x + (Director::getInstance()->getVisibleSize().width), center_.y+40));
      opponent_profile_background->runAction(moveTo);

    } else if(type == "master_info_res") {

      auto result = payload["result"].bool_value();
      if(!result) {
	open_destroy_popup();
	return;
      }

      std::string name = payload["name"].string_value();
      std::string facebookid = payload["facebookid"].string_value();
      if(facebookid == "") facebookid = "100005347304902";
      auto score = payload["score"].int_value();
      auto win_count = payload["win_count"].int_value();
      auto lose_count = payload["lose_count"].int_value();
      auto ranking = payload["ranking"].int_value();
      create_master_profile(facebookid, name, score, win_count, lose_count, ranking);
      user_info::get().account_info_.set_other_name(name);
      user_info::get().account_info_.earn_score = payload["earn_score"].int_value();
      user_info::get().account_info_.lose_score = payload["lose_score"].int_value();
      

    } else if(type == "opponent_info_res") {

      auto result = payload["result"].bool_value();
      if(result) {
        CCLOG("상대방 유저가 존재함");
        std::string name = payload["name"].string_value();
        std::string facebookid = payload["facebookid"].string_value();
        if(facebookid == "") facebookid = "100005347304902";
        auto score = payload["score"].int_value();
        auto win_count = payload["win_count"].int_value();
        auto lose_count = payload["lose_count"].int_value();
        auto ranking = payload["ranking"].int_value();
        create_opponent_profile(facebookid, name, score, win_count, lose_count, ranking);
        user_info::get().account_info_.set_other_name(name);
	user_info::get().account_info_.earn_score = payload["earn_score"].int_value();
	user_info::get().account_info_.lose_score = payload["lose_score"].int_value();

        auto moveTo = MoveTo::create(0.5f, Vec2(center_.x + (Director::getInstance()->getVisibleSize().width / 4) + 25, center_.y+40));
        opponent_profile_background->runAction(moveTo);

	if(opponent_status_font) {
	  opponent_status_font->setPosition(center_.x + 5000.0f, center_.y);
	}

      } else {
        CCLOG("상대방 유저가 존재하지 않음");
	if(opponent_status_font) {
	  opponent_status_font->setPosition(Vec2(center_.x + Director::getInstance()->getVisibleSize().width/4.0f, center_.y));
	}
      }

    } else if(type == "kick_opponent_noti") {
      CCLOG("방장한테 쫓겨남");
      if(opponent_status_font) {
	opponent_status_font->setPosition(Vec2(center_.x + Director::getInstance()->getVisibleSize().width/4.0f, center_.y));
      }

      is_kick = true;
      Json payload = Json::object {
	{ "type", "leave_room_req" }
      };
      connection::get().send2(payload);

      destory_noti_font->setString("방에서 킥 당하셨습니다.");
      open_destroy_popup();
      
      //this->scheduleOnce(SEL_SCHEDULE(&multi_room_scene::replace_multi_lobby_scene), 0.2f);

    } else if(type == "update_game_info_noti") {
      user_info::get().account_info_.score = payload["score"].int_value();
      user_info::get().account_info_.win_count = payload["win_count"].int_value();
      user_info::get().account_info_.lose_count = payload["lose_count"].int_value();
      user_info::get().account_info_.ranking = payload["ranking"].int_value();

    } else if(type == "check_ready_opponent_res") {
      auto is_ready = payload["is_ready"].bool_value();
      if(is_ready) {
	start_button->loadTextures("ui/game_start.png", "ui/game_start.png");
	start_button->setEnabled(true);
      }

    } else if(type == "leave_room_res") {
      auto result = payload["result"].bool_value();
      if(result) {

	if(!is_kick) {
	  this->scheduleOnce(SEL_SCHEDULE(&multi_room_scene::replace_multi_lobby_scene), 0.2f); 
	}
      }

    } else {
      CCLOG("[error] handler 없음");
      CCLOG("type: %s", type.c_str());
    }
}

void multi_room_scene::create_connection_popup() {
  auto offset = 5000.0f;
  connection_background_popup = Sprite::create("ui/background_popup.png");
  connection_background_popup->setScale(2.0f);
  connection_background_popup->setPosition(Vec2(center_.x + offset, center_.y));
  this->addChild(connection_background_popup, 1);

  connection_noti_font = Label::createWithTTF("네트워크 불안정 상태로 서버와 접속 끊김.", "fonts/nanumb.ttf", 40);
  connection_noti_font->setPosition(Vec2(center_.x + offset, center_.y));
  connection_noti_font->setColor(Color3B( 110, 110, 110));
  this->addChild(connection_noti_font, 1);

  connection_confirm_button = ui::Button::create();
  connection_confirm_button->setTouchEnabled(true);
  connection_confirm_button->ignoreContentAdaptWithSize(false);
  connection_confirm_button->setContentSize(Size(286.0f, 126.0f));
  connection_confirm_button->loadTextures("ui/confirm_button.png", "ui/confirm_button.png");
  connection_confirm_button->setPosition(Vec2(center_.x + offset, center_.y));

  connection_confirm_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 1.1f);
        connection_confirm_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
        connection_confirm_button->runAction(scaleTo2);
        if(!connection::get().get_is_connected()) {
          connection::get().create("ws://n.05day.com:8080/echo");
          connection::get().connect();
        }
        for(auto& kv : requests) {
          auto req = kv.second;
          req->setResponseCallback(nullptr);
        }
        auto lobby_scene = lobby_scene::createScene();
        Director::getInstance()->replaceScene(lobby_scene);
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 1.0f);
        connection_confirm_button->runAction(scaleTo);
      }
    });
     
  this->addChild(connection_confirm_button, 1);
}

void multi_room_scene::open_connection_popup() {
  connection_background_popup->setPosition(Vec2(center_));
  connection_noti_font->setPosition(Vec2(center_.x, center_.y + 60.0f));
  connection_confirm_button->setPosition(Vec2(center_.x, center_.y - 100.0f));
  if(opponent_status_font) {
    opponent_status_font->setPosition(center_.x + 5000.0f, center_.y);
  }
}

void multi_room_scene::close_connection_popup() {
  auto offset = 5000.0f;
  connection_noti_font->setPosition(Vec2(center_.x + offset, center_.y));
  connection_noti_font->setPosition(Vec2(center_.x + offset, center_.y + 60.0f));
  connection_confirm_button->setPosition(Vec2(center_.x + offset, center_.y - 100.0f));
}

void multi_room_scene::create_destroy_popup() {
  auto offset = 5000.0f;
  destory_background_popup = Sprite::create("ui/background_popup.png");
  destory_background_popup->setScale(2.0f);
  destory_background_popup->setPosition(Vec2(center_.x + offset, center_.y));
  this->addChild(destory_background_popup, 1);

  destory_noti_font = Label::createWithTTF("방장이 나가셨습니다.", "fonts/nanumb.ttf", 40);
  destory_noti_font->setPosition(Vec2(center_.x + offset, center_.y));
  destory_noti_font->setColor(Color3B( 110, 110, 110));
  this->addChild(destory_noti_font, 1);

  destory_confirm_button = ui::Button::create();
  destory_confirm_button->setTouchEnabled(true);
  destory_confirm_button->ignoreContentAdaptWithSize(false);
  destory_confirm_button->setContentSize(Size(286.0f, 126.0f));
  destory_confirm_button->loadTextures("ui/confirm2_button.png", "ui/confirm2_button.png");
  destory_confirm_button->setPosition(Vec2(center_.x + offset, center_.y));

  destory_confirm_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 1.1f);
        destory_confirm_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {

        Json payload = Json::object {
          { "type", "leave_room_req" }
        };
        connection::get().send2(payload);

	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
        destory_confirm_button->runAction(scaleTo2);
        this->scheduleOnce(SEL_SCHEDULE(&multi_room_scene::replace_multi_lobby_scene), 0.2f);

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 1.0f);
        destory_confirm_button->runAction(scaleTo);
      }
    });
     
  this->addChild(destory_confirm_button, 1);
}

void multi_room_scene::open_destroy_popup() {
  destory_background_popup->setPosition(Vec2(center_));
  destory_noti_font->setPosition(Vec2(center_.x, center_.y + 60.0f));
  destory_confirm_button->setPosition(Vec2(center_.x, center_.y - 100.0f));
}

void multi_room_scene::close_destroy_popup() {
  auto offset = 5000.0f;
  destory_background_popup->setPosition(Vec2(center_.x + offset, center_.y));
  destory_noti_font->setPosition(Vec2(center_.x + offset, center_.y + 60.0f));
  destory_confirm_button->setPosition(Vec2(center_.x + offset, center_.y - 100.0f));
}

void multi_room_scene::create_master_profile(std::string facebookid, std::string name, int score, int win_count, int lose_count, int ranking) {
  if(master_profile.name_font)         master_profile_background->removeChild(master_profile.name_font, true);
  if(master_profile.score_font)        master_profile_background->removeChild(master_profile.score_font, true);
  if(master_profile.score_front_font)  master_profile_background->removeChild(master_profile.score_front_font, true);
  if(master_profile.win_count_font)    master_profile_background->removeChild(master_profile.win_count_font, true);
  if(master_profile.win_font)          master_profile_background->removeChild(master_profile.win_font, true);
  if(master_profile.lose_count_font)    master_profile_background->removeChild(master_profile.lose_count_font, true);
  if(master_profile.lose_font)          master_profile_background->removeChild(master_profile.lose_font, true);
  if(master_profile.ranking_font)          master_profile_background->removeChild(master_profile.ranking_font, true);
  if(master_profile.ranking_front_font)    master_profile_background->removeChild(master_profile.ranking_front_font, true);
  
 
  start_img_req();
  
  auto font_size = 36;
  auto start_x = 140;

  // id
  master_profile.name_font = Label::createWithTTF(name.c_str(), "fonts/nanumb.ttf", font_size+2);
  master_profile.name_font->setPosition(Vec2(140, 60));
  master_profile.name_font->setColor( Color3B( 255, 255, 255) );
  master_profile_background->addChild(master_profile.name_font, 0);

  
  start_x = 280;
  // score
  master_profile.score_font = Label::createWithTTF(ccsf2("%d", score), "fonts/nanumb.ttf", font_size);
  master_profile.score_font->setPosition(Vec2(start_x, 250));
  master_profile.score_font->setColor( Color3B( 255, 255, 255) );
  master_profile.score_font->setAnchorPoint(ccp(0,0.5f)); 
  master_profile_background->addChild(master_profile.score_font, 0);

  master_profile.score_front_font = Label::createWithTTF("점", "fonts/nanumb.ttf", font_size);
  master_profile.score_front_font->setPosition(Vec2(master_profile.score_font->getPosition().x + (master_profile.score_font->getContentSize().width / 2.0f) + 55.0f, 250));
  master_profile.score_front_font->setColor( Color3B( 200, 200, 200) );
  master_profile.score_front_font->setAnchorPoint(ccp(0,0.5f)); 
  master_profile_background->addChild(master_profile.score_front_font, 0);

  
  // win
  auto font_y = 250 - 60;
  master_profile.win_count_font = Label::createWithTTF(ccsf2("%d", win_count), "fonts/nanumb.ttf", font_size);
  master_profile.win_count_font->setPosition(Vec2(start_x, font_y));
  master_profile.win_count_font->setColor( Color3B( 255, 255, 255) );
  master_profile.win_count_font->setAnchorPoint(ccp(0,0.5f)); 
  master_profile_background->addChild(master_profile.win_count_font, 0);

  auto win_font_x = master_profile.win_count_font->getPosition().x + (master_profile.win_count_font->getContentSize().width / 2.0f) + 20.0f;
  master_profile.win_font = Label::createWithTTF(" 승", "fonts/nanumb.ttf", font_size-2);
  master_profile.win_font->setPosition(Vec2(win_font_x, font_y));
  master_profile.win_font->setColor( Color3B( 200, 200, 200) );
  master_profile.win_font->setAnchorPoint(ccp(0,0.5f)); 
  master_profile_background->addChild(master_profile.win_font, 0);

  // lose
  master_profile.lose_count_font = Label::createWithTTF(ccsf2("%d", lose_count), "fonts/nanumb.ttf", font_size);
  master_profile.lose_count_font->setPosition(Vec2(win_font_x + 60, font_y));
  master_profile.lose_count_font->setColor( Color3B( 255, 255, 255) );
  master_profile.lose_count_font->setAnchorPoint(ccp(0,0.5f)); 
  master_profile_background->addChild(master_profile.lose_count_font, 0);

  auto lose_font_x = master_profile.lose_count_font->getPosition().x + (master_profile.lose_count_font->getContentSize().width / 2.0f) + 20.0f;
  master_profile.lose_font = Label::createWithTTF(" 패", "fonts/nanumb.ttf", font_size-2);
  master_profile.lose_font->setPosition(Vec2(lose_font_x, font_y));
  master_profile.lose_font->setColor( Color3B( 200, 200, 200) );
  master_profile.lose_font->setAnchorPoint(ccp(0,0.5f)); 
  master_profile_background->addChild(master_profile.lose_font, 0);

  
  // ranking
  master_profile.ranking_font = Label::createWithTTF(ccsf2("%d", ranking), "fonts/nanumb.ttf", font_size);
  master_profile.ranking_font->setPosition(Vec2(start_x, font_y-60));
  master_profile.ranking_font->setColor( Color3B( 255, 255, 255) );
  master_profile.ranking_font->setAnchorPoint(ccp(0,0.5f)); 
  master_profile_background->addChild(master_profile.ranking_font, 0);

  master_profile.ranking_front_font = Label::createWithTTF(" 위", "fonts/nanumb.ttf", font_size-2);
  master_profile.ranking_front_font->setPosition(Vec2(master_profile.ranking_font->getPosition().x + (master_profile.ranking_font->getContentSize().width / 2.0f) + 25.0f, master_profile.ranking_font->getPosition().y));
  master_profile.ranking_front_font->setColor( Color3B( 200, 200, 200) );
  master_profile.ranking_front_font->setAnchorPoint(ccp(0,0.5f)); 
  master_profile_background->addChild(master_profile.ranking_front_font, 0);

}

void multi_room_scene::create_opponent_profile(std::string facebookid, std::string name, int score, int win_count, int lose_count, int ranking) {
  
  if(opponent_profile.name_font)         opponent_profile_background->removeChild(opponent_profile.name_font, true);
  if(opponent_profile.score_font)        opponent_profile_background->removeChild(opponent_profile.score_font, true);
  if(opponent_profile.score_front_font)  opponent_profile_background->removeChild(opponent_profile.score_front_font, true);
  if(opponent_profile.win_count_font)    opponent_profile_background->removeChild(opponent_profile.win_count_font, true);
  if(opponent_profile.win_font)          opponent_profile_background->removeChild(opponent_profile.win_font, true);
  if(opponent_profile.lose_count_font)    opponent_profile_background->removeChild(opponent_profile.lose_count_font, true);
  if(opponent_profile.lose_font)          opponent_profile_background->removeChild(opponent_profile.lose_font, true);
  if(opponent_profile.ranking_font)          opponent_profile_background->removeChild(opponent_profile.ranking_font, true);
  if(opponent_profile.ranking_front_font)    opponent_profile_background->removeChild(opponent_profile.ranking_front_font, true);
  
 
  start_img_req(facebookid, false);
  
  auto font_size = 36;
  auto start_x = 140;

  // id
  opponent_profile.name_font = Label::createWithTTF(name.c_str(), "fonts/nanumb.ttf", font_size+2);
  opponent_profile.name_font->setPosition(Vec2(140, 60));
  opponent_profile.name_font->setColor( Color3B( 255, 255, 255) );
  opponent_profile_background->addChild(opponent_profile.name_font, 0);

  
  start_x = 280;
  // score
  opponent_profile.score_font = Label::createWithTTF(ccsf2("%d", score), "fonts/nanumb.ttf", font_size);
  opponent_profile.score_font->setPosition(Vec2(start_x, 250));
  opponent_profile.score_font->setColor( Color3B( 255, 255, 255) );
  opponent_profile.score_font->setAnchorPoint(ccp(0,0.5f)); 
  opponent_profile_background->addChild(opponent_profile.score_font, 0);

  opponent_profile.score_front_font = Label::createWithTTF("점", "fonts/nanumb.ttf", font_size);
  opponent_profile.score_front_font->setPosition(Vec2(opponent_profile.score_font->getPosition().x + (opponent_profile.score_font->getContentSize().width / 2.0f) + 55.0f, 250));
  opponent_profile.score_front_font->setColor( Color3B( 200, 200, 200) );
  opponent_profile.score_front_font->setAnchorPoint(ccp(0,0.5f)); 
  opponent_profile_background->addChild(opponent_profile.score_front_font, 0);

  
  // win
  auto font_y = 250 - 60;
  opponent_profile.win_count_font = Label::createWithTTF(ccsf2("%d", win_count), "fonts/nanumb.ttf", font_size);
  opponent_profile.win_count_font->setPosition(Vec2(start_x, font_y));
  opponent_profile.win_count_font->setColor( Color3B( 255, 255, 255) );
  opponent_profile.win_count_font->setAnchorPoint(ccp(0,0.5f)); 
  opponent_profile_background->addChild(opponent_profile.win_count_font, 0);

  auto win_font_x = opponent_profile.win_count_font->getPosition().x + (opponent_profile.win_count_font->getContentSize().width / 2.0f) + 20.0f;
  opponent_profile.win_font = Label::createWithTTF(" 승", "fonts/nanumb.ttf", font_size-2);
  opponent_profile.win_font->setPosition(Vec2(win_font_x, font_y));
  opponent_profile.win_font->setColor( Color3B( 200, 200, 200) );
  opponent_profile.win_font->setAnchorPoint(ccp(0,0.5f)); 
  opponent_profile_background->addChild(opponent_profile.win_font, 0);

  // lose
  opponent_profile.lose_count_font = Label::createWithTTF(ccsf2("%d", lose_count), "fonts/nanumb.ttf", font_size);
  opponent_profile.lose_count_font->setPosition(Vec2(win_font_x + 60, font_y));
  opponent_profile.lose_count_font->setColor( Color3B( 255, 255, 255) );
  opponent_profile.lose_count_font->setAnchorPoint(ccp(0,0.5f)); 
  opponent_profile_background->addChild(opponent_profile.lose_count_font, 0);

  auto lose_font_x = opponent_profile.lose_count_font->getPosition().x + (opponent_profile.lose_count_font->getContentSize().width / 2.0f) + 20.0f;
  opponent_profile.lose_font = Label::createWithTTF(" 패", "fonts/nanumb.ttf", font_size-2);
  opponent_profile.lose_font->setPosition(Vec2(lose_font_x, font_y));
  opponent_profile.lose_font->setColor( Color3B( 200, 200, 200) );
  opponent_profile.lose_font->setAnchorPoint(ccp(0,0.5f)); 
  opponent_profile_background->addChild(opponent_profile.lose_font, 0);

  
  // ranking
  opponent_profile.ranking_font = Label::createWithTTF(ccsf2("%d", ranking), "fonts/nanumb.ttf", font_size);
  opponent_profile.ranking_font->setPosition(Vec2(start_x, font_y-60));
  opponent_profile.ranking_font->setColor( Color3B( 255, 255, 255) );
  opponent_profile.ranking_font->setAnchorPoint(ccp(0,0.5f)); 
  opponent_profile_background->addChild(opponent_profile.ranking_font, 0);

  opponent_profile.ranking_front_font = Label::createWithTTF(" 위", "fonts/nanumb.ttf", font_size-2);
  opponent_profile.ranking_front_font->setPosition(Vec2(opponent_profile.ranking_font->getPosition().x + (opponent_profile.ranking_font->getContentSize().width / 2.0f) + 25.0f, opponent_profile.ranking_font->getPosition().y));
  opponent_profile.ranking_front_font->setColor( Color3B( 200, 200, 200) );
  opponent_profile.ranking_front_font->setAnchorPoint(ccp(0,0.5f)); 
  opponent_profile_background->addChild(opponent_profile.ranking_front_font, 0);

  // kick
  if(user_info::get().room_info_.is_master) {
    kick_button = ui::Button::create();
    kick_button->setTouchEnabled(true);
    kick_button->ignoreContentAdaptWithSize(false);
    kick_button->setContentSize(Size(250, 250));
    kick_button->setScale(0.5f);
    kick_button->loadTextures("ui/kick2.png", "ui/kick2.png");

    kick_button->setPosition(Vec2(opponent_profile_background->getContentSize().width - 68, 50));

    kick_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
        if(type == ui::Widget::TouchEventType::BEGAN) {

          if(is_master_img_requesting || is_opponent_img_requesting) return false;
          auto audio = SimpleAudioEngine::getInstance();
          audio->playEffect("sound/pressing.mp3");
          auto scaleTo = ScaleTo::create(0.1f, 0.7f);
          kick_button->runAction(scaleTo);

        } else if(type == ui::Widget::TouchEventType::ENDED) {

          if(is_master_img_requesting || is_opponent_img_requesting) return false;
          start_button->setEnabled(false);
          start_button->loadTextures("ui/game_start_disable.png", "ui/game_start_disable.png");

          auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
          kick_button->runAction(scaleTo2);
          connection::get().send2(Json::object {
              { "type", "kick_opponent_noti" }
            });

        } else if(type == ui::Widget::TouchEventType::CANCELED) {
          auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
          kick_button->runAction(scaleTo2);
        }
      });

    opponent_profile_background->addChild(kick_button, 0);
  }
}

void multi_room_scene::start_img_req(std::string id, bool is_master) {
  if (is_master) {
    is_master_img_requesting = true;
  } else {
    is_opponent_img_requesting = true;
  }
    
  cocos2d::network::HttpRequest* request = new (std::nothrow) cocos2d::network::HttpRequest();

  requests[request_count] = request;

  std::string url = "https://graph.facebook.com/" + id + "/picture?height=200&width=200";
  request->setUrl(url.c_str());
  request->setRequestType(cocos2d::network::HttpRequest::Type::GET);

  if (is_master) {
    request->setResponseCallback(CC_CALLBACK_2(multi_room_scene::on_request_master_img_completed, this));
  } else {
    request->setResponseCallback(CC_CALLBACK_2(multi_room_scene::on_request_opponent_img_completed, this));
  }

  request->setTag(ccsf2("%d", request_count));
  request_count++;

  cocos2d::network::HttpClient::getInstance()->send(request);
  request->release();
}

void multi_room_scene::on_request_master_img_completed(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response) {

  if(!response) {
    is_master_img_requesting = false;
    return;
  }

  if(!response->isSucceed()) {
    is_master_img_requesting = false;
    return;
  }

  std::vector<char>* buffer = response->getResponseData();

  Image* image = new Image ();
  image->initWithImageData ( reinterpret_cast<const unsigned char*>(&(buffer->front())), buffer->size());


  master_profile.texture.initWithImage(image);
  master_profile.img = Sprite::createWithTexture(&master_profile.texture);
  master_profile.img->setPosition(Vec2(140, 200));
  master_profile_background->addChild(master_profile.img);

  if(image) delete image;

  auto str_tag = response->getHttpRequest()->getTag();
  auto tag = std::atoi(str_tag);
  requests.erase(tag);
  is_master_img_requesting = false;

  /*
  if(ready_button) {
    ready_button->setEnabled(true);
    ready_button->loadTextures("ui/game_ready.png", "ui/game_ready.png");
  }
  */

}

void multi_room_scene::on_request_opponent_img_completed(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response) {

  if(!response) {
    is_opponent_img_requesting = false;
    return;
  }

  if(!response->isSucceed()) {
    is_opponent_img_requesting = false;
    return;
  }

  std::vector<char>* buffer = response->getResponseData();

  Image* image = new Image ();
  image->initWithImageData ( reinterpret_cast<const unsigned char*>(&(buffer->front())), buffer->size());

  opponent_profile.texture.initWithImage(image);
  opponent_profile.img = Sprite::createWithTexture(&opponent_profile.texture);
  opponent_profile.img->setPosition(Vec2(140, 200));
  opponent_profile_background->addChild(opponent_profile.img);

  if(image) delete image; 

  auto str_tag = response->getHttpRequest()->getTag();
  auto tag = std::atoi(str_tag);
  requests.erase(tag);
  is_opponent_img_requesting = false;
}
