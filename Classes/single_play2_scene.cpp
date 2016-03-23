#include "SimpleAudioEngine.h"
#include "single_play2_scene.hpp"
#include "lobby_scene.hpp"
#include "connection.hpp"
#include "single_play_info.hpp"
#include "user_info.hpp"
#include <chrono>

using namespace CocosDenshion;

Scene* single_play2_scene::createScene() {

  auto scene = Scene::create();
  auto layer = single_play2_scene::create();

  scene->addChild(layer);

  return scene;
}

// on "init" you need to initialize your instance
bool single_play2_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }
    
  visible_size = Director::getInstance()->getVisibleSize();
  origin = Director::getInstance()->getVisibleOrigin();
  center = Vec2(visible_size.width/2 + origin.x, visible_size.height/2 + origin.y);

  stage_cnt = play_info_md::get().single_play2_info_.get_stage_cnt();
  max_stage_cnt = 0;
  point_cnt = 0;
  max_point_cnt = 0;

  is_playing = false;
  is_hint_on = false;
  is_pause = false;
  is_store_on = true;
  is_iap_on = false;
  //this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::set_is_store_on_false), 2.0f);

  is_hurry_up = false;

  create_stage_status_cnt = 0;
 
   connection::get().send2(Json::object {
     { "type", "single_img_info_req" },
     { "stage_count", stage_cnt }
    });

   connection::get().send2(Json::object {
     { "type", "max_stage_req" },
    });
 
  create_ui_top();
  create_ui_timer();
  //create_stage_status();

  create_block();
  create_status_font();

  create_pause_popup();
  create_game_end_popup();
  create_connection_popup();
  create_complete_popup();
  create_store_popup();
  create_iap_popup();
  
  auto input_listener = EventListenerTouchOneByOne::create();
  input_listener->setSwallowTouches(true);
 
  input_listener->onTouchBegan = [=](Touch* touch, Event* event) {
    if(!is_playing || is_incorrect_action || is_pause || is_store_on) return false;

    CCPoint touchLocation = touch->getLocationInView();
    touchLocation = cocos2d::CCDirector::sharedDirector()->convertToGL(touchLocation);
    if(touchLocation.y > center.y + 301) {
      CCLOG("top ui 영역 터치");
      return false;
    }
    this->check_user_input(touchLocation.x, touchLocation.y);

    return true;
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(input_listener, this);

#if (CC_TARGET_PLATFORM != CC_PLATFORM_LINUX) 
  //sdkbox::IAP::setDebug(false);
  sdkbox::IAP::setListener(this);
  sdkbox::IAP::init();

  sdkbox::PluginAdColony::setListener(this);
  sdkbox::PluginAdColony::init();
#endif

  this->scheduleUpdate();


  return true;
}

void single_play2_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  }
  
  if(is_playing) {
    check_end_play();
  }
}

void single_play2_scene::handle_payload(float dt) {
    Json payload = connection::get().q.front();
    connection::get().q.pop_front();
    std::string type = payload["type"].string_value();

    if(type == "connection_notify") {
      CCLOG("[debug] 접속 성공");
     
      /*
      connection::get().send2(Json::object {
	  { "type", "login_req" }
	});
      */
      
    } else if(type == "disconnection_notify") {
      CCLOG("[debug] 접속 큰킴");
      open_connection_popup();

      
    } else if(type == "single_img_info_res") {
      game_stage.img = payload["img"].string_value();

      std::deque<float> points;
      for (auto &k : payload["points"].array_items()) {
	points.push_back(static_cast<float>(k.int_value()));
      }

      auto size = points.size();
      for(unsigned i=0; i<size;) {
	auto x = points.front();
	points.pop_front();

	auto y = points.front();
	points.pop_front();

	game_stage.hidden_points.push_back(Vec2(x, y));
	i=i+2;
	max_point_cnt++;
      }

      create_stage_status_cnt++;
      if(create_stage_status_cnt > 1) {
	create_stage_status();
      }

      loading_stage(game_stage.img);
    } else if(type == "max_stage_res") {
      create_stage_status_cnt++;

      max_stage_cnt = payload["max_stage_count"].int_value();

      create_stage_status_cnt++;
      if(create_stage_status_cnt > 1) { 
	create_stage_status();
      }
      //max_stage_cnt_font->setString(ccsf2("%d", max_stage_cnt));
      play_info_md::get().single_play2_info_.set_max_stage_cnt(max_stage_cnt);
    } else {
      CCLOG("[error] handler 없음");
    }
}

