#include "SimpleAudioEngine.h"
#include "lobby_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"
#include "single_lobby_scene.hpp"
#include "multi_lobby_scene.hpp"
#include "assets_scene.hpp"
#include "resource_md.hpp"
#include <chrono>
//#include "single_play_scene.hpp"
using namespace CocosDenshion;

Scene* lobby_scene::createScene() {

  auto scene = Scene::create();
  auto layer = lobby_scene::create();

  scene->addChild(layer);

  return scene;
}

// on "init" you need to initialize your instance
bool lobby_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }

  // 파싱 데이터
  /*
  CCUserDefault *def=CCUserDefault::sharedUserDefault();
  def-> setIntegerForKey("high_score", 2000);
  def->flush();
  // int high_score=def->getIntegerForKey("high_score");
  //setStringForKey
  CCLOG("high score: %d", high_score);
  */
  auto audio = SimpleAudioEngine::getInstance();
  audio->playBackgroundMusic("sound/bg2.mp3", true);
  audio->setBackgroundMusicVolume(0.4f);
  
  // 커넥터 초기화
  
  if(!connection::get().get_is_connected()) {
    connection::get().create("ws://t.05day.com:8080/echo");
    connection::get().connect();
  }
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);
    
  auto closeItem = MenuItemImage::create(
					 "CloseNormal.png",
					 "CloseSelected.png",
					 CC_CALLBACK_1(lobby_scene::menuCloseCallback, this));
  closeItem->setScale(2.0f, 2.0f);
  closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2-20, origin.y + closeItem->getContentSize().height/2+15));

  // create menu, it's an autorelease object
  auto menu = Menu::create(closeItem, NULL);
  menu->setPosition(Vec2::ZERO);
  this->addChild(menu, 1);

  /*
  auto background_all = Sprite::create("background/all.jpg");
  background_all->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background_all, 0);
  */

  // 
  auto background = Sprite::create("background/lobby_scene.png");
  //auto background = Sprite::create(resource_md::get().path + "right_2.jpg");
  background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background, 0);

  sp_button = ui::Button::create();
  sp_button->setTouchEnabled(true);
  //sp_button->setScale(3.0f);
  sp_button->ignoreContentAdaptWithSize(false);
  sp_button->setContentSize(Size(200.0f, 200.0f));
  sp_button->loadTextures("ui/sp_button.png", "ui/sp_button.png");

  sp_button->setPosition(Vec2(222, center_.y));

  sp_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	if(is_popup_on) return;

        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3", false, 1.0f, 1.0f, 1.0f);

	auto scaleTo = ScaleTo::create(0.1f, 1.3f);
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	sp_button->runAction(seq2);

        this->scheduleOnce(SEL_SCHEDULE(&lobby_scene::replace_single_lobby_scene), 0.2f); 
      }
    });
     
  this->addChild(sp_button, 0);


  mp_button = ui::Button::create();
  mp_button->setTouchEnabled(true);
  //mp_button->setScale(3.0f);
  mp_button->ignoreContentAdaptWithSize(false);
  mp_button->setContentSize(Size(200.0f, 200.0f));
  mp_button->loadTextures("ui/mp_button.png", "ui/mp_button.png");

  mp_button->setPosition(Vec2(222*2, center_.y));

  mp_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	if(is_popup_on) return;

	auto scaleTo = ScaleTo::create(0.1f, 1.3f);
	mp_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	mp_button->runAction(scaleTo2);

        if(connection::get().get_is_connected()) {
          open_multi_popup();
        } else {
          open_connection_popup(1);
        }


      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	mp_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(mp_button, 0);


  ranking_button = ui::Button::create();
  ranking_button->setTouchEnabled(true);
  //ranking_button->setScale(3.0f);
  ranking_button->ignoreContentAdaptWithSize(false);
  ranking_button->setContentSize(Size(200.0f, 200.0f));
  ranking_button->loadTextures("ui/ranking_button.png", "ui/ranking_button.png");

  ranking_button->setPosition(Vec2(center_.x, center_.y));

  ranking_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	if(is_popup_on) return;
	auto scaleTo = ScaleTo::create(0.1f, 1.3f);
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	ranking_button->runAction(seq2);
      }
    });
     
  this->addChild(ranking_button, 0);


  setting_button = ui::Button::create();
  setting_button->setTouchEnabled(true);
  //setting_button->setScale(3.0f);
  setting_button->ignoreContentAdaptWithSize(false);
  setting_button->setContentSize(Size(200.0f, 200.0f));
  setting_button->loadTextures("ui/setting_button.png", "ui/setting_button.png");

  setting_button->setPosition(Vec2(889, center_.y));

  setting_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	if(is_popup_on) return;
	auto scaleTo = ScaleTo::create(0.1f, 1.3f);
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	setting_button->runAction(seq2);
	//tmp();
      }
    });
     
  this->addChild(setting_button, 0);


  quit_button = ui::Button::create();
  quit_button->setTouchEnabled(true);
  //quit_button->setScale(3.0f);
  quit_button->ignoreContentAdaptWithSize(false);
  quit_button->setContentSize(Size(200.0f, 200.0f));
  quit_button->loadTextures("ui/quit_button.png", "ui/quit_button.png");

  quit_button->setPosition(Vec2(1111, center_.y));

  quit_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	if(is_popup_on) return;
	auto scaleTo = ScaleTo::create(0.1f, 1.3f);
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	quit_button->runAction(seq2);
      }
    });
     
  this->addChild(quit_button, 0);


  /* 닉네임 인풋 받는 부분
  textField = TextField::create("bbbb","Arial", 40);
  textField->setMaxLength(10);
  textField->setMaxLengthEnabled(true);
  textField->setPosition(Vec2(center_.x, center_.y));
  textField->addEventListener([&](Ref* sender,ui::TextField::EventType event) {
      CCLOG("%s", textField->getString().c_str());
    });

  this->addChild(textField, 2);
  */
 

  /*
  ActionInterval* lens = Lens3D::create(1, Size(32,24), Vec2(100,180), 150);
  ActionInterval* waves = Waves3D::create(1, Size(15,10), 18, 15);
  auto nodeGrid = NodeGrid::create();
  nodeGrid->addChild(background);
  nodeGrid->runAction(Sequence::create(waves, lens, NULL));
  this->addChild(nodeGrid);
  */

  create_multi_popup();
  create_connection_popup();
  is_requesting = false;
  is_popup_on = false;

  //texture_ptr = new Texture2D();

  this->scheduleUpdate();
    
  return true;
}

