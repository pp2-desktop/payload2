#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "multi_lobby_scene.hpp"
#include "lobby_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"
#include "json11.hpp"
#include "single_play_info.hpp"
#include "multi_room_scene.hpp"

using namespace ui;
using namespace CocosDenshion;
using namespace json11;

Scene* multi_lobby_scene::createScene() {

  auto scene = Scene::create();
  auto layer = multi_lobby_scene::create();

  scene->addChild(layer);

  return scene;
}

bool multi_lobby_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }
  //dummy_data();
  auto audio = SimpleAudioEngine::getInstance();
  //audio->playBackgroundMusic("sound/bg1.mp3", true);
  //audio->setBackgroundMusicVolume(0.5f);
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);

  auto background = Sprite::create("background/lobby_scene.png");
  //auto background = Sprite::create(resource_md::get().path + "right_2.jpg");
  background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background, 0);

  create_ui_top();
  //dummy_data();

  CCLOG("다시 멀티 로비로 돌아옴");

  create_ui_buttons();
  create_ui_room_info();
  create_ui_chat_info();

  connection::get().send2(Json::object {
      { "type", "join_lobby_req" }
    });

  connection::get().send2(Json::object {
      { "type", "room_list_req" }
    });

  connection::get().send2(Json::object {
      { "type", "chat_list_req" }
    });

  is_requesting = false;
  is_quick_requesting = false;
 
  this->scheduleUpdate();
    
  return true;
}

void multi_lobby_scene::create_ui_buttons() {

  auto y = center_.y + _play_screen_y/2 - _offset_y+0;

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
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);;
	back_button->runAction(scaleTo2);
        this->scheduleOnce(SEL_SCHEDULE(&multi_lobby_scene::replace_lobby_scene), 0.2f); 

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);;
	back_button->runAction(scaleTo2);
      }
    });

  this->addChild(back_button, 0);



  create_room_button = ui::Button::create();
  create_room_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  create_room_button->loadTextures("ui/create_button2.png", "ui/create_button2.png");
  create_room_button->ignoreContentAdaptWithSize(false);
  create_room_button->setContentSize(Size(221, 120));
  create_room_button->setPosition(Vec2(920, center_.y-300));
  create_room_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.2f, 1.3f);
	create_room_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	create_room_button->runAction(scaleTo2);
	create_room_req("아무나 빨리좀 들어오세요 제발", "");


      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	create_room_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(create_room_button, 0);



  // 빠른 접속
  quick_join_button = ui::Button::create();
  quick_join_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  quick_join_button->loadTextures("ui/quick_join_button.png", "ui/quick_join_button.png");
  quick_join_button->ignoreContentAdaptWithSize(false);
  quick_join_button->setContentSize(Size(221, 120));
  quick_join_button->setPosition(Vec2(1170, center_.y-300));
  quick_join_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.2f, 1.3f);
	quick_join_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	quick_join_button->runAction(scaleTo2);

        is_quick_requesting = true;
        connection::get().send2(Json::object {
            { "type", "quick_join_req" }
          });

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	quick_join_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(quick_join_button, 0);

}

void multi_lobby_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }
  
}

