#include "ranking_scene.hpp"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "lobby_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"
#include "json11.hpp"
#include "single_play_info.hpp"

using namespace ui;
using namespace CocosDenshion;
using namespace json11;

Scene* ranking_scene::createScene() {

  auto scene = Scene::create();
  auto layer = ranking_scene::create();

  scene->addChild(layer);

  return scene;
}

bool ranking_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }

  auto audio = SimpleAudioEngine::getInstance();
  audio->playBackgroundMusic("sound/bg1.mp3", true);
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);
    

  auto bg = Sprite::create("background/ranking_scene.jpg");
  bg->setPosition(Vec2(center_.x, center_.y));
  //bg->setOpacity(125);
  this->addChild(bg, 0);

  /*
  for(auto i=0; i<100; i++) {
    ranking_info ri;
    ri.name = "guest534501";
    ri.score = 1200;
    ri.win_count = 125;
    ri.lose_count = 224;
    ranking_infos_.push_back(ri);
  }
  */

  create_ui_top();
  create_connection_popup();
  create_loading_status_font();
  //create_ui_ranking_info();


  connection::get().send2(Json::object {
      { "type", "get_ranking_req" }
    });


  if(user_info::get().account_info_.get_uid() != "") {
    connection::get().send2(Json::object {
	{ "type", "update_game_info_noti" },
	{ "uid", user_info::get().account_info_.get_uid() }
      });
  }
  

  this->scheduleUpdate();

  return true;
}

void ranking_scene::create_ui_top() {

  auto ui_top_bg = Sprite::create("ui/top_multi_lobby.png");
  auto y = center_.y + _play_screen_y/2 - _offset_y+0;
  ui_top_bg->setPosition(Vec2(center_.x, center_.y + _play_screen_y/2 - _offset_y+0));
  this->addChild(ui_top_bg, 0);

  back_button = ui::Button::create();
  back_button->setTouchEnabled(true);
  back_button->ignoreContentAdaptWithSize(false);
  back_button->setContentSize(Size(128, 128));
  back_button->setScale(0.5f);
  back_button->loadTextures("ui/back2.png", "ui/back2.png");

  back_button->setPosition(Vec2(40, y));

  back_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {

        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);
	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	back_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	back_button->runAction(scaleTo2);
        this->scheduleOnce(SEL_SCHEDULE(&ranking_scene::replace_lobby_scene), 0.2f); 
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	back_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(back_button, 0);

}

void ranking_scene::create_ui_game_info() {

  auto font_x = 280;
  auto font_y = center_.y + _play_screen_y/2 - _offset_y+0;
  font_y = font_y + 1;
  auto font_size = 30;

  auto ui_offset_x = 80;
  // name
  auto name_font = Label::createWithTTF(user_info::get().account_info_.get_name().c_str(), "fonts/nanumb.ttf", font_size);
  name_font->setPosition(Vec2(ui_offset_x + (name_font->getContentSize().width / 2.0f) + 45.0f, font_y));
  name_font->setColor( Color3B( 0, 0, 0) );
  this->addChild(name_font, 0);

  auto len = user_info::get().account_info_.get_name().size();

  //auto end_name_x = ui_offset_x + (len * 10) + 120 + 40;

  auto end_name_x = ui_offset_x + name_font->getPosition().x + (name_font->getContentSize().width/2.0f);

  auto score_font = Label::createWithTTF(ccsf2("%d 점", user_info::get().account_info_.score), "fonts/nanumb.ttf", font_size);
  score_font->setPosition(Vec2(end_name_x + 40, font_y));
  score_font->setColor( Color3B( 165, 42, 42) );
  this->addChild(score_font, 0);

  // win
  auto win_count_font = Label::createWithTTF(ccsf2("%d", user_info::get().account_info_.win_count), "fonts/nanumb.ttf", font_size);
  win_count_font->setPosition(Vec2(end_name_x + 70 + 100, font_y));
  win_count_font->setColor( Color3B( 0, 0, 255) );
  this->addChild(win_count_font, 0);

  auto win_font_x = win_count_font->getPosition().x + (win_count_font->getContentSize().width / 2.0f) + 20.0f;
  auto win_font = Label::createWithTTF(" 승", "fonts/nanumb.ttf", font_size);
  win_font->setPosition(Vec2(win_font_x, font_y));
  win_font->setColor( Color3B( 10, 0, 10) );
  this->addChild(win_font, 0);

  // lose
  auto lose_count_font = Label::createWithTTF(ccsf2("%d", user_info::get().account_info_.lose_count), "fonts/nanumb.ttf", font_size);
  lose_count_font->setPosition(Vec2(win_font_x + 60, font_y));
  lose_count_font->setColor( Color3B( 255, 0, 0) );
  this->addChild(lose_count_font, 0);

  auto lose_font_x = lose_count_font->getPosition().x + (lose_count_font->getContentSize().width / 2.0f) + 20.0f;
  auto lose_font = Label::createWithTTF(" 패", "fonts/nanumb.ttf", font_size);
  lose_font->setPosition(Vec2(lose_font_x, font_y));
  lose_font->setColor( Color3B( 10, 0, 10) );
  this->addChild(lose_font, 0);

  // ranking
 auto ranking_font = Label::createWithTTF(ccsf2("%d 위", user_info::get().account_info_.ranking), "fonts/nanumb.ttf", font_size);
  ranking_font->setPosition(Vec2(lose_font_x + 110, font_y));
  ranking_font->setColor( Color3B( 50, 200, 50) );
  this->addChild(ranking_font, 0);
  
}

