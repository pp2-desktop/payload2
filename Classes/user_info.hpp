#ifndef __user_info_HPP__
#define __user_info_HPP__

#include <memory>

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

  std::string uid;

  static user_info& get() {
    static user_info obj;
    return obj;
  }

  std::shared_ptr<room_info> room_info_ptr;
};

#endif