void multi_lobby_scene::create_ui_room_info() {
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();  

  auto scroll_frame_width = 740; 
  auto scroll_frame_height = 650; 

  Size scollFrameSize = Size(scroll_frame_width, scroll_frame_height);

  scrollView = cocos2d::ui::ScrollView::create();
  scrollView->setContentSize(scollFrameSize);
  scrollView->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
  scrollView->setBackGroundColor(Color3B(200, 200, 200));
  //scrollView->setOpacity(80);

  // 
  auto cheight = ((visibleSize.height - scollFrameSize.height) / 2) - 30.0f;
  scrollView->setPosition(Point(25, cheight));
  scrollView->setDirection(cocos2d::ui::ScrollView::Direction::VERTICAL);
  //scrollView->setBounceEnabled(true);
  scrollView->setTouchEnabled(true);

  const auto margin = 10.0f;

  auto room_bar_height = 69.0f;
  auto containerSize = Size(scollFrameSize.width, (room_bar_height+margin) * rooms.size() + (room_bar_height / 2.0f) - margin);

  if(containerSize.height < scroll_frame_height) {
    containerSize.height = scroll_frame_height;
  }

  scrollView->setInnerContainerSize(containerSize);
  auto y = containerSize.height - (room_bar_height / 2.0f) - margin;
  for(auto i=rooms.size(); i>0; i--) {

    auto index = i - 1;

    // 룸 백그라운드(sprite)
    auto tmp = Sprite::create("ui/room_bar.png");
    tmp->setPosition(Vec2(scroll_frame_width/2.0f, y));
    scrollView->addChild(tmp, 0);
    rooms[index].sprite_ptr = tmp;

    // 룸 방제목(label)
    auto room_font = Label::createWithTTF(rooms[index].title.c_str(), "fonts/nanumb.ttf", 30);
    room_font->setPosition(Vec2((room_font->getContentSize().width/2.0f)+25.0f, y));
    room_font->setColor(Color3B( 47, 79, 79));
    scrollView->addChild(room_font, 0);
    rooms[index].label_ptr = room_font;
    
    //chat_fonts.push_back(room_font);

    // 룸 상태 버튼(button)
    auto join_button = ui::Button::create();
    join_button->setTouchEnabled(true);
    join_button->loadTextures("ui/join_button.png", "ui/join_button.png");
    join_button->ignoreContentAdaptWithSize(false);
    join_button->setContentSize(Size(136, 63));
    join_button->setPosition(Vec2(scroll_frame_width/2.0f+272.0f, y));

    join_button->addTouchEventListener([&, index](Ref* sender, Widget::TouchEventType type) {
        auto i = index;
        if(type == ui::Widget::TouchEventType::BEGAN) {
          auto scaleTo = ScaleTo::create(0.2f, 1.2f);
          rooms[i].button_ptr->runAction(scaleTo);

        } else if(type == ui::Widget::TouchEventType::ENDED) {
          auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
          rooms[i].button_ptr->runAction(scaleTo2);
	  join_room_req(rooms[i].id);

        } else if(type == ui::Widget::TouchEventType::CANCELED) {
          auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
          rooms[i].button_ptr->runAction(scaleTo2);
        }
      });
     
    scrollView->addChild(join_button, 0);
    rooms[index].button_ptr = join_button;

    
    y = y - (room_bar_height + margin);
  }
  
  //  vec.erase(std::remove(vec.begin(), vec.end(), 8), vec.end());
 
  this->addChild(scrollView);
  //scrollView->scrollToPercentVertical( 0.0f, 1.0f, true);
}

void multi_lobby_scene::resize_ui_room_info() {

  for(auto i=0; i<rooms.size(); i++) {
   scrollView->removeChild(rooms[i].sprite_ptr, true);
   scrollView->removeChild(rooms[i].label_ptr, true);
   scrollView->removeChild(rooms[i].button_ptr, true);
  }

  auto font_height = 25.0f;
  const auto margin = 5.0f;

  auto room_bar_height = 69.0f;
  auto scroll_frame_width = 740;
  auto scroll_frame_height = 650;
  Size scollFrameSize = Size(scroll_frame_width, scroll_frame_height);

  auto containerSize = Size(scollFrameSize.width, (room_bar_height+margin) * rooms.size() + (room_bar_height / 2.0f) - margin);

  if(containerSize.height < scroll_frame_height) {
    containerSize.height = scroll_frame_height;
  }

  scrollView->setInnerContainerSize(containerSize);

  auto y = containerSize.height - (room_bar_height / 2.0f) - margin;

  auto mid_x = (containerSize.width/2.0f);
  for(auto i=rooms.size(); i>0; i--) {
    auto index = i - 1;

    // 룸 백그라운드(sprite)
    auto tmp = Sprite::create("ui/room_bar.png");
    tmp->setPosition(Vec2(scroll_frame_width/2.0f, y));
    scrollView->addChild(tmp, 0);
    rooms[index].sprite_ptr = tmp;

    // 룸 방제목(label)
    auto room_font = Label::createWithTTF(rooms[index].title.c_str(), "fonts/nanumb.ttf", 30);
    room_font->setPosition(Vec2((room_font->getContentSize().width/2.0f)+25.0f, y));
    room_font->setColor(Color3B( 47, 79, 79));
    scrollView->addChild(room_font, 0);
    rooms[index].label_ptr = room_font;
    
    //chat_fonts.push_back(room_font);

    // 룸 상태 버튼(button)
    auto join_button = ui::Button::create();
    join_button->setTouchEnabled(true);
    join_button->loadTextures("ui/join_button.png", "ui/join_button.png");
    join_button->ignoreContentAdaptWithSize(false);
    join_button->setContentSize(Size(136, 63));
    join_button->setPosition(Vec2(scroll_frame_width/2.0f+272.0f, y));

    join_button->addTouchEventListener([&, index](Ref* sender, Widget::TouchEventType type) {
        auto i = index;
        if(type == ui::Widget::TouchEventType::BEGAN) {
          auto scaleTo = ScaleTo::create(0.2f, 1.2f);
          rooms[i].button_ptr->runAction(scaleTo);

        } else if(type == ui::Widget::TouchEventType::ENDED) {
          auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
          rooms[i].button_ptr->runAction(scaleTo2);
	  join_room_req(rooms[i].id);

        } else if(type == ui::Widget::TouchEventType::CANCELED) {
          auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
          rooms[i].button_ptr->runAction(scaleTo2);
        }
      });
     
    scrollView->addChild(join_button, 0);
    rooms[index].button_ptr = join_button;

    
    y = y - (room_bar_height + margin);
  }

  scrollView->scrollToPercentVertical(0.0f, 1.0f, true);
}

