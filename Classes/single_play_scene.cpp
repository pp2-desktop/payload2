#include "SimpleAudioEngine.h"
#include "single_lobby_scene.hpp"
#include "single_play_scene.hpp"
#include "connection.hpp"
#include "single_play_info.hpp"
#include "user_info.hpp"
#include "single_lobby_scene.hpp"
#include <time.h>  

//using namespace ui;
using namespace CocosDenshion;

//#define ccsf2(...) CCString::createWithFormat(__VA_ARGS__)->getCString()

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

  auto audio = SimpleAudioEngine::getInstance();
  audio->playBackgroundMusic("sound/bg0.mp3", true);
 
  visible_size = Director::getInstance()->getVisibleSize();
  origin = Director::getInstance()->getVisibleOrigin();
  center = Vec2(visible_size.width/2 + origin.x, visible_size.height/2 + origin.y);
  center_ = center;

  is_calm_down = true;
  this->scheduleOnce(SEL_SCHEDULE(&single_play_scene::reset_calm_down), 2.0f);

  auto ui_top_bg = Sprite::create("ui/top23.png");
  ui_top_bg->setPosition(Vec2(center.x, center.y + _play_screen_y/2 - _offset_y+0));
  this->addChild(ui_top_bg, 0);

  // pause button
  pause_button = ui::Button::create();
  pause_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  pause_button->ignoreContentAdaptWithSize(false);
  pause_button->setContentSize(Size(64, 64));
  pause_button->loadTextures("ui/pause.png", "ui/pause.png");

  pause_button->setPosition(Vec2(45, center.y + _play_screen_y/2 - _offset_y));

  pause_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.2f, 1.3f);
	pause_button->runAction(scaleTo);
	//auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	//auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	//pause_button->runAction(seq2);
      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	pause_button->runAction(scaleTo2);
        start_pause();
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	pause_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(pause_button, 2);


  // find button
  find_button = ui::Button::create();
  find_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  find_button->ignoreContentAdaptWithSize(false);
  find_button->setContentSize(Size(64, 64));
  find_button->loadTextures("ui/find.png", "ui/find.png");

  find_button->setPosition(Vec2(1334-50, center.y + _play_screen_y/2 - _offset_y));

  find_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {

      if(is_paused) return;

      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.2f, 1.3f);
	auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	auto seq2 = Sequence::create(scaleTo, scaleTo2, nullptr);
	find_button->runAction(seq2);
      }
    });
     
  this->addChild(find_button, 2);
  // end



  std::string theme = play_info_md::get().playing_theme;
  auto clear_stage = play_info_md::get().user_played_infos[theme].clear_stage;
  CCLOG("clear_stage: %d", clear_stage);
  max_stage_cnt = play_info_md::get().user_played_infos[theme].max_stage_cnt;
  current_stage = clear_stage + 1;


  stage_info si = play_info_md::get().get_stage_info(theme, clear_stage);

  max_spot_cnt = si.spots.size();

  // tmp
  play_info::get().reset();
  play_info::get().img = si.img;
  play_info::get().play_time_sec = static_cast<float>(si.time);

  for(auto& v : si.spots) {
    play_info::get().add_spot_info(v.x, v.y);
  }

  auto img = play_info::get().img;

  auto left_img = Sprite::create("img/" + theme + "/left_" + img);
  left_img->setPosition(Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y));
  this->addChild(left_img, 1);

  auto right_img = Sprite::create("img/" + theme + "/right_" + img);
  right_img->setPosition(Vec2( (visible_size.width/2)+(visible_size.width/2/2) + origin.x + _offset_x, visible_size.height/2 + origin.y  - _offset_y));
  this->addChild(right_img, 1);




  auto input_listener = EventListenerTouchOneByOne::create();
  input_listener->setSwallowTouches(true);
 
  input_listener->onTouchBegan = [=](Touch* touch, Event* event) {

    if(is_calm_down || is_paused) return true;

    CCPoint touchLocation = touch->getLocationInView();
    touchLocation = cocos2d::CCDirector::sharedDirector()->convertToGL(touchLocation);
    this->check_user_input(touchLocation.x, touchLocation.y);

    return true;
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(input_listener, this);

  create_timer();
  create_stage_status();
  create_pause_menu();

  this->scheduleOnce(SEL_SCHEDULE(&single_play_scene::ready_go), 0.5f);
  this->scheduleUpdate();

  return true;
}