void ranking_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }

}

void ranking_scene::replace_lobby_scene() {
  auto lobby_scene = lobby_scene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.0f, lobby_scene, Color3B(0,255,255)));
}

void ranking_scene::handle_payload(float dt) {
    Json payload = connection::get().q.front();
    connection::get().q.pop_front();
    std::string type = payload["type"].string_value();

    if(type == "connection_notify") {
      CCLOG("[debug] 접속 성공");

    } else if(type == "disconnection_notify") { 
      CCLOG("[debug] 접속 큰킴");
      open_connection_popup();

    } else if(type == "update_alive_noti") { 
      connection::get().send2(Json::object {
	  { "type", "update_alive_noti" }
	});
    } else if(type == "get_ranking_res") {
      loading_status_font->setPosition(Vec2(center_.x + 5000.0f, center_.y));

      auto ranking_infos = payload["ranking_infos"].array_items();
      for(auto& ri : ranking_infos) {

	ranking_info tmp;
	tmp.name = ri["name"].string_value();
	tmp.score = ri["score"].int_value();
	tmp.win_count = ri["win_count"].int_value();
	tmp.lose_count = ri["lose_count"].int_value();
	ranking_infos_.push_back(tmp);
      }

      create_ui_ranking_info();


    } else if(type == "update_game_info_noti") {

      CCLOG("111111111111!");
      user_info::get().account_info_.score = payload["score"].int_value();
      user_info::get().account_info_.win_count = payload["win_count"].int_value();
      user_info::get().account_info_.lose_count = payload["lose_count"].int_value();
      user_info::get().account_info_.ranking = payload["ranking"].int_value();

      create_ui_game_info();
      
    } else {
      CCLOG("[error] handler 없음");
    }
}

