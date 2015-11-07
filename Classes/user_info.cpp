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

bool user_info::create_room(int rid, bool is_master) {

  if(room_info_ptr) {
    return false;
  }

  room_info_ptr = std::make_shared<room_info>(rid, is_master);
  return true;
}