void single_play2_scene::create_ui_top() {
  auto ui_top_bg = Sprite::create("ui/single_play24.png");
  ui_top_bg->setPosition(Vec2(center.x, center.y + _play_screen_y/2 - _offset_y+0));
  this->addChild(ui_top_bg, 0);

  // pause button
  pause_button = ui::Button::create();
  pause_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  pause_button->ignoreContentAdaptWithSize(false);
  pause_button->setContentSize(Size(128, 128));
  pause_button->setScale(0.5f);
  pause_button->loadTextures("ui/pause.png", "ui/pause.png");

  pause_button->setPosition(Vec2(40, center.y + _play_screen_y/2 - _offset_y));

  pause_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {

	if(img_complete_cnt <= 1 || !is_playing || is_pause || is_store_on) {
	  return;
	}

        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 0.6f);
	pause_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {

	if(img_complete_cnt <= 1 || !is_playing || is_pause || is_store_on) {
	  return;
	}

	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	pause_button->runAction(scaleTo2);

	open_pause_popup();

        //start_pause();
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	pause_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(pause_button, 0);

  // hint button
  hint_button = ui::Button::create();
  hint_button->setTouchEnabled(true);
  //pause_button->setScale(1.0f);
  hint_button->ignoreContentAdaptWithSize(false);
  hint_button->setContentSize(Size(128, 128));
  hint_button->setScale(0.5f);
  hint_button->loadTextures("ui/hint.png", "ui/hint.png");

  hint_button->setPosition(Vec2(1142, center.y + _play_screen_y/2 - _offset_y));

  hint_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	if(!is_playing || is_pause || is_store_on) return;
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 0.6f);
	hint_button->runAction(scaleTo);


      } else if(type == ui::Widget::TouchEventType::ENDED) {
	if(!is_playing || is_pause || is_store_on) return;

	auto r = user_info::get().item_info_.use_hint();
	if(r) {
	  for(auto i=0; i<game_stage.hidden_points.size(); i++) {
	    const auto is_in = game_stage.found_indexs.find(i) != game_stage.found_indexs.end();

	    const auto is_in2 = hint_indexs.find(i) != hint_indexs.end();
	    if(!is_in && !is_in2) {
	      CCLOG("use hint");
	      //auto img_pos = game_stage.hidden_points[i];
	      action_hint(game_stage.hidden_points[i]);
	      hint_indexs.insert(i);
	      break;
	    }
	  }

	} else {
	  CCLOG("no more hint");
	}

	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	hint_button->runAction(scaleTo2);

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	hint_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(hint_button, 0);

  if(user_info::get().item_info_.get_hint_count() <= 0) {
    hint_button->setEnabled(false);
    hint_button->setBright(false);
  }

  auto font_size = 35;
  if(user_info::get().item_info_.get_hint_count() > 99) {
    font_size = 30;
  }

  hint_status_font = Label::createWithTTF(ccsf2("x %d", user_info::get().item_info_.get_hint_count()), "fonts/nanumb.ttf", font_size);
  hint_status_font->setPosition(Vec2(hint_button->getPosition().x + (hint_button->getContentSize().width / 2) + (hint_status_font->getContentSize().width/2.0f) - 15.0f, center.y + _play_screen_y/2 - _offset_y));
  hint_status_font->setColor( Color3B( 255, 255, 255) );
  //hint_status_font->setAnchorPoint(ccp(0,0.5f)); 
  hint_status_font->setAnchorPoint(ccp(0.5f,0.5f));
  this->addChild(hint_status_font, 0);

 // add hint button
  add_hint_button = ui::Button::create();
  add_hint_button->setTouchEnabled(true);
  add_hint_button->ignoreContentAdaptWithSize(false);
  add_hint_button->setContentSize(Size(128, 128));
  add_hint_button->setScale(0.5f);
  add_hint_button->loadTextures("ui/add_hint.png", "ui/add_hint.png");

  auto offset = 5.0f;
  if(user_info::get().item_info_.get_hint_count() < 10) {
    offset = 18.0f;
  }


  add_hint_button->setPosition(Vec2(hint_status_font->getPosition().x + hint_status_font->getContentSize().width + offset, center.y + _play_screen_y/2 - _offset_y));

  add_hint_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	if(is_store_on) return;
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 0.6f);
	add_hint_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	if(is_store_on) return;
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	add_hint_button->runAction(scaleTo2);
	open_store_popup();

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	add_hint_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(add_hint_button, 0);
  
}

void single_play2_scene::create_ui_timer() {

  timeBar = CCSprite::create("ui/timebar2.png");

  // 10초 동안 게이지 100% 동안 내려옴
  //CCProgressFromTo* progressToZero = CCProgressFromTo::create(10, 100, 0);
  //progressTimeBar_->runAction(progressToZero);
 
  auto timeOutline = CCSprite::create("ui/timeoutline2.png");
  timeOutline->setPosition(Vec2(timeBar->getContentSize().width/2 + 55, center.y + _play_screen_y/2 - _offset_y-1+0));

  timeOutline->setScaleX(0.65f);
  timeOutline->setScaleY(0.7f);
  timeOutline->setVisible(true);
  this->addChild(timeOutline, 0);

  progressTimeBar_ = CCProgressTimer::create(timeBar);

  progressTimeBar_->setPosition(Vec2(timeBar->getContentSize().width/2 + 55, center.y + _play_screen_y/2 - _offset_y-1+4));
  progressTimeBar_->setScaleX(0.65f);
  progressTimeBar_->setScaleY(0.7f);
  progressTimeBar_->setMidpoint(ccp(0, 1.0f));
  progressTimeBar_->setBarChangeRate(ccp(1, 0));
  progressTimeBar_->setType(kCCProgressTimerTypeBar);
  progressTimeBar_->setPercentage(100);
  this->addChild(progressTimeBar_, 0);
  
  auto timer = CCSprite::create("ui/timer.png");
  timer->setPosition(Vec2(120, center.y + _play_screen_y/2 - _offset_y+0));
  timer->setScale(0.1f);
  timer->setVisible(true);
  this->addChild(timer, 0);
}

void single_play2_scene::update_timer() {
  if(!is_playing || is_pause || is_store_on) return;
  // call 4 times in a sec => 60초에 100%달게 할려면
  // 240번 불러야함
  float timer_sec = 45;
  float cPercentage = progressTimeBar_->getPercentage();
  progressTimeBar_->setPercentage(cPercentage - (100 / (60 * timer_sec)));
  
}

void single_play2_scene::increase_timer(int percentage) {
  if(!is_playing || is_pause) return;
  float cPercentage = progressTimeBar_->getPercentage();
  progressTimeBar_->setPercentage(cPercentage + percentage);
  //progressTimeBar_->setPercentage(cPercentage + ((60 * sec)));
}

void single_play2_scene::create_ready(float move_to_sec, float offset, std::string img) {
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

void single_play2_scene::ready_go() {

  this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::create_go), 1.8f);

  auto offset = 160.0f;
  create_ready(0.4f, 0.0f, "ui/Y.png");
  create_ready(0.3f, offset, "ui/D.png");
  create_ready(0.2f, offset*2, "ui/A.png");
  create_ready(0.1f, offset*3, "ui/E.png");
  create_ready(0.0f, offset*4, "ui/R.png");

  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/Ready_1.wav");
}

void single_play2_scene::create_go() {

  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/Go.wav");
 
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

  this->schedule(SEL_SCHEDULE(&single_play2_scene::update_timer), 1/10);
  this->schedule(SEL_SCHEDULE(&single_play2_scene::clear_hint), 5.0f);
  is_playing = true;
  is_store_on = false;
}


