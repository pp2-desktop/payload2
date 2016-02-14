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
    
  //
  /* 
  auto background = Sprite::create("background/lobby_scene.png");
  background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background, 0);
  */
  auto background = Sprite::create("background/vs_play_scene.png");
  background->setPosition(Vec2(center_.x, center_.y));
  this->addChild(background, 0);

  is_loading = false;
  start_button = nullptr;
  ready_button = nullptr;
  is_master_img_requesting = false;
  is_opponent_img_requesting = false;

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
	  auto audio = SimpleAudioEngine::getInstance();
	  audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);

	  auto scaleTo = ScaleTo::create(0.1f, 1.3f);
	  start_button->runAction(scaleTo);

	} else if(type == ui::Widget::TouchEventType::ENDED) {
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

    create_master_profile();

  } else {
    ready_button = ui::Button::create();
    ready_button->setTouchEnabled(true);
    ready_button->ignoreContentAdaptWithSize(false);
    ready_button->setContentSize(Size(282.0f, 126.0f));
    ready_button->loadTextures("ui/game_ready.png", "ui/game_ready.png");

    ready_button->setPosition(Vec2(1140.0f, center_.y-254.0f));

    ready_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
	if(type == ui::Widget::TouchEventType::BEGAN) {
	  auto audio = SimpleAudioEngine::getInstance();
	  audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);

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

    create_opponent_profile();
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

	if(is_master_img_requesting || is_opponent_img_requesting) return;

        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);

	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	back_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {

	  Json payload = Json::object {
	    { "type", "leave_room_req" }
	  };
	  connection::get().send2(payload);
	
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	back_button->runAction(scaleTo2);
        this->scheduleOnce(SEL_SCHEDULE(&multi_room_scene::replace_multi_lobby_scene), 0.2f); 

      } else if(type == ui::Widget::TouchEventType::CANCELED) {

	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	back_button->runAction(scaleTo2);
      }

    });
     
  this->addChild(back_button, 0);

  auto debug_font = Label::createWithTTF("vs", "fonts/nanumb.ttf", 100);
  debug_font->setPosition(center_);
  debug_font->setColor(Color3B( 255, 215, 0));

  this->addChild(debug_font, 0);

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
  auto multi_play_scene = multi_play_scene::createScene();
  Director::getInstance()->replaceScene(multi_play_scene);
}

void multi_room_scene::replace_multi_lobby_scene() {
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
      start_button->loadTextures("ui/game_start.png", "ui/game_start.png");
      start_button->setEnabled(true);

    } else if(type == "room_destroy_noti") {
      CCLOG("방장이 나감");
      open_destroy_popup();
      
    } else if(type == "join_opponent_noti") {
      CCLOG("상대측이 들어옴");

    } else if(type == "leave_opponent_noti") {
      CCLOG("상대측이 나감");
      start_button->setEnabled(false);
      start_button->loadTextures("ui/game_start_disable.png", "ui/game_start_disable.png");
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

  connection_noti_font = Label::createWithTTF("네트워크 불안정 상태로 서버와 접속 끊김", "fonts/nanumb.ttf", 40);
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
     
  this->addChild(connection_confirm_button, 1);
}