void multi_lobby_scene::create_ui_chat_info() {

  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();  

  auto scroll_frame_width = 520;
  auto scroll_frame_height = 450; 

  Size scollFrameSize = Size(scroll_frame_width-20, scroll_frame_height);

  ChatScrollView = cocos2d::ui::ScrollView::create();
  ChatScrollView->setContentSize(scollFrameSize);
  ChatScrollView->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
  ChatScrollView->setBackGroundColor(Color3B(200, 200, 200));
  ChatScrollView->setOpacity(80);

  ChatScrollView->setPosition(Point(center_.x + 122, center_.y - 225+70));
  ChatScrollView->setDirection(cocos2d::ui::ScrollView::Direction::VERTICAL);
  ChatScrollView->setBounceEnabled(true);
  ChatScrollView->setTouchEnabled(true);

  auto font_height = 25.0f;
  const auto margin = 5.0f;

  auto containerSize = Size(scollFrameSize.width, (font_height+margin) * chat_msgs.size() + (font_height / 2.0f) - margin);

  if(containerSize.height < scroll_frame_height) {
    containerSize.height = scroll_frame_height;
  }

  ChatScrollView->setInnerContainerSize(containerSize);

  auto y = containerSize.height - (font_height / 2.0f) - margin;

  //auto mid_x = (containerSize.width/2.0f);
  for(auto i=0; i<chat_msgs.size(); i++) {
    Label* chat_font = Label::createWithTTF(chat_msgs[i].msg.c_str(), "fonts/nanumb.ttf", font_height);
    CCLOG("font width: %f", chat_font->getContentSize().width);
    chat_font->setPosition(Vec2(chat_font->getContentSize().width/2.0f, y));
    chat_font->setColor(Color3B( 0, 0, 0));
    ChatScrollView->addChild(chat_font, 0);
    chat_fonts.push_back(chat_font);
    y = y - (font_height + margin);
  }

  ChatScrollView->scrollToPercentVertical( 100.0f, 1.0f, true);
  this->addChild(ChatScrollView);
  
  auto chat_input = Sprite::create("ui/chat_input.png");
  chat_input->setPosition(Vec2(center_.x + 370, center_.y - 192));
  this->addChild(chat_input, 0);

  // input field
  textField = TextField::create("메세지를 입력해주세요.","fonts/nanumb.ttf", 25);
  textField->setMaxLength(25);
  textField->setColor(Color3B( 0, 0, 0));
  textField->setMaxLengthEnabled(true);
  textField->setTextHorizontalAlignment(TextHAlignment::CENTER);
  textField->setTextVerticalAlignment(TextVAlignment::CENTER);
  textField->setPosition(Vec2(center_.x + 285, center_.y - 192));
  //textField->setTouchSize(Size(chat_input->getContentSize().width,chat_input->getContentSize().height));
  textField->addEventListener([&](Ref* sender,ui::TextField::EventType event) {

      if(event == TextField::EventType::ATTACH_WITH_IME) {

      } else if(event == TextField::EventType::DETACH_WITH_IME) {

	if(textField->getString().size() > 0 ) {
	  Json payload = Json::object {
	    { "type", "send_chat_noti" },
	    { "msg", textField->getString().c_str() }
	  };
	
	  connection::get().send2(payload);
	}
	textField->setString("");
      } else if(event == TextField::EventType::INSERT_TEXT) {

      } else if(event == TextField::EventType::DELETE_BACKWARD){

      } else {
	//CCLOG("%s", textField->getString().c_str());
      }
    });
  
  this->addChild(textField, 0);

}

