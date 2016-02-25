#include "user_info.hpp"
#include "cocos2d.h"

user_info::user_info() : version_(1){
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
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  return pUserDefault->getBoolForKey("background_sound", true);
}

void sound_option::set_effect(bool on) {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  pUserDefault->setBoolForKey("effect_sound", on);
  pUserDefault->flush();
  is_effect_on = on;
}

bool sound_option::get_effect() {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  return pUserDefault->getBoolForKey("effect_sound", true);
}

std::string account_info::get_uid() {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  return pUserDefault->getStringForKey("uid", "");
}

void account_info::set_uid(std::string uid) {
  CCLOG("set uid called");
  CCLOG("uid :%s", uid.c_str());
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  pUserDefault->setStringForKey("uid", uid);
  pUserDefault->flush();
}

std::string account_info::get_facebookid() {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  return pUserDefault->getStringForKey("facebook", "");
}

void account_info::set_facebookid(std::string facebookid) {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  pUserDefault->setStringForKey("facebookid", facebookid);
  pUserDefault->flush();
}

std::string account_info::get_name() {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  return pUserDefault->getStringForKey("name", "");
}

void account_info::set_name(std::string name) {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  pUserDefault->setStringForKey("name", name);
  pUserDefault->flush();
}

std::string account_info::get_password() {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  return pUserDefault->getStringForKey("password", "");
}

void account_info::set_password(std::string name) {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  pUserDefault->setStringForKey("password", name);
  pUserDefault->flush();
}

std::string account_info::get_other_name() {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  return pUserDefault->getStringForKey("other_name", "");
}

void account_info::set_other_name(std::string name) {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  pUserDefault->setStringForKey("other_name", name);
  pUserDefault->flush();
}