void single_play2_scene::create_block() {
// 화면 가릴것 2개 로딩하기
  left_block = Sprite::create("ui/hide1.png");
  left_block->setPosition(Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y));
  this->addChild(left_block, 2);

  right_block = Sprite::create("ui/hide1.png");
  right_block->setPosition(Vec2((visible_size.width/2)+(visible_size.width/2/2) + origin.x + _offset_x, visible_size.height/2 + origin.y  - _offset_y));
  this->addChild(right_block, 2);
}

void single_play2_scene::open_block() {
  auto lx = left_block->getPosition().x - left_block->getContentSize().width;
  auto rx = right_block->getPosition().x + right_block->getContentSize().width;

  auto moveTo = MoveTo::create(0.8f, Vec2(lx, left_block->getPosition().y)); 
  left_block->runAction(moveTo);

  auto moveTo2 = MoveTo::create(0.8f, Vec2(rx, right_block->getPosition().y)); 
  right_block->runAction(moveTo2);
}

void single_play2_scene::close_block() {
  auto moveTo = MoveTo::create(0.8f, Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y)); 
  left_block->runAction(moveTo);

  auto moveTo2 = MoveTo::create(0.8f, Vec2((visible_size.width/2)+(visible_size.width/2/2) + origin.x + _offset_x, visible_size.height/2 + origin.y  - _offset_y)); 
  right_block->runAction(moveTo2);
}

void single_play2_scene::start_get_img(bool is_left, std::string img) {
  cocos2d::network::HttpRequest* request = new (std::nothrow) cocos2d::network::HttpRequest();

  if(is_left) {
    string url = "https://s3.ap-northeast-2.amazonaws.com/payload2/" + img + "_left.jpg";
    request->setUrl(url.c_str());
    request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
    request->setResponseCallback(CC_CALLBACK_2(single_play2_scene::on_request_left_img_completed, this));
    request->setTag("get_left_img");
  } else {
    string url = "https://s3.ap-northeast-2.amazonaws.com/payload2/" + img + "_right.jpg";
    request->setUrl(url.c_str());
    request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
    request->setResponseCallback(CC_CALLBACK_2(single_play2_scene::on_request_right_img_completed, this));
    request->setTag("get_right_img");
  }

  cocos2d::network::HttpClient::getInstance()->send(request);
  request->release();
}

void single_play2_scene::on_request_left_img_completed(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response) {

  if(!response) {
    open_connection_popup();
    return;
  }

  if(!response->isSucceed()) {
    open_connection_popup();
    return;
  }

  std::vector<char>* buffer = response->getResponseData();

  Image* image = new Image();
  image->initWithImageData ( reinterpret_cast<const unsigned char*>(&(buffer->front())), buffer->size());
  left_texture.initWithImage(image);

  left_img = Sprite::createWithTexture(&left_texture);
  left_img->setPosition(Vec2((visible_size.width/2)/2 + origin.x - _offset_x, visible_size.height/2 + origin.y - _offset_y));
  this->addChild(left_img, 0);
  delete image;

  img_complete_cnt++;
  if(img_complete_cnt > 1) {
    start_game();
  }
}

void single_play2_scene::on_request_right_img_completed(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response) {

  if(!response) {
    open_connection_popup();
    return;
  }

  if(!response->isSucceed()) {
    open_connection_popup();
    return;
  }

  std::vector<char>* buffer = response->getResponseData();

  Image* image = new Image();
  image->initWithImageData ( reinterpret_cast<const unsigned char*>(&(buffer->front())), buffer->size());

  right_texture.initWithImage(image);

  right_img = Sprite::createWithTexture(&right_texture);
  right_img->setPosition(Vec2( (visible_size.width/2)+(visible_size.width/2/2) + origin.x + _offset_x, visible_size.height/2 + origin.y  - _offset_y));
  this->addChild(right_img, 0);
  delete image;

  img_complete_cnt++;
  if(img_complete_cnt > 1) {
    start_game();
  }

}

void single_play2_scene::create_status_font() {
  resource_status_font = Label::createWithTTF("이미지 다운로드 중", "fonts/nanumb.ttf", 50);
  resource_status_font->setPosition(Vec2(center.x, center.y));
  resource_status_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(resource_status_font, 3);

  auto scaleTo = ScaleTo::create(1.1f, 1.1f);
  resource_status_font->runAction(scaleTo);
  auto delay = DelayTime::create(0.25f);
  auto scaleTo2 = ScaleTo::create(1.0f, 1.0f);
  auto seq = Sequence::create(scaleTo, delay, scaleTo2,
			      delay->clone(), nullptr);
  resource_status_font->runAction(RepeatForever::create(seq));
}

void single_play2_scene::loading_stage(std::string img) {
  CCLOG("img: %s", img.c_str());
  img_complete_cnt = 0;
  start_get_img(true, img);
  start_get_img(false, img);
}

void single_play2_scene::destroy_stage() {
  this->removeChild(left_img);
  this->removeChild(right_img);

  for(auto& sprite : correct_spots) {
    this->removeChild(sprite);
  }
}

void single_play2_scene::start_game() {
  resource_status_font->setPosition(Vec2(center.x+5000.0f, center.y));
  open_block();
  this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::ready_go), 0.5f);
}

void single_play2_scene::end_game() {
  close_block();
  open_game_end_popup();
  //destroy_stage();
}

void single_play2_scene::win_game() {
  close_block();
  ///destroy_stage()

  bool r = play_info_md::get().single_play2_info_.increase_stage_cnt();
  if(r) {
    CCLOG("replace");
    this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::replace_single_play2_scene), 1.0f);
  } else {
    // end of stage
    this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::complete_stages), 1.0f);
  }
}

void single_play2_scene::complete_stages() {
  open_complete_popup();
}