void lobby_scene::menuCloseCallback(Ref* pSender) {
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}

void lobby_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }
  
  //CCLOG("update");
}

void lobby_scene::replace_single_lobby_scene() {
  auto single_lobby_scene = single_lobby_scene::createScene();
  Director::getInstance()->replaceScene(single_lobby_scene);
}

void lobby_scene::replace_multi_lobby_scene() {
  auto multi_lobby_scene = multi_lobby_scene::createScene();
  Director::getInstance()->replaceScene(multi_lobby_scene);
}

void lobby_scene::handle_payload(float dt) {
    Json payload = connection::get().q.front();
    connection::get().q.pop_front();
    std::string type = payload["type"].string_value();

    if(type == "connection_notify") {
      CCLOG("[debug] 접속 성공");
     
      /*
      connection::get().send2(Json::object {
	  { "type", "login_req" }
	});
      */

    } else if(type == "disconnection_notify") {
      CCLOG("[debug] 접속 큰킴");
      
    } else if(type == "update_alive_noti") { 
      CCLOG("[noti] update alive noti");
      connection::get().send2(Json::object {
	  { "type", "update_alive_noti" }
	});
    } else if(type == "login_res") {
      bool result = payload["result"].bool_value();
      if(result) {
        //user_info::get().uid = payload["uid"].string_value();
        auto score = payload["score"].int_value();
        auto win_count = payload["win_count"].int_value();
        auto lose_count = payload["lose_count"].int_value();
        auto ranking = payload["ranking"].int_value();

        user_info::get().account_info_.score = score;
        user_info::get().account_info_.win_count = win_count;
        user_info::get().account_info_.lose_count = lose_count;
        user_info::get().account_info_.ranking = ranking;

        CCLOG("uid: %s", user_info::get().uid.c_str());
        replace_multi_lobby_scene();
      } else {
        CCLOG("로그인에 실패하였습니다");
      }
    } else if(type == "create_guest_account_res") {
      auto uid = payload["uid"].string_value();
      auto name = payload["name"].string_value();
      auto password = payload["password"].string_value();
      
      CCLOG("uid %s", uid.c_str());

      user_info::get().account_info_.set_uid(uid);
      user_info::get().account_info_.set_name(name);
      user_info::get().account_info_.set_password(password);
      login_req(user_info::get().account_info_.get_uid(), user_info::get().account_info_.get_name(), user_info::get().account_info_.get_password());
    } else {
      CCLOG("[error] handler 없음");
    }
}

