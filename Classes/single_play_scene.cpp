#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "single_play_scene.hpp"
#include "connection.hpp"
#include "single_play_info.hpp"

//using namespace ui;
using namespace cocos2d::ui;
using namespace CocosDenshion;

Scene* single_play_scene::createScene() {

  auto scene = Scene::create();
  auto layer = single_play_scene::create();

  scene->addChild(layer);
  CCLOG("createScene");
  return scene;
}

// on "init" you need to initialize your instance
bool single_play_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() ) {
      return false;
  }
 
  visible_size = Director::getInstance()->getVisibleSize();
  origin = Director::getInstance()->getVisibleOrigin();
  center = Vec2(visible_size.width/2 + origin.x, visible_size.height/2 + origin.y);

  // tmp
  play_info::get().img = "test.jpg";


  // loading resource


  // loading ui
  // top ui
  auto ui_top_bg = Sprite::create("ui/top.png");
  ui_top_bg->setPosition(Vec2(center.x, center.y + _play_screen_y/2 - _offset_y+0));
  this->addChild(ui_top_bg, 2);

  // loading images
  auto img = play_info::get().img;
  auto left_img = Sprite::create("img/left_" + img);
  left_img->setPosition(Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y));
  this->addChild(left_img, 1);

  auto right_img = Sprite::create("img/right_" + img);
  right_img->setPosition(Vec2( (visible_size.width/2)+(visible_size.width/2/2) + origin.x + _offset_x, visible_size.height/2 + origin.y  - _offset_y));
  this->addChild(right_img, 1);
  

  create_timer();
  this->scheduleOnce(SEL_SCHEDULE(&single_play_scene::ready_go), 0.5f);
  this->scheduleUpdate();

  return true;
}

void single_play_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }
 
  // logic처리
  check_end_play();

  
}

void single_play_scene::create_ready(float move_to_sec, float offset, std::string img) {
  const auto LAST_OFFSET = 300;
  const auto START_SEC = 0.8f;

  auto moveTo = MoveTo::create(START_SEC - move_to_sec, Vec2(center.x + LAST_OFFSET - offset, center.y));
  auto delay = DelayTime::create(0.25f);
  auto moveBy = MoveBy::create(0.25f, Vec2(-50, 0));
  auto moveBy2 = MoveBy::create(0.50f, Vec2(_play_screen_x, 0));

  auto s = Sprite::create(img);
  s->setScale(0.7f);
  s->setPosition(Vec2(-10, center.y));
  this->addChild(s, 1);
  auto seq = Sequence::create(moveTo, delay, moveBy, moveBy2, nullptr);
  s->runAction(seq);
}

void single_play_scene::ready_go() {

  this->scheduleOnce(SEL_SCHEDULE(&single_play_scene::create_go), 1.8f);

  auto offset = 160.0f;
  create_ready(0.4f, 0.0f, "ui/Y.png");
  create_ready(0.3f, offset, "ui/D.png");
  create_ready(0.2f, offset*2, "ui/A.png");
  create_ready(0.1f, offset*3, "ui/E.png");
  create_ready(0.0f, offset*4, "ui/R.png");

  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/Ready_1.wav", false, 1.0f, 1.0f, 1.0f);
}

void single_play_scene::create_go() {

  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/Go.wav", false, 1.0f, 1.0f, 1.0f);
 
  auto g = Sprite::create("ui/G.png");
  g->setScale(0.7f);
  g->setPosition(Vec2(center.x - 120, center.y));
  this->addChild(g, 1);

  auto g_fadeout = FadeOut::create(0.8f);
  g->runAction(g_fadeout);

  auto o = Sprite::create("ui/O.png");
  o->setScale(0.7f);
  o->setPosition(Vec2(center.x + 120, center.y));
  this->addChild(o, 1);

  auto o_fadeout = FadeOut::create(0.8f);
  o->runAction(o_fadeout);

  auto ep = Sprite::create("ui/EP.png");
  ep->setScale(0.7f);
  ep->setPosition(Vec2(center.x + 260, center.y));
  this->addChild(ep, 1);

  auto ep_fadeout = FadeOut::create(0.8f);
  ep->runAction(ep_fadeout);

  this->schedule(SEL_SCHEDULE(&single_play_scene::update_timer), 1/10);
  this->schedule(SEL_SCHEDULE(&single_play_scene::check_tmp_timer), 1);
}

void single_play_scene::create_timer() {

  CCSprite* timeBar = CCSprite::create("ui/timebar.png");
  progressTimeBar_ = CCProgressTimer::create(timeBar);

  progressTimeBar_->setPosition(Vec2(center.x, center.y + _play_screen_y/2 - _offset_y+0));
  progressTimeBar_->setScale(0.8f);
  //progressTimeBar_->setMidpoint(ccp(0, 0.5f));
  progressTimeBar_->setMidpoint(ccp(0, 1.0f));
  progressTimeBar_->setBarChangeRate(ccp(1, 0));
  progressTimeBar_->setType(kCCProgressTimerTypeBar);
  progressTimeBar_->setPercentage(100);

  this->addChild(progressTimeBar_, 2);

  // 10초 동안 게이지 100% 동안 내려옴
  //CCProgressFromTo* progressToZero = CCProgressFromTo::create(10, 100, 0);
  //progressTimeBar_->runAction(progressToZero);
 
  CCSprite* timeOutline = CCSprite::create("ui/timeoutline.png");
  timeOutline->setPosition(Vec2(center.x, center.y + _play_screen_y/2 - _offset_y+0));
  timeOutline->setScale(0.8f);
  timeOutline->setVisible(true);
  this->addChild(timeOutline, 2);
}

void single_play_scene::update_timer() {
  // call 4 times in a sec => 60초에 100%달게 할려면
  // 240번 불러야함
  float timer_sec = play_info::get().play_time_sec;

  float cPercentage = progressTimeBar_->getPercentage();
  progressTimeBar_->setPercentage(cPercentage - (100 / (60 * timer_sec)));
}

void single_play_scene::increase_timer(int sec) {
  float cPercentage = progressTimeBar_->getPercentage();
  progressTimeBar_->setPercentage(cPercentage + (60 * sec));
}

void single_play_scene::check_tmp_timer() {
  static int t = 0;
  t++;
  CCLOG("sec: %d", t);
}

void single_play_scene::menuCloseCallback(Ref* pSender) {

  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}

void single_play_scene::handle_payload(float dt) {
  Json payload = connection::get().q.front();
  connection::get().q.pop_front();
  std::string type = payload["type"].string_value();

  if(type == "connection_notify") {
    CCLOG("[debug] 접속 성공");
    connection::get().send2(Json::object {
	{ "type", "login_req" }
      });

  } else if(type == "disconnection_notify") {
    CCLOG("[debug] 접속 큰킴");

  } else {
    CCLOG("[error] handler 없음");
  }
}

void single_play_scene::check_end_play() {
  int cPercentage = progressTimeBar_->getPercentage();
  CCLOG("Percentage: %d", cPercentage);
  if(cPercentage <= 0) {
    CCLOG("Percentage below 0");
    auto single_play_scene = single_play_scene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.0f, single_play_scene, Color3B(0,255,255)));
  } 
}