void single_play2_scene::check_end_play() {
  int cPercentage = progressTimeBar_->getPercentage();
  //CCLOG("Percentage: %d", cPercentage);
  if(cPercentage <= 0 && (!is_end_play)) {
    progressTimeBar_->setPercentage(0);
    is_end_play = true;
    is_playing = false;

    this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::end_game), 1.2f);

    auto audio = SimpleAudioEngine::getInstance();
    audio->playEffect("sound/YouFailed.wav");

    auto youfail = Sprite::create("ui/youfail.png");
    youfail->setScale(2.0f);
    youfail->setPosition(Vec2(visible_size.width + 100.0f, center.y));
    this->addChild(youfail, 0);

    auto moveTo = MoveTo::create(0.8f, Vec2(center.x, center.y));
    auto fadeOut = FadeOut::create(1.2f);
    auto seq = Sequence::create(moveTo, fadeOut, nullptr);
    youfail->runAction(seq);


    // 게임 끝날경우 다시하기, 돌아가기 물어볼것
    /*
    auto single_play_scene = single_play_scene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.0f, single_play_scene, Color3B(0,255,255)));
    */
  } else if(cPercentage <= 40) {
    if(!is_hurry_up) {
      auto audio = SimpleAudioEngine::getInstance();
      audio->playEffect("sound/HurryUp_2.wav");
      is_hurry_up = true;
    }
    timeBar->setColor(Color3B(255, 0, 0));
  } else {
    timeBar->setColor(Color3B(255, 255, 255));
  }
}

void single_play2_scene::check_user_input(float x, float y) {
  Vec2 img_pos = change_device_to_img_pos(x, y);
  CCLOG("x: %f", img_pos.x);
  CCLOG("y: %f", img_pos.y);

  auto index = check_point(img_pos.x, img_pos.y);

  if(index >= 0) {
    CCLOG("맞춤");
    increase_timer(10);
    action_correct(game_stage.hidden_points[index]);

    game_stage.found_indexs.insert(index);

    if(game_stage.hidden_points.size() <= game_stage.found_indexs.size()) {

      is_playing = false;

      this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::action_win_game), 0.8f);
      this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::win_game), 1.8f);
    }

  } else {
    CCLOG("틀림");
    action_incorrect(x, y);
  }
}

int single_play2_scene::check_point(float x, float y) {

  auto index = 0;
  for(auto& point : game_stage.hidden_points) {
    if(is_point_in_area(x, y, point.x, point.y)) {
      if(is_found_point(index, game_stage)) {
        CCLOG("이미 찾은 포인트라서 false 처리");
        return -1;
      }
      return index;
    }
    index++;
  }

  return -1;
}

bool single_play2_scene::is_point_in_area(float ux, float uy, float xc, float yc, float r) {
   return ((ux-xc)*(ux-xc) + (uy-yc)*(uy-yc)) < r*r;
}

void single_play2_scene::check_point_req(int index) {
  Vec2 point = game_stage.hidden_points[index];
  int x = static_cast<int>(point.x);
  int y = static_cast<int>(point.y);

}

void single_play2_scene::action_correct(Vec2 point) {
 auto audio = SimpleAudioEngine::getInstance();
  srand(time(NULL));
  auto r = rand() % 4;
  if(r == 0) {
    audio->playEffect("sound/great.wav");
  } else if(r==1) {
    audio->playEffect("sound/good.wav");
  } else if(r==2) {
    audio->playEffect("sound/cool.wav");
  } else {
    audio->playEffect("sound/yeah.wav");
  }

  auto circle_animation = Animation::create();
  circle_animation->setDelayPerUnit(0.1f);
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle0.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle1.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle2.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle3.png");
  circle_animation->addSpriteFrameWithFileName("animation/corrects/circle4.png");

  Vec2 left_pos = change_img_to_device_pos(true, point.x, point.y);
  auto left_spot = CCSprite::create("animation/corrects/circle0.png");
  left_spot->setPosition(Vec2(left_pos.x, left_pos.y));
  left_spot->setScale(0.5f);

  left_spot->runAction(Animate::create(circle_animation));
  this->addChild(left_spot, 0);

  Vec2 right_pos = change_img_to_device_pos(false, point.x, point.y);
  auto right_spot = CCSprite::create("animation/corrects/circle0.png");
  right_spot->setPosition(Vec2(right_pos.x, right_pos.y));
  right_spot->setScale(0.5f);

  right_spot->runAction(Animate::create(circle_animation));
  this->addChild(right_spot, 0);

  correct_spots.push_back(left_spot);
  correct_spots.push_back(right_spot);

  point_cnt_font->setString(ccsf2("%d", ++point_cnt));
}

void single_play2_scene::action_incorrect(float x, float y) {
  is_incorrect_action = true;
  this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::release_incorrect_action), 0.75f);
  
  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/incorrect2.wav");

  auto incorrect2 = Sprite::create("ui/incorrect2.png");
  incorrect2->setScale(0.5f);
  incorrect2->setPosition(Vec2(x,y));
  this->addChild(incorrect2);
  auto fadeOut = FadeOut::create(0.75f);
  incorrect2->runAction(fadeOut);
  
  auto moveBy0 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy1 = MoveBy::create(0.05f, Vec2(20, 0));
  auto moveBy2 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy3 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy4 = MoveBy::create(0.05f, Vec2(20, 0));
  auto moveBy5 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy6 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy7 = MoveBy::create(0.05f, Vec2(20, 0));
  auto moveBy8 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy9 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy10 = MoveBy::create(0.05f, Vec2(20, 0));
  auto moveBy11 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy12 = MoveBy::create(0.05f, Vec2(-10, 0));
  auto moveBy13 = MoveBy::create(0.05f, Vec2(20, 0));
  auto moveBy14 = MoveBy::create(0.05f, Vec2(-10, 0));

  auto seq = Sequence::create(moveBy0, moveBy1, moveBy2, moveBy3, moveBy4, moveBy5, moveBy6, moveBy7, moveBy8, moveBy9, moveBy10, moveBy11, moveBy12, moveBy13, moveBy14, nullptr);

  if(x < visible_size.width / 2.0f) {
    left_img->runAction(seq);
  } else {
    right_img->runAction(seq);
  }

  auto progress = progressTimeBar_->getPercentage();
  progressTimeBar_->setPercentage(progress - 10);
}