void single_play_scene::create_stage_status() {

  auto ui_offset_x = 70;
  auto font_size = 30;
  
  auto font_x = visible_size.width/2 + ui_offset_x;
  auto font_y = center.y + _play_screen_y/2 - _offset_y+0;
  font_y = font_y + 1;

  /*
  auto top_stage_font = Label::createWithTTF("스테이지", "fonts/nanumb.ttf", font_size);
  top_stage_font->setPosition(Vec2(font_x, font_y));
  top_stage_font->setColor( Color3B( 200, 200, 200) );
  this->addChild(top_stage_font, 1);
  */

  top_left_stage_font = Label::createWithTTF(ccsf2("%d", current_stage), "fonts/nanumb.ttf", font_size);
  top_left_stage_font->setPosition(Vec2(font_x + 80, font_y));
  top_left_stage_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(top_left_stage_font, 1);
 
  auto top_stage_slash_font = Label::createWithTTF("/", "fonts/nanumb.ttf", font_size);
  top_stage_slash_font->setPosition(Vec2(font_x + 105, font_y));
  top_stage_slash_font->setColor( Color3B( 225, 225, 225) );
  this->addChild(top_stage_slash_font, 1);

  top_right_stage_font = Label::createWithTTF(ccsf2("%d", max_stage_cnt), "fonts/nanumb.ttf", font_size);
  top_right_stage_font->setPosition(Vec2(font_x + 130, font_y));
  top_right_stage_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(top_right_stage_font, 1);
  
  /*
  auto div_font = Label::createWithTTF("|", "fonts/nanumb.ttf", font_size+10);
  div_font->setPosition(Vec2(font_x + 150, font_y));
  div_font->setColor( Color3B( 255, 255, 204) );
  this->addChild(div_font, 1);
  */

  /*
  auto top_spot_font = Label::createWithTTF("틀린그림", "fonts/nanumb.ttf", font_size);
  top_spot_font->setPosition(Vec2(font_x + 225, font_y));
  top_spot_font->setColor( Color3B( 200, 200, 200) );
  this->addChild(top_spot_font, 1);
  */

  find_spot_cnt = 0;
  top_left_spot_font = Label::createWithTTF("0", "fonts/nanumb.ttf", font_size);
  top_left_spot_font->setPosition(Vec2(font_x + 300, font_y));
  top_left_spot_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(top_left_spot_font, 1);

  auto top_spot_slash_font = Label::createWithTTF("/", "fonts/nanumb.ttf", font_size);
  top_spot_slash_font->setPosition(Vec2(font_x + 325, font_y));
  top_spot_slash_font->setColor( Color3B( 225, 225, 225) );
  this->addChild(top_spot_slash_font, 1);

  top_right_spot_font = Label::createWithTTF(ccsf2("%d", max_spot_cnt), "fonts/nanumb.ttf", font_size);
  top_right_spot_font->setPosition(Vec2(font_x + 350, font_y));
  top_right_spot_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(top_right_spot_font, 1);

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

  timeBar = CCSprite::create("ui/timebar2.png");

  // 10초 동안 게이지 100% 동안 내려옴
  //CCProgressFromTo* progressToZero = CCProgressFromTo::create(10, 100, 0);
  //progressTimeBar_->runAction(progressToZero);
 
  auto timeOutline = CCSprite::create("ui/timeoutline2.png");
  timeOutline->setPosition(Vec2(timeBar->getContentSize().width/2 + 120, center.y + _play_screen_y/2 - _offset_y-1+0));
  //timeOutline->setScale(0.8f);
  timeOutline->setScaleX(0.9f);
  timeOutline->setScaleY(0.7f);
  timeOutline->setVisible(true);
  this->addChild(timeOutline, 2);

  progressTimeBar_ = CCProgressTimer::create(timeBar);

  progressTimeBar_->setPosition(Vec2(timeBar->getContentSize().width/2 + 120, center.y + _play_screen_y/2 - _offset_y-1+4));
  //progressTimeBar_->setScale(0.8f);
  progressTimeBar_->setScaleX(0.9f);
  progressTimeBar_->setScaleY(0.7f);
  //progressTimeBar_->setMidpoint(ccp(0, 0.5f));
  progressTimeBar_->setMidpoint(ccp(0, 1.0f));
  progressTimeBar_->setBarChangeRate(ccp(1, 0));
  progressTimeBar_->setType(kCCProgressTimerTypeBar);
  progressTimeBar_->setPercentage(100);
  this->addChild(progressTimeBar_, 2);

  
  auto timer = CCSprite::create("ui/timer.png");
  timer->setPosition(Vec2(120, center.y + _play_screen_y/2 - _offset_y+0));
  timer->setScale(0.1f);
  timer->setVisible(true);
  this->addChild(timer, 2);
}