void multi_room_scene::open_connection_popup() {
  connection_background_popup->setPosition(Vec2(center_));
  connection_noti_font->setPosition(Vec2(center_.x, center_.y + 60.0f));
  connection_confirm_button->setPosition(Vec2(center_.x, center_.y - 100.0f));
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

  destory_noti_font = Label::createWithTTF("방장이 나가셨습니다", "fonts/nanumb.ttf", 40);
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

void multi_room_scene::create_master_profile() {
  start_img_req();
  // id
  auto name_font = Label::createWithTTF(user_info::get().account_info_.get_name().c_str(), "fonts/nanumb.ttf", 40);
  name_font->setPosition(Vec2(200, center_.y+80-150));
  name_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(name_font, 0);

  auto font_size = 36;
  // score
  auto score_font = Label::createWithTTF(ccsf2("%d", user_info::get().account_info_.score), "fonts/nanumb.ttf", font_size+2);
  score_font->setPosition(Vec2(350, center_.y+150));
  score_font->setColor( Color3B( 10, 0, 10) );
  score_font->setAnchorPoint(ccp(0,0.5f)); 
  this->addChild(score_font, 0);

  auto score_front_font = Label::createWithTTF("점", "fonts/nanumb.ttf", font_size);
  score_front_font->setPosition(Vec2(score_font->getPosition().x + (score_font->getContentSize().width / 2.0f) + 55.0f, center_.y+150));
  score_front_font->setColor( Color3B( 40, 40, 40) );
  score_front_font->setAnchorPoint(ccp(0,0.5f)); 
  this->addChild(score_front_font, 0);

  // win
  auto font_y = center_.y+100;
  auto win_count_font = Label::createWithTTF(ccsf2("%d", user_info::get().account_info_.win_count), "fonts/nanumb.ttf", font_size);
  win_count_font->setPosition(Vec2(350, font_y));
  win_count_font->setColor( Color3B( 0, 0, 255) );
  win_count_font->setAnchorPoint(ccp(0,0.5f)); 
  this->addChild(win_count_font, 0);

  auto win_font_x = win_count_font->getPosition().x + (win_count_font->getContentSize().width / 2.0f) + 20.0f;
  auto win_font = Label::createWithTTF(" 승", "fonts/nanumb.ttf", font_size-2);
  win_font->setPosition(Vec2(win_font_x, font_y));
  win_font->setColor( Color3B( 10, 0, 10) );
  win_font->setAnchorPoint(ccp(0,0.5f)); 
  this->addChild(win_font, 0);

  // lose
  auto lose_count_font = Label::createWithTTF(ccsf2("%d", user_info::get().account_info_.lose_count), "fonts/nanumb.ttf", font_size);
  lose_count_font->setPosition(Vec2(win_font_x + 60, font_y));
  lose_count_font->setColor( Color3B( 255, 0, 0) );
  lose_count_font->setAnchorPoint(ccp(0,0.5f)); 
  this->addChild(lose_count_font, 0);

  auto lose_font_x = lose_count_font->getPosition().x + (lose_count_font->getContentSize().width / 2.0f) + 20.0f;
  auto lose_font = Label::createWithTTF(" 패", "fonts/nanumb.ttf", font_size-2);
  lose_font->setPosition(Vec2(lose_font_x, font_y));
  lose_font->setColor( Color3B( 10, 0, 10) );
  lose_font->setAnchorPoint(ccp(0,0.5f)); 
  this->addChild(lose_font, 0);

  // ranking
  auto ranking_font = Label::createWithTTF(ccsf2("%d", user_info::get().account_info_.ranking), "fonts/nanumb.ttf", font_size);
  ranking_font->setPosition(Vec2(350, font_y-50));
  ranking_font->setColor( Color3B( 165, 42, 42) );
  ranking_font->setAnchorPoint(ccp(0,0.5f)); 
  this->addChild(ranking_font, 0);

  auto ranking_front_font = Label::createWithTTF("등", "fonts/nanumb.ttf", font_size-2);
  ranking_front_font->setPosition(Vec2(ranking_font->getPosition().x + (ranking_font->getContentSize().width / 2.0f) + 25.0f, ranking_font->getPosition().y));
  ranking_front_font->setColor( Color3B( 40, 40, 40) );
  ranking_front_font->setAnchorPoint(ccp(0,0.5f)); 
  this->addChild(ranking_front_font, 0);
}

void multi_room_scene::create_opponent_profile() {
  start_img_req(false);
}

void multi_room_scene::start_img_req(std::string id, bool is_master) {
  if(is_master) is_master_img_requesting = true;
  else is_opponent_img_requesting = true;

  cocos2d::network::HttpRequest* request = new (std::nothrow) cocos2d::network::HttpRequest();
  string url = "https://graph.facebook.com/"+id+"/picture?height=200&width=200";
  request->setUrl(url.c_str());
  request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
  request->setResponseCallback(CC_CALLBACK_2(multi_room_scene::on_request_master_img_completed, this));
  request->setTag("GetImage");
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
  this->addChild(master_profile.img);
  master_profile.img->setPosition(Vec2(200, center_.y+80));

  if(image) delete image;
  is_master_img_requesting = false;
}

void multi_room_scene::on_request_opponent_img_completed(cocos2d::network::HttpClient* sender, cocos2d::network::HttpResponse* response) {
  if(!response) {
    is_opponent_img_requesting = true;
    return;
  }

  if(!response->isSucceed()) {
    is_opponent_img_requesting = true;
    return;
  }

  std::vector<char>* buffer = response->getResponseData();

  Image* image = new Image ();
  image->initWithImageData ( reinterpret_cast<const unsigned char*>(&(buffer->front())), buffer->size());

  opponent_profile.texture.initWithImage(image);
  opponent_profile.img = Sprite::createWithTexture(&opponent_profile.texture);
  this->addChild(opponent_profile.img);
  opponent_profile.img->setPosition(Vec2(center_.x+200 + (Director::getInstance()->getVisibleSize().width/2.0f), center_.y+80));

  if(image) delete image; 
  is_opponent_img_requesting = true;
}