void single_play2_scene::action_hint(Vec2 point) {
  hint_status_font->setString(ccsf2("x %d", user_info::get().item_info_.get_hint_count()));

  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/hint2.wav");
  
  Vec2 pos = change_img_to_device_pos(true, point.x, point.y);

  srand(time(NULL));
  auto r = rand() % 2;
  if(r % 2) {
    pos = change_img_to_device_pos(false, point.x, point.y);
  }

  auto hint_spot = Sprite::create("ui/hint_circle2.png");
  hint_spot->setScale(0.12f);
  hint_spot->setPosition(Vec2(pos.x, pos.y));

  auto scaleTo = ScaleTo::create(0.15f, 0.15f);
  auto scaleTo2 = ScaleTo::create(0.12f, 0.12f);
  auto scaleTo3 = ScaleTo::create(0.15f, 0.15f);
  auto scaleTo4 = ScaleTo::create(0.12f, 0.12f);
  auto scaleTo5 = ScaleTo::create(0.15f, 0.15f);
  auto scaleTo6 = ScaleTo::create(0.12f, 0.12f);
  auto scaleTo7 = ScaleTo::create(0.15f, 0.15f);
  auto scaleTo8 = ScaleTo::create(0.12f, 0.12f);
  auto fadeOut = FadeOut::create(1.0f);

  auto delay = DelayTime::create(0.2f);
  auto seq = Sequence::create(scaleTo, delay, scaleTo2, delay, scaleTo3, delay, scaleTo4, delay, scaleTo5, delay, scaleTo6, delay, scaleTo7, delay, scaleTo8, fadeOut, nullptr);
  hint_spot->runAction(seq);
  this->addChild(hint_spot, 0);

  if(user_info::get().item_info_.get_hint_count() <= 0) {
    hint_button->setEnabled(false);
    hint_button->setBright(false);
  }

}

void single_play2_scene::release_incorrect_action() {
  is_incorrect_action = false;
}

bool single_play2_scene::is_found_point(int index, stage& game_stage) {

  for(auto i : game_stage.found_indexs) {
    CCLOG("contained index: %d", i);
  }

  auto it = game_stage.found_indexs.find(index);
  if(it != game_stage.found_indexs.end()) return true;

  return false;
}

Vec2 single_play2_scene::change_img_to_device_pos(bool is_left, float x, float y) {
  const auto half_width = _play_screen_x / 2;
  const auto offset_height = (visible_size.height - _play_screen_y) / 2;
  y = y + offset_height;

  if(is_left) {
    return Vec2(x, y);
  }
  x = x + half_width + _offset_x;
  return Vec2(x, y);
}

Vec2 single_play2_scene::change_device_to_img_pos(float x, float y) {
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

void single_play2_scene::replace_single_play2_scene() {
  destroy_stage();
  auto single_play2_scene = single_play2_scene::createScene();
  Director::getInstance()->replaceScene(single_play2_scene);
}

void single_play2_scene::replace_lobby_scene() {
  destroy_stage();
  auto lobby_scene = lobby_scene::createScene();
  Director::getInstance()->replaceScene(lobby_scene);
}

void single_play2_scene::create_connection_popup() {
  auto offset = 5000.0f;
  connection_background_popup = Sprite::create("ui/background_popup.png");
  connection_background_popup->setScale(2.0f);
  connection_background_popup->setPosition(Vec2(center.x + offset, center.y));
  this->addChild(connection_background_popup, 2);

  connection_noti_font = Label::createWithTTF("네트워크 불안정 상태로 서버와 접속 끊김.", "fonts/nanumb.ttf", 40);
  connection_noti_font->setPosition(Vec2(center.x + offset, center.y));
  connection_noti_font->setColor(Color3B( 110, 110, 110));
  this->addChild(connection_noti_font, 2);

  connection_confirm_button = ui::Button::create();
  connection_confirm_button->setTouchEnabled(true);
  connection_confirm_button->ignoreContentAdaptWithSize(false);
  connection_confirm_button->setContentSize(Size(286.0f, 126.0f));
  connection_confirm_button->loadTextures("ui/confirm_button.png", "ui/confirm_button.png");
  connection_confirm_button->setPosition(Vec2(center.x + offset, center.y));

  connection_confirm_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 1.1f);
        connection_confirm_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
        connection_confirm_button->runAction(scaleTo2);
        if(!connection::get().get_is_connected()) {
          connection::get().create("ws://t.05day.com:8080/echo");
          connection::get().connect();
        }
        auto lobby_scene = lobby_scene::createScene();
        Director::getInstance()->replaceScene(lobby_scene);
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 1.0f);
        connection_confirm_button->runAction(scaleTo);
      }
    });
     
  this->addChild(connection_confirm_button, 2);
}

void single_play2_scene::open_connection_popup() {
  is_playing = false;
  if(is_pause) close_pause_popup();
  close_game_end_popup();
  close_complete_popup();
  resource_status_font->setPosition(Vec2(center.x+5000.0f, center.y));

  connection_background_popup->setPosition(Vec2(center));
  connection_noti_font->setPosition(Vec2(center.x, center.y + 60.0f));
  connection_confirm_button->setPosition(Vec2(center.x, center.y - 100.0f));
}

void single_play2_scene::close_connection_popup() {
  auto offset = 5000.0f;
  connection_background_popup->setPosition(Vec2(center.x + offset, center.y));
  connection_noti_font->setPosition(Vec2(center.x + offset, center.y + 60.0f));
  connection_confirm_button->setPosition(Vec2(center.x + offset, center.y - 100.0f));
}

void single_play2_scene::create_pause_popup() {
  pause_background = Sprite::create("ui/paused_windows.png");
  pause_background->setPosition(center.x, center.y);
  pause_background->setVisible(false);
  this->addChild(pause_background, 2);

  // resume
  resume_button = ui::Button::create();
  resume_button->setTouchEnabled(true);
  resume_button->ignoreContentAdaptWithSize(false);
  resume_button->setContentSize(Size(286.0f, 126.0f));
  resume_button->loadTextures("ui/resume2_button.png", "ui/resume2_button.png");
  resume_button->setScale(0.8f);
  resume_button->setPosition(Vec2(center.x, center.y + 50));
  resume_button->setVisible(false);

  resume_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	if(is_store_on) return;
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 0.95f);
	resume_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	if(is_store_on) return;
	auto scaleTo2 = ScaleTo::create(0.1f, 0.8f);
	resume_button->runAction(scaleTo2);
	close_pause_popup();

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	resume_button->runAction(scaleTo);
      }
    });

  this->addChild(resume_button, 2);

  // back
  back_button = ui::Button::create();
  back_button->setTouchEnabled(true);
  back_button->ignoreContentAdaptWithSize(false);
  back_button->setContentSize(Size(286.0f, 126.0f));
  back_button->loadTextures("ui/back2_button.png", "ui/back2_button.png");
  back_button->setScale(0.8f);
  back_button->setPosition(Vec2(center.x, center.y - 80));
  back_button->setVisible(false);

  back_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	if(is_store_on) return;
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 0.95f);
	back_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	if(is_store_on) return;
	auto scaleTo2 = ScaleTo::create(0.1f, 0.8f);
	back_button->runAction(scaleTo2);
	replace_lobby_scene();

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	back_button->runAction(scaleTo);
      }
    });

  this->addChild(back_button, 2);
}

