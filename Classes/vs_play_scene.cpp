#include "vs_play_scene.hpp"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "connection.hpp"
#include "user_info.hpp"

using namespace ui;
using namespace CocosDenshion;

Scene* vs_play_scene::createScene()
{

  auto scene = Scene::create();
   
  auto layer = vs_play_scene::create();

  scene->addChild(layer);

  return scene;
}


bool vs_play_scene::init()
{
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }
    
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);

  // 백그라운드
  auto background = Sprite::create("background/vs_play_scene.png");
  background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
  this->addChild(background, 0);

    
  // xx


  // round_info_req
  // round_info_res => name of map, collision points and round count

  this->scheduleUpdate();

  connection::get().send2(Json::object {
      { "type", "round_info_req" }
    });

  return true;
}

void vs_play_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    CCLOG("11111111111111111111111111111111");
    handle_payload(dt);
  } 


  //CCLOG("update");
}

void vs_play_scene::handle_payload(float dt) {

  Json payload = connection::get().q.front();
  connection::get().q.pop_front();

  std::string type = payload["type"].string_value();
  CCLOG("type: %s", type.c_str());
  

  if (type == "connection_notify") {

  } else if (type == "disconnection_notify") {
    CCLOG("[debug] 접속 큰킴");
    //user_info::get().destroy_room(); 
    // after reconnect prev scene

  } else if (type == "round_info_res") {

    round_info_res(payload["round_infos"]);
    
  } else if (type == "start_round_res" ) {

    // 시작

  } else if (type == "find_spot_res" ) {

    // check who's?

    // end of game? next round?

  } else {
    CCLOG("[error] vs_play_scene handler 없음");
  }

}

void vs_play_scene::round_info_res(Json round_infos) {

  for (auto &r : round_infos.array_items()) {
    auto img0 = r["img0"].string_value();
    auto img1 = r["img1"].string_value();

    CCLOG("img0: %s", img0.c_str());
    CCLOG("img1: %s", img1.c_str());

    auto points = r["point_infos"];
    for(auto& p : points.array_items()) {
      auto point = p.int_value();
      CCLOG("ponts: %d", point);
    }
  }
}

void vs_play_scene::handle_sound(sound_type type) {

  auto audio = SimpleAudioEngine::getInstance();
  if(type == sound_type::BUTTON_PRESSED) {
    audio->playEffect("sound/button_pressed.mp3", false, 1.0f, 1.0f, 1.0f);
  }
}
