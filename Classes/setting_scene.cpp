#include "setting_scene.hpp"
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

Scene* setting_scene::createScene() {

  auto scene = Scene::create();
  auto layer = setting_scene::create();

  scene->addChild(layer);

  return scene;
}

bool setting_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }


  if(user_info::get().sound_option_.get_background()) {
    auto audio = SimpleAudioEngine::getInstance();
    audio->playBackgroundMusic("sound/besound_buddy.mp3", true);
  }
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);
    

  auto bg = Sprite::create("background/setting_scene.jpg");
  bg->setPosition(Vec2(center_.x, center_.y));
  this->addChild(bg, 0);

  create_ui_top();
  create_ui_buttons();
  
  this->scheduleUpdate();
    
  return true;
}

void setting_scene::create_ui_top() {

  auto ui_top_bg = Sprite::create("ui/top_multi_lobby.png");
  auto y = center_.y + _play_screen_y/2 - _offset_y+0;
  ui_top_bg->setPosition(Vec2(center_.x, center_.y + _play_screen_y/2 - _offset_y+0));
  this->addChild(ui_top_bg, 0);

  auto font_x = 280;
  auto font_y = center_.y + _play_screen_y/2 - _offset_y+0;
  font_y = font_y + 1;
  auto font_size = 30;

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
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	back_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	back_button->runAction(scaleTo2);
        this->scheduleOnce(SEL_SCHEDULE(&setting_scene::replace_lobby_scene), 0.2f); 
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	back_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(back_button, 0);
}

void setting_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }
  
}

void setting_scene::replace_lobby_scene() {
  auto lobby_scene = lobby_scene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.0f, lobby_scene, Color3B(0,255,255)));
}

void setting_scene::handle_payload(float dt) {
    Json payload = connection::get().q.front();
    connection::get().q.pop_front();
    std::string type = payload["type"].string_value();

    if(type == "connection_notify") {
      CCLOG("[debug] 접속 성공");
      connection::get().send2(Json::object {
	  { "type", "login_req" }
	});

    }  else {
      CCLOG("[error] handler 없음");
    }
}


void setting_scene::create_ui_buttons() {
  // background sound
  auto switchBackgroundSoundControl = ControlSwitch::create(
                Sprite::create("ui/switch-mask.png"),
                Sprite::create("ui/switch-on.png"),
                Sprite::create("ui/switch-off.png"),
                Sprite::create("ui/switch-thumb.png"),
                Label::createWithTTF("켬", "fonts/nanumb.ttf", 16),
                Label::createWithTTF("끔", "fonts/nanumb.ttf", 16)
            );
  switchBackgroundSoundControl->setPosition(Vec2(center_.x - 300.0f, center_.y + 90.0f));
  switchBackgroundSoundControl->setScale(2.2f);
  switchBackgroundSoundControl->setOn(user_info::get().sound_option_.get_background());
  this->addChild(switchBackgroundSoundControl);
  
  switchBackgroundSoundControl->addTargetWithActionForControlEvents(this, cccontrol_selector(setting_scene::value_background_sound_changed), Control::EventType::VALUE_CHANGED);

  //value_background_sound_changed(switchBackgroundSoundControl, Control::EventType::VALUE_CHANGED);

  // effect_sound
  auto switchEffectdSoundControl = ControlSwitch::create(
                Sprite::create("ui/switch-mask.png"),
                Sprite::create("ui/switch-on.png"),
                Sprite::create("ui/switch-off.png"),
                Sprite::create("ui/switch-thumb.png"),
                Label::createWithTTF("켬", "fonts/nanumb.ttf", 16),
                Label::createWithTTF("끔", "fonts/nanumb.ttf", 16)
            );
  switchEffectdSoundControl->setPosition(Vec2(center_.x - 300.0f, center_.y - 175.0f));
  switchEffectdSoundControl->setScale(2.2f);
  switchEffectdSoundControl->setOn(user_info::get().sound_option_.get_effect());
  this->addChild(switchEffectdSoundControl);
  
  switchEffectdSoundControl->addTargetWithActionForControlEvents(this, cccontrol_selector(setting_scene::value_effect_sound_changed), Control::EventType::VALUE_CHANGED);

  //value_effect_sound_changed(switchEffectdSoundControl, Control::EventType::VALUE_CHANGED);
}

void setting_scene::value_background_sound_changed(Ref* sender, Control::EventType controlEvent) {
    ControlSwitch* pSwitch = (ControlSwitch*)sender;
    if (pSwitch->isOn()) {
      auto audio = SimpleAudioEngine::getInstance();
      audio->playEffect("sound/toggle.wav");
      audio->setBackgroundMusicVolume(0.4f);
      user_info::get().sound_option_.set_background(true);
    } 
    else {
      auto audio = SimpleAudioEngine::getInstance();
      audio->playEffect("sound/toggle.wav");
      audio->setBackgroundMusicVolume(0.0f);
      user_info::get().sound_option_.set_background(false);
    }
}

void setting_scene::value_effect_sound_changed(Ref* sender, Control::EventType controlEvent) {
    ControlSwitch* pSwitch = (ControlSwitch*)sender;
    if (pSwitch->isOn()) {
      auto audio = SimpleAudioEngine::getInstance();
      audio->setEffectsVolume(1.0f);
      audio->playEffect("sound/toggle.wav");
      user_info::get().sound_option_.set_effect(true);
    } 
    else {
      auto audio = SimpleAudioEngine::getInstance();
      audio->playEffect("sound/toggle.wav");
      audio->setEffectsVolume(0.0f);
      user_info::get().sound_option_.set_effect(false);
    }
}