void single_play2_scene::open_pause_popup() {
  is_pause = true;
  close_block();
  pause_background->setVisible(true);
  resume_button->setVisible(true);
  back_button->setVisible(true);
}

void single_play2_scene::close_pause_popup() {
  open_block();
  pause_background->setVisible(false);
  resume_button->setVisible(false);
  back_button->setVisible(false);
  this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::set_is_pause_false), 1.0f);
  //is_pause = false;
}

void single_play2_scene::create_game_end_popup() {
  game_end_background = Sprite::create("ui/game_end_windows2.png");
  game_end_background->setPosition(center.x, center.y);
  game_end_background->setVisible(false);
  this->addChild(game_end_background, 2);

  // resume
  retry_button = ui::Button::create();
  retry_button->setTouchEnabled(true);
  retry_button->ignoreContentAdaptWithSize(false);
  retry_button->setContentSize(Size(286.0f, 126.0f));
  retry_button->loadTextures("ui/retry_button.png", "ui/retry_button.png");
  retry_button->setScale(0.8f);
  retry_button->setPosition(Vec2(center.x, center.y - 25));
  retry_button->setVisible(false);

  retry_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 0.95f);
	retry_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.8f);
	retry_button->runAction(scaleTo2);

	bool is_show_video = false;

	auto retry_cnt = play_info_md::get().single_play2_info_.get_retry_cnt();
	if(retry_cnt % 4 == 0) {
	  is_show_video = true;
	}

	if(is_show_video) {

#if (CC_TARGET_PLATFORM != CC_PLATFORM_LINUX) 
	  sdkbox::PluginAdColony::show("video");
#endif

	} else {
	  replace_single_play2_scene();
	}

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	retry_button->runAction(scaleTo);
      }
    });

  this->addChild(retry_button, 2);

  // back
  /*
  back_button = ui::Button::create();
  back_button->setTouchEnabled(true);
  back_button->ignoreContentAdaptWithSize(false);
  back_button->setContentSize(Size(286.0f, 126.0f));
  back_button->loadTextures("ui/back2_button.png", "ui/back2_button.png");
  back_button->setScale(0.8f);
  back_button->setPosition(Vec2(center.x, center.y - 80));
  back_button->setVisible(false);

  back_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 0.95f);
	back_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.8f);
	back_button->runAction(scaleTo2);
	replace_lobby_scene();

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	back_button->runAction(scaleTo);
      }
    });

  this->addChild(back_button, 2);
  */
}

void single_play2_scene::open_game_end_popup() {
  game_end_background->setVisible(true);
  retry_button->setVisible(true);
  back_button->setVisible(true);
}

void single_play2_scene::close_game_end_popup() {
  game_end_background->setVisible(false);
  retry_button->setVisible(false);
  back_button->setVisible(false);
}

void single_play2_scene::create_complete_popup() {
  auto offset = 5000.0f;
  complete_background_popup = Sprite::create("ui/background_popup.png");
  complete_background_popup->setScale(2.0f);
  complete_background_popup->setPosition(Vec2(center.x + offset, center.y));
  this->addChild(complete_background_popup, 2);

  complete_noti_font = Label::createWithTTF("   마지막 스테이지 입니다. \n 빠른 업데이트 하겠습니다.", "fonts/nanumb.ttf", 40);
  complete_noti_font->setPosition(Vec2(center.x + offset, center.y));
  complete_noti_font->setColor(Color3B( 110, 110, 110));
  this->addChild(complete_noti_font, 2);

  complete_confirm_button = ui::Button::create();
  complete_confirm_button->setTouchEnabled(true);
  complete_confirm_button->ignoreContentAdaptWithSize(false);
  complete_confirm_button->setContentSize(Size(286.0f, 126.0f));
  complete_confirm_button->loadTextures("ui/confirm_button.png", "ui/confirm_button.png");
  complete_confirm_button->setPosition(Vec2(center.x + offset, center.y));

  complete_confirm_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 1.1f);
        complete_confirm_button->runAction(scaleTo);


      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
        complete_confirm_button->runAction(scaleTo2);
        replace_lobby_scene();
        
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 1.0f);
        complete_confirm_button->runAction(scaleTo);
      }
    });
     
  this->addChild(complete_confirm_button, 2);
}

void single_play2_scene::open_complete_popup() {
  complete_background_popup->setPosition(Vec2(center));
  complete_noti_font->setPosition(Vec2(center.x, center.y + 60.0f));
  complete_confirm_button->setPosition(Vec2(center.x, center.y - 100.0f));
}

void single_play2_scene::close_complete_popup() {
  auto offset = 5000.0f;
  complete_background_popup->setPosition(Vec2(center.x + offset, center.y));
  complete_noti_font->setPosition(Vec2(center.x + offset, center.y + 60.0f));
  complete_confirm_button->setPosition(Vec2(center.x + offset, center.y - 100.0f));
}

void single_play2_scene::set_is_pause_false() {
  is_pause = false;
}

void single_play2_scene::set_is_store_on_false() {
  is_store_on = false;
}