void multi_lobby_scene::resize_ui_chat_info() {

  CCLOG("step 0");
  for(auto i=0; i<chat_fonts.size(); i++) {
      ChatScrollView->removeChild(chat_fonts[i], true);
  }
  chat_fonts.clear();
  CCLOG("step 1");

  auto font_height = 25.0f;
  const auto margin = 5.0f;

  auto scroll_frame_width = 520;
  auto scroll_frame_height = 450;
  auto scollFrameSize = Size(scroll_frame_width-20, scroll_frame_height);

  auto containerSize = Size(scollFrameSize.width, (font_height+margin) * chat_msgs.size() + (font_height / 2.0f) - margin);

  if(containerSize.height < scroll_frame_height) {
    containerSize.height = scroll_frame_height;
  }

  ChatScrollView->setInnerContainerSize(containerSize);

  auto y = containerSize.height - (font_height / 2.0f) - margin;

  auto mid_x = (containerSize.width/2.0f);
  for(auto i=0; i<chat_msgs.size(); i++) {
    Label* chat_font = Label::createWithTTF(chat_msgs[i].msg.c_str(), "fonts/nanumb.ttf", font_height);
    CCLOG("font width: %f", chat_font->getContentSize().width);
    chat_font->setPosition(Vec2(chat_font->getContentSize().width/2.0f, y));
    chat_font->setColor( Color3B( 0, 0, 0) );
    ChatScrollView->addChild(chat_font, 0);
    chat_fonts.push_back(chat_font);
    y = y - (font_height + margin);
  }

  ChatScrollView->scrollToPercentVertical( 100.0f, 1.0f, true);
}

void multi_lobby_scene::handle_payload(float dt) {
    Json payload = connection::get().q.front();
    connection::get().q.pop_front();
    std::string type = payload["type"].string_value();

    if(type == "connection_notify") {

      // 접속 끈켜서 재접속시
      CCLOG("[debug] 접속 성공");
      connection::get().send2(Json::object {
	  { "type", "login_req" }
	});

    } else if(type == "update_alive_noti") { 
      CCLOG("[noti] update alive noti");
      connection::get().send2(Json::object {
	  { "type", "update_alive_noti" }
	});
    } else if(type == "send_chat_noti") {
      chat_msg cm;
      cm.name = "지코";
      cm.msg = payload["msg"].string_value();
      chat_msgs.push_back(cm);
      resize_ui_chat_info();

    } else if(type == "room_list_res") {

      auto room_list = payload["room_list"].array_items();
      for(auto& r : room_list) {
	auto rid = r["rid"].int_value();
	auto title = r["title"].string_value();
	auto password = r.string_value();
	auto is_full = r.bool_value();

	room tmp;
	tmp.id = rid;
	tmp.title = title;
	tmp.password = password;
	tmp.is_full = is_full;
	rooms.push_back(tmp);
      }
      resize_ui_room_info();

    } else if(type == "chat_list_res") {
      CCLOG("[debug] chat_list_res");



    } else if(type == "join_lobby_res") {
      CCLOG("[debug] join_lobby_res");



    } else if(type == "create_room_res") {
      user_info::get().room_info_.is_master = true;
      auto multi_room_scene = multi_room_scene::createScene();
      Director::getInstance()->replaceScene(multi_room_scene);

    } else if(type == "quick_join_res") {
      auto is_create = payload["is_create"].bool_value();
      if(is_create) {
        //auto title = payload["title"].bool_value();
        create_room_req(get_quick_room_title(), "");
	//create_room_req("아무나 빨리좀 들어오세요 제발", "");
      } else {
        auto rid = payload["rid"].number_value();
        join_room_req(rid);
      }
      //auto rid = payload["rid"].number_value();

    } else if(type == "join_room_res") {
      auto r = payload["result"].bool_value();
      if(r) {
        user_info::get().room_info_.is_master = false;
        auto multi_room_scene = multi_room_scene::createScene();
        Director::getInstance()->replaceScene(multi_room_scene);
      } else {
        if(is_quick_requesting) {
          // 다시 시도
        } else {
          // 팝업 노티
        }
      }

    } else if(type == "create_room_noti") {
      auto rid = payload["rid"].int_value();
      auto title = payload["title"].string_value();
      auto password = payload["password"].string_value();
      add_room(rid, title, password);
    } else if(type == "destroy_room_noti") {
      auto rid = payload["rid"].int_value();
      remove_room(rid);
    } else {
      CCLOG("[error] handler 없음");
      CCLOG("type: %s", type.c_str());
    }
}

