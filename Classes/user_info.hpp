#ifndef __user_info_HPP__
#define __user_info_HPP__

#include <string>
#include <memory>
#include "cocos2d.h"
USING_NS_CC;

class room_info {
public:
  bool is_master_;
  int rid_;

  room_info(int rid, bool is_master) {
    rid_ = rid;
    is_master_ = is_master;
  }
  ~room_info() {}

};

class user_info {

public:

  user_info();
  ~user_info();

  void init();
  bool create_room(int rid, bool is_master);
  void destroy_room();

  std::string uid;

  static user_info& get() {
    static user_info obj;
    return obj;
  }

  int money;

  std::shared_ptr<room_info> room_info_ptr;
};

#define num_to_string(...) CCString::createWithFormat(__VA_ARGS__)->getCString()

static std::string num_to_money(int money) {
  std::string input = num_to_string("%d", money);

  for( int i = input.size() - 3; i > 0; i -= 3 ) {
    input.insert(  input.begin() + i, ',' );
  }

  return input;
}

#endif