void single_play2_scene::action_win_game() {
  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/YouWin.wav");

  auto youwin = Sprite::create("ui/youwin.png");
  youwin->setScale(2.0f);
  youwin->setPosition(Vec2(visible_size.width + 100.0f, center.y));
  this->addChild(youwin, 0);

  auto moveTo = MoveTo::create(0.8f, Vec2(center.x, center.y));
  auto fadeOut = FadeOut::create(1.2f);
  auto seq = Sequence::create(moveTo, fadeOut, nullptr);
  youwin->runAction(seq);
}

void single_play2_scene::create_stage_status() {
  auto ui_offset_x = 300;
  auto font_size = 30;
  
  auto font_x = visible_size.width/4 + ui_offset_x;
  auto font_y = center.y + _play_screen_y/2 - _offset_y+0;
  font_y = font_y + 1;

  stage_cnt_font = Label::createWithTTF(ccsf2("%d", stage_cnt+1), "fonts/nanumb.ttf", font_size);
  stage_cnt_font->setPosition(Vec2(font_x + 80, font_y));
  stage_cnt_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(stage_cnt_font, 1);
 
  auto stage_slash_font = Label::createWithTTF("/", "fonts/nanumb.ttf", font_size);
  stage_slash_font->setPosition(Vec2(stage_cnt_font->getPosition().x + (stage_cnt_font->getContentSize().width/2.0f) + 20, font_y));
  stage_slash_font->setColor( Color3B( 225, 225, 225) );
  this->addChild(stage_slash_font, 1);

  max_stage_cnt_font = Label::createWithTTF(ccsf2("%d", max_stage_cnt), "fonts/nanumb.ttf", font_size);
  max_stage_cnt_font->setPosition(Vec2(stage_slash_font->getPosition().x + (stage_slash_font->getContentSize().width/2.0f) + 10 + (max_stage_cnt_font->getContentSize().width/2.0f), font_y));
  max_stage_cnt_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(max_stage_cnt_font, 1);
  
  
  point_cnt_font = Label::createWithTTF("0", "fonts/nanumb.ttf", font_size);
  point_cnt_font->setPosition(Vec2(font_x + 365, font_y));
  point_cnt_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(point_cnt_font, 1);

  auto point_slash_font = Label::createWithTTF("/", "fonts/nanumb.ttf", font_size);
  point_slash_font->setPosition(Vec2(font_x + 390, font_y));
  point_slash_font->setColor( Color3B( 225, 225, 225) );
  this->addChild(point_slash_font, 1);

  max_point_cnt_font = Label::createWithTTF(ccsf2("%d", max_point_cnt), "fonts/nanumb.ttf", font_size);
  max_point_cnt_font->setPosition(Vec2(font_x + 415, font_y));
  max_point_cnt_font->setColor( Color3B( 255, 255, 255) );
  this->addChild(max_point_cnt_font, 1); 
  
}

void single_play2_scene::clear_hint() {
  //CCLOG("hint clear");
  hint_indexs.clear();
}

void single_play2_scene::create_store_popup() {
  store_background = Sprite::create("ui/store.png");
  store_background->setPosition(Vec2(center.x, center.y));
  this->addChild(store_background, 2);

  close_store_button = ui::Button::create();
  close_store_button->setTouchEnabled(true);
  close_store_button->ignoreContentAdaptWithSize(false);
  close_store_button->setContentSize(Size(128.0f, 128.0f));
  close_store_button->loadTextures("ui/close_popup.png", "ui/close_popup.png");
  close_store_button->setPosition(Vec2(center.x + store_background->getContentSize().width/2.0f- 48.0f, center.y + store_background->getContentSize().height / 2.0f - 75.0f));
  close_store_button->setScale(0.7f);

  close_store_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
        close_store_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.7f);
        close_store_button->runAction(scaleTo2);
	close_store_popup();

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.7f);
        close_store_button->runAction(scaleTo2);
      }
    });
     
  close_store_button->setOpacity(190);
  this->addChild(close_store_button, 2);

  hint10_button = ui::Button::create();
  hint10_button->setTouchEnabled(true);
  hint10_button->ignoreContentAdaptWithSize(false);
  hint10_button->setContentSize(Size(240.0f, 110.0f));
  hint10_button->setScale(0.8f);
  hint10_button->loadTextures("ui/buy.png", "ui/buy.png");
  hint10_button->setPosition(Vec2(center.x - 255.0f, center.y - 163.0f));

  hint10_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 0.9f);
	hint10_button->runAction(scaleTo);


      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.8f);
	hint10_button->runAction(scaleTo2);

#if (CC_TARGET_PLATFORM != CC_PLATFORM_LINUX) 
	sdkbox::IAP::purchase("hint10");
#endif
        
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	hint10_button->runAction(scaleTo);
      }
    });
     
  this->addChild(hint10_button, 2);

  hint25_button = ui::Button::create();
  hint25_button->setTouchEnabled(true);
  hint25_button->ignoreContentAdaptWithSize(false);
  hint25_button->setContentSize(Size(240.0f, 110.0f));
  hint25_button->setScale(0.8f);
  hint25_button->loadTextures("ui/buy.png", "ui/buy.png");
  hint25_button->setPosition(Vec2(center.x - 6.0f, center.y - 163.0f));
  hint25_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 0.9f);
	hint25_button->runAction(scaleTo);


      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.8f);
	hint25_button->runAction(scaleTo2);

#if (CC_TARGET_PLATFORM != CC_PLATFORM_LINUX) 
	sdkbox::IAP::purchase("hint25");
#endif
        
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	hint25_button->runAction(scaleTo);
      }
    });
     
  this->addChild(hint25_button, 2);

  hint99_button = ui::Button::create();
  hint99_button->setTouchEnabled(true);
  hint99_button->ignoreContentAdaptWithSize(false);
  hint99_button->setContentSize(Size(240.0f, 110.0f));
  hint99_button->setScale(0.8f);
  hint99_button->loadTextures("ui/buy.png", "ui/buy.png");
  hint99_button->setPosition(Vec2(center.x + 245.0f, center.y - 163.0f));
  hint99_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 0.9f);
	hint99_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.8f);
	hint99_button->runAction(scaleTo2);

#if (CC_TARGET_PLATFORM != CC_PLATFORM_LINUX) 
	sdkbox::IAP::purchase("hint99");
