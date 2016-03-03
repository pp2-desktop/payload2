#ifndef __user_info_HPP__
#define __user_info_HPP__

#include <string>
#include <memory>
#include "cocos2d.h"
USING_NS_CC;

const  int playing_game_cost = 200;

class stage {
public:
  std::string img;
  int known_point_count;
  std::vector<Vec2> hidden_points;
  std::set<int> found_indexs;
  bool is_win;
  int find_point_count;
  stage() {
    known_point_count = 0;
    find_point_count = 0;
    is_win = false;
  }
  ~stage() {
  }
};

class room_info {
public:
  bool is_master;
  std::string title;
  std::string pasword;
  std::vector<stage> stages;
};

struct sound_option {
  bool is_background_on;
  bool is_effect_on;
  sound_option();
  ~sound_option();


  void set_background(bool on);
  bool get_background();
  void set_effect(bool on);
  bool get_effect();
};

class account_info {
public:
  account_info() { is_fb_login = false; }
  ~account_info() {}

  std::string get_uid();
  void set_uid(std::string);
  std::string get_facebookid();
  void set_facebookid(std::string);
  std::string get_name();
  void set_name(std::string);
  std::string get_password();
  void set_password(std::string);
  std::string get_other_name();
  void set_other_name(std::string);

  std::string uid;
  std::string facebookid;
  std::string name;
  std::string other_name;

  int score;
  int win_count;
  int lose_count;
  int ranking;
  bool is_fb_login;

  int earn_score;
  int lose_score;
};

class item_info {
public:
  item_info() { hint_count_ = 0; }
  ~item_info() {}
  int get_hint_count();
  void set_hint_count(int hint_count);
  bool use_hint();

  int hint_count_;
};

class user_info {

public:

  user_info();
  ~user_info();

  void init();
  bool create_room(int rid, bool is_master);
  void destroy_room();

  int get_money();
  void set_money(int money);

  std::string uid;

  static user_info& get() {
    static user_info obj;
    return obj;
  }

  int money;
  sound_option sound_option_;
  room_info room_info_;
  account_info account_info_;
  item_info item_info_;

  int version_;
  //std::shared_ptr<room_info> room_info_ptr;
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