void ranking_scene::create_ui_ranking_info() {
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();  

  auto scroll_frame_width = 740 + 200; 
  auto scroll_frame_height = 650; 

  Size scollFrameSize = Size(scroll_frame_width, scroll_frame_height);

  scrollView = cocos2d::ui::ScrollView::create();
  scrollView->setContentSize(scollFrameSize);
  scrollView->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
  scrollView->setBackGroundColor(Color3B(50, 200, 50));
  scrollView->setOpacity(125);

  // 
  auto cheight = ((visibleSize.height - scollFrameSize.height) / 2) - 30.0f;
  scrollView->setPosition(Point(visibleSize.width/4.0f - 140.0f, cheight));
  scrollView->setDirection(cocos2d::ui::ScrollView::Direction::VERTICAL);
  scrollView->setBounceEnabled(true);
  scrollView->setTouchEnabled(true);

  const auto margin = 10.0f;

  auto ranking_bar_height = 69.0f;
  auto containerSize = Size(scollFrameSize.width, (ranking_bar_height+margin) * ranking_infos_.size() + (ranking_bar_height / 2.0f) - margin);

  if(containerSize.height < scroll_frame_height) {
    containerSize.height = scroll_frame_height;
  }

  scrollView->setInnerContainerSize(containerSize);
  auto y = containerSize.height - (ranking_bar_height / 2.0f) - margin;
  for(auto i=0; i<ranking_infos_.size(); i++) {

    auto index = i + 1;

    // 룸 백그라운드(sprite)
    auto tmp = Sprite::create("ui/ranking_bar.png");
    tmp->setPosition(Vec2(scroll_frame_width/2.0f, y));
    tmp->setOpacity(180);
    scrollView->addChild(tmp, 0);

    auto ranking_font = Label::createWithTTF(ccsf2("%d.", index), "fonts/nanumb.ttf", 30);
    ranking_font->setPosition(Vec2(75,  y));
    ranking_font->setColor(Color3B( 125, 125, 125));
    scrollView->addChild(ranking_font, 0);


    auto name_font = Label::createWithTTF(ranking_infos_[i].name.c_str(), "fonts/nanumb.ttf", 30);
    name_font->setPosition(Vec2(210,  y));
    name_font->setColor(Color3B( 0, 0, 0));
    //name_font->setAnchorPoint(ccp(0,0.5f));
    scrollView->addChild(name_font, 0);


    auto score_font = Label::createWithTTF(ccsf2("%d 점", ranking_infos_[i].score), "fonts/nanumb.ttf", 30);
    score_font->setPosition(Vec2(500,  y));
    score_font->setColor(Color3B( 165, 42, 42));
    scrollView->addChild(score_font, 0);

    // win
    auto win_count_font = Label::createWithTTF(ccsf2("%d", ranking_infos_[i].win_count), "fonts/nanumb.ttf", 30);
    win_count_font->setPosition(Vec2(450 + 70 + 150, y));
    win_count_font->setColor( Color3B( 0, 0, 255) );
    scrollView->addChild(win_count_font, 0);

    auto win_font_x = win_count_font->getPosition().x + (win_count_font->getContentSize().width / 2.0f) + 20.0f;
    auto win_font = Label::createWithTTF(" 승", "fonts/nanumb.ttf", 30);
    win_font->setPosition(Vec2(win_font_x, y));
    win_font->setColor( Color3B( 10, 0, 10) );
    scrollView->addChild(win_font, 0);

    // lose
    auto lose_count_font = Label::createWithTTF(ccsf2("%d", ranking_infos_[i].lose_count), "fonts/nanumb.ttf", 30);
    lose_count_font->setPosition(Vec2(win_font_x + 80, y));
    lose_count_font->setColor( Color3B( 255, 0, 0) );
    scrollView->addChild(lose_count_font, 0);

    auto lose_font_x = lose_count_font->getPosition().x + (lose_count_font->getContentSize().width / 2.0f) + 20.0f;
    auto lose_font = Label::createWithTTF(" 패", "fonts/nanumb.ttf", 30);
    lose_font->setPosition(Vec2(lose_font_x, y));
    lose_font->setColor( Color3B( 10, 0, 10) );
    scrollView->addChild(lose_font, 0);

    
    y = y - (ranking_bar_height + margin);
  }
 
  this->addChild(scrollView);
}

void ranking_scene::create_connection_popup() {
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

void ranking_scene::open_connection_popup() {
  connection_background_popup->setPosition(Vec2(center_));
  connection_noti_font->setPosition(Vec2(center_.x, center_.y + 60.0f));
  connection_confirm_button->setPosition(Vec2(center_.x, center_.y - 100.0f));
  loading_status_font->setPosition(Vec2(center_.x + 5000.0f, center_.y));
}

void ranking_scene::close_connection_popup() {
  auto offset = 5000.0f;
  connection_background_popup->setPosition(Vec2(center_.x + offset, center_.y));
  connection_noti_font->setPosition(Vec2(center_.x + offset, center_.y + 60.0f));
  connection_confirm_button->setPosition(Vec2(center_.x + offset, center_.y - 100.0f));
}

void ranking_scene::create_loading_status_font() {
  loading_status_font = Label::createWithTTF("랭킹 정보를 불러오는중", "fonts/nanumb.ttf", 40);
  loading_status_font->setPosition(Vec2(Director::getInstance()->getVisibleSize().width/2.0f, center_.y));
  loading_status_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(loading_status_font, 3);

  auto scaleTo = ScaleTo::create(1.1f, 1.1f);
  loading_status_font->runAction(scaleTo);
  auto delay = DelayTime::create(0.25f);
  auto scaleTo2 = ScaleTo::create(1.0f, 1.0f);
  auto seq = Sequence::create(scaleTo, delay, scaleTo2, delay->clone(), nullptr);
  loading_status_font->runAction(RepeatForever::create(seq));
}