#endif
        
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 0.8f);
	hint99_button->runAction(scaleTo);
      }
    });
     
  this->addChild(hint99_button, 2);

  store_background->setVisible(false);
  close_store_button->setVisible(false);
  hint10_button->setVisible(false);
  hint25_button->setVisible(false);
  hint99_button->setVisible(false);
}

void single_play2_scene::open_store_popup() {
  close_block();
  is_store_on = true;
  store_background->setVisible(true);
  close_store_button->setVisible(true);
  hint10_button->setVisible(true);
  hint25_button->setVisible(true);
  hint99_button->setVisible(true);
}

void single_play2_scene::close_store_popup() {
  if(!is_pause && !is_end_play) open_block();
  this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::set_is_store_on_false), 1.0f);
  store_background->setVisible(false);
  close_store_button->setVisible(false);
  hint10_button->setVisible(false);
  hint25_button->setVisible(false);
  hint99_button->setVisible(false);
}

void single_play2_scene::add_hint_item(int hint_count) {
  auto remaining_hint_count = user_info::get().item_info_.get_hint_count();
  user_info::get().item_info_.set_hint_count(remaining_hint_count + hint_count);  
  hint_status_font->setString(ccsf2("x %d", user_info::get().item_info_.get_hint_count()));
  hint_button->setEnabled(true);
  hint_button->setBright(true);
}

void single_play2_scene::create_iap_popup() {
  iap_background_popup = Sprite::create("ui/background_popup.png");
  iap_background_popup->setScale(2.0f);
  iap_background_popup->setPosition(center.x, center.y);
  iap_background_popup->setVisible(false);
  this->addChild(iap_background_popup, 2);

  iap_noti_font = Label::createWithTTF("힌트 10개 충전이 완료되었습니다.", "fonts/nanumb.ttf", 40);
  iap_noti_font->setPosition(Vec2(center.x, center.y + 60.0f));
  iap_noti_font->setColor(Color3B( 110, 110, 110));
  iap_noti_font->setVisible(false);
  this->addChild(iap_noti_font, 2);

  iap_confirm_button = ui::Button::create();
  iap_confirm_button->setTouchEnabled(true);
  iap_confirm_button->ignoreContentAdaptWithSize(false);
  iap_confirm_button->setContentSize(Size(286.0f, 126.0f));
  iap_confirm_button->loadTextures("ui/confirm_button.png", "ui/confirm_button.png");
  iap_confirm_button->setPosition(Vec2(center.x, center.y - 100.0f));
  iap_confirm_button->setVisible(false);

  iap_confirm_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect("sound/pressing.mp3");
	auto scaleTo = ScaleTo::create(0.1f, 1.1f);
	iap_confirm_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 1.0f);
	iap_confirm_button->runAction(scaleTo2);
	close_iap_popup();
	close_store_popup();

      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo = ScaleTo::create(0.1f, 1.0f);
	iap_confirm_button->runAction(scaleTo);
      }
    });

  this->addChild(iap_confirm_button, 2);
}

void single_play2_scene::open_iap_popup() {
  iap_background_popup->setVisible(true);
  iap_confirm_button->setVisible(true);
  iap_noti_font->setVisible(true);
}

void single_play2_scene::close_iap_popup() {
  iap_background_popup->setVisible(false);
  iap_confirm_button->setVisible(false);
  iap_noti_font->setVisible(false);
}

#if (CC_TARGET_PLATFORM != CC_PLATFORM_LINUX) 
void single_play2_scene::onInitialized(bool ok) {
  if(ok) {
    is_iap_on = true;
  }
}

void single_play2_scene::onSuccess(const sdkbox::Product &p) {
  if (p.name == "hint10") {
    add_hint_item(10);
    iap_noti_font->setString("힌트 10개 충전이 완료되었습니다.");
  } else if (p.name == "hint25") {
    add_hint_item(25);
    iap_noti_font->setString("힌트 25개 충전이 완료되었습니다.");
  } else if (p.name == "hint99") {
    add_hint_item(99);
    iap_noti_font->setString("힌트 99개 충전이 완료되었습니다.");
  }

  open_iap_popup();
}

void single_play2_scene::onFailure(const sdkbox::Product &p, const std::string &msg) {
  // 결제 실패(인터넷등의 문제 etc)
  iap_noti_font->setString("결제를 실패하셨습니다.");
}

void single_play2_scene::onCanceled(const sdkbox::Product &p) {
  // 결제 취소
}

void single_play2_scene::onRestored(const sdkbox::Product& p) {
    CCLOG("Purchase Restored: %s", p.name.c_str());
}

void single_play2_scene::updateIAP(const std::vector<sdkbox::Product>& products) {
  /*
  for (int i=0; i < products.size(); i++) {
    CCLOG("IAP: ========= IAP Item =========");
    CCLOG("IAP: Name: %s", products[i].name.c_str());
    CCLOG("IAP: ID: %s", products[i].id.c_str());
    CCLOG("IAP: Title: %s", products[i].title.c_str());
    CCLOG("IAP: Desc: %s", products[i].description.c_str());
    CCLOG("IAP: Price: %s", products[i].price.c_str());
    CCLOG("IAP: Price Value: %f", products[i].priceValue);
  }
  */
}

void single_play2_scene::onProductRequestSuccess(const std::vector<sdkbox::Product> &products) {
  //updateIAP(products);
}

void single_play2_scene::onProductRequestFailure(const std::string &msg) {
  //CCLOG("Fail to load products");
}

void single_play2_scene::onRestoreComplete(bool ok, const std::string &msg) {
  //CCLOG("%s:%d:%s", __func__, ok, msg.data());
}

void single_play2_scene::onAdColonyChange(const sdkbox::AdColonyAdInfo& info, bool available) {

}

void single_play2_scene::onAdColonyReward(const sdkbox::AdColonyAdInfo& info, const std::string& currencyName, int amount, bool success) {

}

void single_play2_scene::onAdColonyStarted(const sdkbox::AdColonyAdInfo& info) {

}

void single_play2_scene::onAdColonyFinished(const sdkbox::AdColonyAdInfo& info) {
  replace_single_play2_scene(); 
}

#endif