void single_play_scene::create_pause_menu() {
  pause_background = Sprite::create("ui/paused_windows.png");
  pause_background->setPosition(center_.x, center_.y);
  pause_background->setVisible(false);
  this->addChild(pause_background, 2);


  // sound_bg_on_button
  bg_sound_button = ui::Button::create();
  bg_sound_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  bg_sound_button->loadTextures("ui/sound_bg_on_button.png", "ui/sound_bg_on_button.png");
  bg_sound_button->ignoreContentAdaptWithSize(false);
  bg_sound_button->setContentSize(Size(90, 90));
  bg_sound_button->setPosition(Vec2(center_.x-60, center_.y + 130.0f));
  bg_sound_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	//auto scaleTo = ScaleTo::create(0.2f, 1.3f);
	//pause_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	//auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	//pause_button->runAction(scaleTo2);
	bg_sound_button->loadTextures("ui/sound_bg_off_button.png", "ui/sound_bg_off_button.png");
	control_sound(0);

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	//auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	//pause_button->runAction(scaleTo2);
      }
    });
     
  bg_sound_button->setVisible(false);
  this->addChild(bg_sound_button, 2);

  // effect_bg_on_button
  effect_sound_button = ui::Button::create();
  effect_sound_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  effect_sound_button->loadTextures("ui/sound_effect_on_button.png", "ui/sound_effect_on_button.png");
  effect_sound_button->ignoreContentAdaptWithSize(false);
  effect_sound_button->setContentSize(Size(100, 100));
  effect_sound_button->setPosition(Vec2(center_.x+60, center_.y + 130.0f));
  effect_sound_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	//auto scaleTo = ScaleTo::create(0.2f, 1.3f);
	//pause_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	//auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	//pause_button->runAction(scaleTo2);
	
	effect_sound_button->loadTextures("ui/sound_effect_off_button.png", "ui/sound_effect_off_button.png");
	control_sound(1);

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	//auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	//pause_button->runAction(scaleTo2);
      }
    });
     
  effect_sound_button->setVisible(false);
  this->addChild(effect_sound_button, 2);


  // resume
  resume_button = ui::Button::create();
  resume_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  resume_button->loadTextures("ui/resume_button.png", "ui/resume_button.png");
  resume_button->ignoreContentAdaptWithSize(false);
  //resume_button->setContentSize(Size(64, 64));
  resume_button->setPosition(Vec2(center_.x, center_.y+20.0f));
  resume_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.2f, 1.3f);
	resume_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	resume_button->runAction(scaleTo2);
	start_resume();

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	resume_button->runAction(scaleTo2);
      }
    });
     
  resume_button->setVisible(false);
  this->addChild(resume_button, 2);



  // restart
  restart_button = ui::Button::create();
  restart_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  restart_button->loadTextures("ui/restart2_button.png", "ui/restart2_button.png");
  restart_button->ignoreContentAdaptWithSize(false);
  //resume_button->setContentSize(Size(64, 64));
  restart_button->setPosition(Vec2(center_.x, center_.y - resume_button->getContentSize().height + 5.0f));
  restart_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	//auto scaleTo = ScaleTo::create(0.2f, 1.3f);
	//pause_button->runAction(scaleTo);
	start_resume();
      } else if(type == ui::Widget::TouchEventType::ENDED) {
	//auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	//pause_button->runAction(scaleTo2);

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	//auto scaleTo2 = ScaleTo::create(0.2f, 1.0f);
	//pause_button->runAction(scaleTo2);
      }
    });
     
  restart_button->setVisible(false);
  this->addChild(restart_button, 2);


  // exit
  exit_button = ui::Button::create();
  exit_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  exit_button->loadTextures("ui/exit_button.png", "ui/exit_button.png");
  exit_button->ignoreContentAdaptWithSize(false);
  //resume_button->setContentSize(Size(64, 64));
  exit_button->setPosition(Vec2(center_.x, center_.y - restart_button->getContentSize().height*2.0f));
  exit_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {


      } else if(type == ui::Widget::TouchEventType::ENDED) {
	start_resume();
	auto single_lobby_scene = single_lobby_scene::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(0.0f, single_lobby_scene, Color3B(0,255,255)));
      } else if(type == ui::Widget::TouchEventType::CANCELED) {

      }
    });
     
  exit_button->setVisible(false);
  this->addChild(exit_button, 2);


}

