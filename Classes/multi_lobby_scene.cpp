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

  //dummy_data();

  create_ui_buttons();
  create_ui_room_info();
  create_ui_chat_info();

  connection::get().send2(Json::object {
      { "type", "room_list_req" }
    });

  connection::get().send2(Json::object {
      { "type", "chat_list_req" }
    });


  
 
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
  create_room_button->loadTextures("ui/quick_join_button.png", "ui/quick_join_button.png");
  create_room_button->ignoreContentAdaptWithSize(false);
  create_room_button->setContentSize(Size(221, 120));
  create_room_button->setPosition(Vec2(900, center_.y-300));
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
  quick_join_button->setPosition(Vec2(1190, center_.y-300));
  quick_join_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.2f, 1.3f);
	quick_join_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	quick_join_button->runAction(scaleTo2);

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



  // send button
  /*
  send_button = ui::Button::create();
  send_button->setTouchEnabled(true);
  send_button->ignoreContentAdaptWithSize(false);
  send_button->setContentSize(Size(136, 63));
  send_button->loadTextures("ui/playing_button.png", "ui/playing_button.png");

  send_button->setPosition(Vec2(center_.x + 570, center_.y - 192));

  send_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);
	auto scaleTo = ScaleTo::create(0.1f, 1.3f);
	send_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);;
	send_button->runAction(scaleTo2);
	std::string tmp = textField->getString();
	CCLOG("chat msg: %s", tmp.c_str());
	textField->setString("");

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);;
	send_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(send_button, 1);
  */

  // input field
  textField = TextField::create("메세지를 입력해주세요.","fonts/nanumb.ttf", 25);
  textField->setMaxLength(25);
  textField->setColor(Color3B( 0, 0, 0));
  textField->setMaxLengthEnabled(true);
  textField->setTextHorizontalAlignment(TextHAlignment::CENTER);
  textField->setTextVerticalAlignment(TextVAlignment::CENTER);
  textField->setPosition(Vec2(center_.x + 285, center_.y - 192));
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

    } else if(type == "send_chat_noti") {
      chat_msg cm;
      cm.nickname = "지코";
      cm.msg = payload["msg"].string_value();
      chat_msgs.push_back(cm);
      resize_ui_chat_info();

    } else if(type == "room_list_res") {
      CCLOG("[debug] room_list_res");


    } else if(type == "chat_list_res") {
      CCLOG("[debug] chat_list_res");


    } else if(type == "create_room_res") {
      user_info::get().room_info_.is_master = true;

      auto multi_room_scene = multi_room_scene::createScene();
      Director::getInstance()->replaceScene(multi_room_scene);
    } else {
      CCLOG("[error] handler 없음");
    }
}

void multi_lobby_scene::replace_lobby_scene() {
  auto lobby_scene = lobby_scene::createScene();
  Director::getInstance()->replaceScene(lobby_scene);
}

void multi_lobby_scene::create_room_req(std::string title, std::string password) {
  connection::get().send2(Json::object {
      { "type", "create_room_req" },
      { "title", title },
      { "password", password },
   });
}


void multi_lobby_scene::dummy_data() {

  for(auto i=0; i<20; i++) {
    room r;
    r.id = i;
    r.title = "점수 1400점 이상만";
    
    player p;
    p.nickname = "xxxx";
    p.rating = 1420;

    r.players.push_back(p);

    rooms.push_back(r);
  }

  for(auto i=0; i<20; i++) {
    chat_msg cm;
    cm.nickname = "xxxxxxx사랑";
    if(i%2) {
      cm.msg = "밥은 먹고 xxxx?";
    } else {
      cm.msg = "밥은 먹고 aaaaa?2222223333333333332";
    }
    chat_msgs.push_back(cm);
  }

}