void lobby_scene::handle_sound(sound_type type) {
  auto audio = SimpleAudioEngine::getInstance();

  if(type == sound_type::BUTTON_PRESSED) {
    audio->playEffect("sound/pressing.wav", false, 1.0f, 1.0f, 1.0f);
  }
}

void lobby_scene::create_guest_account() {
  connection::get().send2(Json::object {
      { "type", "create_guest_account_req" }
    });
  /*
    time_t _time = time(0);
    std::chrono::system_clock::time_point _now = std::chrono::system_clock::from_time_t(_time);
    CCLOG("%ld", _now);
  */
}

void lobby_scene::login_req(std::string uid, std::string name, std::string password) {
  connection::get().send2(Json::object {
      { "type", "login_req" },
      { "uid", uid },
      { "name", name },
      { "password", password }
    });
}

void lobby_scene::create_multi_popup() {
  auto offset = 5000.0f;
  background_popup = Sprite::create("ui/background_popup.png");
  background_popup->setScale(2.0f);
  background_popup->setPosition(Vec2(center_.x + offset, center_.y));
  this->addChild(background_popup, 0);

  facebook_login_button = ui::Button::create();
  facebook_login_button->setTouchEnabled(true);
  facebook_login_button->ignoreContentAdaptWithSize(false);
  facebook_login_button->setContentSize(Size(312.0f, 60.0f));
  facebook_login_button->loadTextures("ui/facebook_login.png", "ui/facebook_login.png");
  facebook_login_button->setPosition(Vec2(center_.x + offset, center_.y + 80.0f));
  facebook_login_button->setScale(2.0f);

  facebook_login_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 2.2f);
	facebook_login_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 2.0f);
	facebook_login_button->runAction(scaleTo2);

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 2.0f);
	facebook_login_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(facebook_login_button, 0);

  guest_login_button = ui::Button::create();
  guest_login_button->setTouchEnabled(true);
  guest_login_button->ignoreContentAdaptWithSize(false);
  guest_login_button->setContentSize(Size(312.0f, 60.0f));
  guest_login_button->loadTextures("ui/guest_login.png", "ui/guest_login.png");
  guest_login_button->setPosition(Vec2(center_.x + offset, center_.y - 80.0f));
  guest_login_button->setScale(2.0f);

  guest_login_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 2.2f);
	guest_login_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
        if(is_requesting) return;
	auto scaleTo2 = ScaleTo::create(0.1f, 2.0f);
	guest_login_button->runAction(scaleTo2);
        is_requesting = true;
        guest_login();
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 2.0f);
	guest_login_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(guest_login_button, 0);

  close_popup_button = ui::Button::create();
  close_popup_button->setTouchEnabled(true);
  close_popup_button->ignoreContentAdaptWithSize(false);
  close_popup_button->setContentSize(Size(128.0f, 128.0f));
  close_popup_button->loadTextures("ui/close_popup.png", "ui/close_popup.png");
  close_popup_button->setPosition(Vec2(background_popup->getPosition().x + (background_popup->getContentSize().width - 20.0f) + offset, background_popup->getPosition().y + background_popup->getContentSize().height - 20.0f ));
  close_popup_button->setScale(0.7f);

  close_popup_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 0.9f);
        close_popup_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.7f);
        close_popup_button->runAction(scaleTo2);
        close_multi_popup();
        is_requesting = false;

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.7f);
        close_popup_button->runAction(scaleTo2);
      }
    });
     
  close_popup_button->setOpacity(190);
  this->addChild(close_popup_button, 0);
}

void lobby_scene::open_multi_popup() {
  is_popup_on = true;
  background_popup->setPosition(Vec2(center_));
  facebook_login_button->setPosition(Vec2(center_.x, center_.y + 80.0f));
  guest_login_button->setPosition(Vec2(center_.x, center_.y - 80.0f));
  close_popup_button->setPosition(Vec2(background_popup->getPosition().x + (background_popup->getContentSize().width - 20.0f), background_popup->getPosition().y + background_popup->getContentSize().height - 20.0f ));
}