void single_play_scene::update_timer() {
  // call 4 times in a sec => 60초에 100%달게 할려면
  // 240번 불러야함
  float timer_sec = play_info::get().play_time_sec;
  //CCLOG("playing sec: %f", timer_sec);

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
  //CCLOG("Percentage: %d", cPercentage);
  if(cPercentage <= 0) {
    //CCLOG("Percentage below 0");
    auto single_play_scene = single_play_scene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.0f, single_play_scene, Color3B(0,255,255)));
  } else if(cPercentage <= 40) {
    timeBar->setColor(Color3B(255, 40, 40));
  }

}

void single_play_scene::check_win_play() {
  for(auto& spot: play_info::get().spot_infos) {
    if(!spot.is_find) {
      return;
    }
  }

  std::string theme = play_info_md::get().playing_theme;
  CCLOG("playing theme: %s", theme.c_str());

  auto r = play_info_md::get().increase_clear_stage(theme);
  if(r == -1) {
    auto single_lobby_scene = single_lobby_scene::createScene();  
    Director::getInstance()->replaceScene(TransitionFade::create(0.0f, single_lobby_scene, Color3B(0,255,255)));
    return;
  }

  auto single_play_scene = single_play_scene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.0f, single_play_scene, Color3B(0,255,255)));
}

Vec2 single_play_scene::change_device_to_img_pos(float x, float y) {
  const auto half_width = _play_screen_x / 2;
  const auto offset_height = visible_size.height - _play_screen_y;
  // ui 영역 및 나머지 부분 터치했는지 확인 해야함
  y = y - offset_height/2;
  if(x < half_width) {
    return Vec2(x, y);
  }

  x = x - half_width - _offset_x;
  return Vec2(x, y);
}

void single_play_scene::check_user_input(float x, float y) {
  Vec2 img_pos = change_device_to_img_pos(x, y);
  CCLOG("x: %f", img_pos.x);
  CCLOG("y: %f", img_pos.y);
  auto index = play_info::get().check_spot_info(img_pos.x, img_pos.y);
  if(index > -1) {
    CCLOG("맞춤");
    action_correct(index);
  } else {
    CCLOG("틀림");
    action_incorrect(x, y);
  }
}

