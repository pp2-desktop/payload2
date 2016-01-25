#include "user_info.hpp"
#include "cocos2d.h"

user_info::user_info() {
  CCLOG("user_info 생성자 called");
}

user_info::~user_info() {
  CCLOG("user_info 소멸자 called");
}

void user_info::init() {
  CCLOG("user_info 초기화 called");
}

int user_info::get_money() {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  return pUserDefault->getIntegerForKey("money", 1000);
}

void user_info::set_money(int money) {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  if(money <= 0) {
    money = 0;
  }
  pUserDefault->setIntegerForKey("money", money);
}


sound_option::sound_option() {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  is_background_on = pUserDefault->getBoolForKey("background_sound", true);
  is_effect_on = pUserDefault->getBoolForKey("effect_sound", true);
}

sound_option::~sound_option() {
}

void sound_option::set_background(bool on) {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  pUserDefault->setBoolForKey("background_sound", on);
  pUserDefault->flush();
  is_background_on = on;
}

bool sound_option::get_background() {
  return is_background_on;
}

void sound_option::set_effect(bool on) {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  pUserDefault->setBoolForKey("effect_sound", on);
  pUserDefault->flush();
  is_effect_on = on;
}

bool sound_option::get_effect() {
  return is_effect_on;
}