void lobby_scene::close_multi_popup() {
  is_popup_on = false;
  auto offset = 5000.0f;
  background_popup->setPosition(Vec2(center_.x + offset, center_.y));
  facebook_login_button->setPosition(Vec2(center_.x + offset, center_.y + 80.0f));
  guest_login_button->setPosition(Vec2(center_.x + offset, center_.y - 80.0f));
  close_popup_button->setPosition(Vec2(background_popup->getPosition().x + offset + (background_popup->getContentSize().width - 20.0f), background_popup->getPosition().y + background_popup->getContentSize().height - 20.0f ));
}

void lobby_scene::facebook_login() {

}

void lobby_scene::guest_login() {
  if(user_info::get().account_info_.get_name() == "") {
    create_guest_account(); 
  } else {
    CCLOG("uid: %s", user_info::get().account_info_.get_uid().c_str());
    login_req(user_info::get().account_info_.get_uid(), user_info::get().account_info_.get_name(), user_info::get().account_info_.get_password());
  }
}

void lobby_scene::create_connection_popup() {
  auto offset = 5000.0f;
  connection_background_popup = Sprite::create("ui/background_popup.png");
  connection_background_popup->setScale(2.0f);
  connection_background_popup->setPosition(Vec2(center_.x + offset, center_.y));
  this->addChild(connection_background_popup, 0);

  connection_noti_font = Label::createWithTTF("네트워크 불안정 상태로 서버와 접속 끊김", "fonts/nanumb.ttf", 40);
  connection_noti_font->setPosition(Vec2(center_.x + offset, center_.y));
  connection_noti_font->setColor(Color3B( 110, 110, 110));
  this->addChild(connection_noti_font, 0);

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
        close_connection_popup();

        if(!connection::get().get_is_connected()) {
          connection::get().create("ws://t.05day.com:8080/echo");
          connection::get().connect();
        }

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 1.0f);
        connection_confirm_button->runAction(scaleTo);
      }
    });
     
  this->addChild(connection_confirm_button, 0);
}

void lobby_scene::open_connection_popup(int type) {
  is_popup_on = true;
  if(type == 1) {
    connection_noti_font->setAnchorPoint(ccp(0.5f,0.5f));
    connection_noti_font->setString("        서버에 접속 실패!\n네트워크 상태를 확인해주세요");
  } else {
    connection_noti_font->setAnchorPoint(ccp(0.5f,0.5f));
    connection_noti_font->setString("네트워크 불안정 상태로 서버와 접속 끊김");
  }
  connection_background_popup->setPosition(Vec2(center_));
  connection_noti_font->setPosition(Vec2(center_.x, center_.y + 60.0f));
  connection_confirm_button->setPosition(Vec2(center_.x, center_.y - 100.0f));
}

void lobby_scene::close_connection_popup() {
  is_popup_on = false;
  auto offset = 5000.0f;
  connection_background_popup->setPosition(Vec2(center_.x + offset, center_.y));
  connection_noti_font->setPosition(Vec2(center_.x + offset, center_.y + 60.0f));
  connection_confirm_button->setPosition(Vec2(center_.x + offset, center_.y - 100.0f));
}

void lobby_scene::tmp() {
  string _id = "100005347304902"; // id require to whome you want to fectch photo
  cocos2d::network::HttpRequest* request = new (std::nothrow) cocos2d::network::HttpRequest();
  string url = "https://graph.facebook.com/"+_id+"/picture?height=120&width=120";
  request->setUrl(url.c_str());
  request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
  request->setResponseCallback(CC_CALLBACK_2(lobby_scene::onRequestImgCompleted, this));
  //request->setTag("GetImage");
  cocos2d::network::HttpClient::getInstance()->send(request);
  request->release();
}
static int xx = 0;
void lobby_scene::onRequestImgCompleted(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response)
{
  if(!response) {
    return;
  }

  if(!response->isSucceed())
    {
      return;
    }

  std::vector<char>* buffer = response->getResponseData();

  Image* image = new Image ();
  image->initWithImageData ( reinterpret_cast<const unsigned char*>(&(buffer->front())), buffer->size());   

  texture.initWithImage(image);
    
  Sprite* sp = Sprite::createWithTexture(&texture);
  this->addChild(sp);
  sp->setPosition(Vec2(0 + xx, center_.y));
  xx = xx + 50;
  delete image;  
}