void single_play_scene::action_correct(int index) {

  //Vec2 left_pos = ;
  //Vec2 right_pos = ;
  auto audio = SimpleAudioEngine::getInstance();

  srand(time(NULL));
  auto r = rand() % 4;
  if(r == 0) {
    audio->playEffect("sound/great.wav", false, 1.0f, 1.0f, 1.0f);
  } else if(r==1) {
    audio->playEffect("sound/good.wav", false, 1.0f, 1.0f, 1.0f);
  } else if(r==2) {
    audio->playEffect("sound/cool.wav", false, 1.0f, 1.0f, 1.0f);
  } else {
    audio->playEffect("sound/yeah.wav", false, 1.0f, 1.0f, 1.0f);
  }
  


  auto si = play_info::get().get_spot_info(index);
  if(!si.is_find) {
    CCLOG("[error] spot is_find update안됨");    
  }


  auto circle_animation = Animation::create();
  circle_animation->setDelayPerUnit(0.1f);
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle0.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle1.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle2.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle3.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle4.png");

  Vec2 left_pos = change_img_to_device_pos(true, si.pos.x, si.pos.y);
  auto left_spot = CCSprite::create("animation/corrects/correct7.png");
  left_spot->setPosition(Vec2(left_pos.x, left_pos.y));
  left_spot->setScale(0.5f);

  left_spot->runAction(Animate::create(circle_animation));
  this->addChild(left_spot, 1);


  Vec2 right_pos = change_img_to_device_pos(false, si.pos.x, si.pos.y);
  auto right_spot = CCSprite::create("animation/corrects/correct7.png");
  right_spot->setPosition(Vec2(right_pos.x, right_pos.y));
  right_spot->setScale(0.5f);

  right_spot->runAction(Animate::create(circle_animation));
  this->addChild(right_spot, 1);

  // 상황판 업데이트
  find_spot_cnt++;
  top_left_spot_font->setString(ccsf2("%d", find_spot_cnt));

  check_win_play();
}


void single_play_scene::action_incorrect(float x, float y) {

  is_calm_down = true;
  this->scheduleOnce(SEL_SCHEDULE(&single_play_scene::reset_calm_down), 0.5f);

  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/oh_no.wav", false, 1.0f, 1.0f, 1.0f);

  auto incorrect = Sprite::create("ui/incorrect.png");
  incorrect->setPosition(Vec2(center.x, center.y));
  this->addChild(incorrect, 4);

  auto fadeIn = FadeIn::create(0.2f);
  auto fadeOut = FadeOut::create(0.4f);

  auto seq = Sequence::create(fadeIn, fadeOut, nullptr);
  incorrect->runAction(seq);


  auto progress = progressTimeBar_->getPercentage();
  progressTimeBar_->setPercentage(progress - 20);
}

void single_play_scene::reset_calm_down() {
  is_calm_down = false;
};

Vec2 single_play_scene::change_img_to_device_pos(bool is_left, float x, float y) {
  const auto half_width = _play_screen_x / 2;
  const auto offset_height = (visible_size.height - _play_screen_y) / 2;
  y = y + offset_height;

  if(is_left) {
    return Vec2(x, y);
  }
  x = x + half_width + _offset_x;
  return Vec2(x, y);
}

void single_play_scene::start_pause() {
  is_paused = true;

  pause_background->setVisible(true);
  bg_sound_button->setVisible(true);
  effect_sound_button->setVisible(true);
  resume_button->setVisible(true);
  restart_button->setVisible(true);
  exit_button->setVisible(true);

  Director::getInstance()->pause();

  auto audio = SimpleAudioEngine::getInstance();
  audio->pauseBackgroundMusic();
  //audio->pauseEffect();
  audio->pauseAllEffects();
}

void single_play_scene::start_resume() {

  pause_background->setVisible(false);
  bg_sound_button->setVisible(false);
  effect_sound_button->setVisible(false);
  resume_button->setVisible(false);
  restart_button->setVisible(false);
  exit_button->setVisible(false);

  Director::getInstance()->resume(); 

  auto audio = SimpleAudioEngine::getInstance();
  audio->resumeBackgroundMusic();
  audio->resumeAllEffects();
  is_paused = false;
}

void single_play_scene::control_sound(int type) {

  auto audio = SimpleAudioEngine::getInstance();
  
  if(type == 0) {
    audio->stopBackgroundMusic();
    audio->setBackgroundMusicVolume(0.0f);
  } else if(type == 1) {
    audio->stopAllEffects();
    audio->setEffectsVolume(0.0f);
  } else if(type == 2) {
    audio->resumeBackgroundMusic();
    audio->setBackgroundMusicVolume(1.0f);
  } else {
    audio->resumeAllEffects();
    audio->setEffectsVolume(1.0f);
  }
  
}