void multi_lobby_scene::replace_lobby_scene() {
  connection::get().send2(Json::object {
      { "type", "leave_lobby_req" }
   });
  auto lobby_scene = lobby_scene::createScene();
  Director::getInstance()->replaceScene(lobby_scene);
}

void multi_lobby_scene::create_room_req(std::string title, std::string password) {
  if(is_requesting) return;

  connection::get().send2(Json::object {
      { "type", "create_room_req" },
      { "title", title },
      { "password", password },
   });

  is_requesting = true;
}

void multi_lobby_scene::join_room_req(int rid) {
  connection::get().send2(Json::object {
      { "type", "join_room_req" },
      { "rid", rid },
   });
}

void multi_lobby_scene::dummy_data() {

  for(auto i=0; i<10; i++) {
    room r;
    r.id = i;
    r.title = "점수 1400점 이상만";
    
    player p;
    p.name = "xxxx";
    p.score = 1420;

    r.players.push_back(p);

    rooms.push_back(r);
  }

  for(auto i=0; i<12; i++) {
    chat_msg cm;
    cm.name = "xxxxxxx사랑";
    if(i%2) {
      cm.msg = "밥은 먹고 xxxx?";
    } else {
      cm.msg = "밥은 먹고 aaaaa?2222223333333333332";
    }
    chat_msgs.push_back(cm);
  }
}

void multi_lobby_scene::add_room(int rid, std::string title, std::string password) {
  room r;
  r.id = rid;
  r.title = title;
  r.password = password;
  player p;
  p.name = "xxxx";
  p.score = 1420;
  r.players.push_back(p);
  rooms.push_back(r);

  resize_ui_room_info();
}

void multi_lobby_scene::remove_room(int rid) {
  auto it = rooms.begin();
  while (it != rooms.end()) {
    if (it->id == rid) {
      scrollView->removeChild(it->sprite_ptr, true);
      scrollView->removeChild(it->label_ptr, true);
      scrollView->removeChild(it->button_ptr, true);

      it = rooms.erase(it);
    }
    else {
      ++it;
    }
  }
  resize_ui_room_info();
}

void multi_lobby_scene::create_ui_top() {
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto ui_offset_x = 70;
  auto font_size = 30;
  auto y = center_.y + _play_screen_y/2 - _offset_y+0;
  auto font_y = center_.y + _play_screen_y/2 - _offset_y+0;
  auto top_ui = Sprite::create("ui/top_multi_lobby.png");
  top_ui->setPosition(Vec2(center_.x, center_.y + _play_screen_y/2 - _offset_y+0));
  this->addChild(top_ui, 0);

  // name
  auto name_font = Label::createWithTTF(user_info::get().account_info_.get_name().c_str(), "fonts/nanumb.ttf", font_size);
  name_font->setPosition(Vec2(ui_offset_x + (name_font->getContentSize().width / 2.0f) + 35.0f, font_y));
  name_font->setColor( Color3B( 0, 255, 0) );
  this->addChild(name_font, 0);

  auto len = user_info::get().account_info_.get_name().size();

  //auto end_name_x = ui_offset_x + (len * 10) + 120 + 40;

  auto end_name_x = ui_offset_x + name_font->getPosition().x + (name_font->getContentSize().width/2.0f);

  // score
  auto score_front_font = Label::createWithTTF("점수: ", "fonts/nanumb.ttf", font_size - 5);
  score_front_font->setPosition(Vec2(end_name_x, font_y));
  score_front_font->setColor( Color3B( 10, 0, 10) );
  this->addChild(score_front_font, 0);

  auto score_font = Label::createWithTTF(ccsf2("%d", user_info::get().account_info_.score), "fonts/nanumb.ttf", font_size);
  score_font->setPosition(Vec2(end_name_x + 70, font_y));
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
}

std::string multi_lobby_scene::get_quick_room_title() {
  srand(time(NULL));
  auto r = rand() % 4;
  if(r == 0) {
    return "아무나 빨리좀 들어오세요 제발";
  } else if(r==1) {
    return "왕초보만 들어오세요";
  } else if(r==2) {
    return "천천히 즐기시면서 하실분만 들어오세요";
  } else {
    return "나 이기면 오만원 쏜다";
  }
}
