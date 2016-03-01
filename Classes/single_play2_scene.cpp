#include "SimpleAudioEngine.h"
#include "single_play2_scene.hpp"
#include "connection.hpp"
#include "single_play_info.hpp"
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

  auto audio = SimpleAudioEngine::getInstance();
  
  /*
  if(user_info::get().sound_option_.get_background()) {
    audio->playBackgroundMusic("sound/besound_ukulele.mp3", true);
    audio->setBackgroundMusicVolume(0.4f);
  } else {
    audio->setBackgroundMusicVolume(0.0f);
  }
  */
  
    
  visible_size = Director::getInstance()->getVisibleSize();
  origin = Director::getInstance()->getVisibleOrigin();
  center = Vec2(visible_size.width/2 + origin.x, visible_size.height/2 + origin.y);

  auto stage_cnt = play_info_md::get().single_play2_info_.get_stage_cnt();
  is_playing = false;

   connection::get().send2(Json::object {
     { "type", "single_img_info_req" },
     { "stage_count", stage_cnt }
    });


   connection::get().send2(Json::object {
     { "type", "max_stage_req" },
    });
 
  create_ui_top();
  create_ui_timer();

  create_block();
  create_status_font();
    
  auto input_listener = EventListenerTouchOneByOne::create();
  input_listener->setSwallowTouches(true);
 
  input_listener->onTouchBegan = [=](Touch* touch, Event* event) {
    if(!is_playing || is_incorrect_action) return false;

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
      }

      loading_stage(game_stage.img);
    } else if(type == "max_stage_res") {
      auto max_stage_cnt = payload["max_stage_count"].int_value();
      play_info_md::get().single_play2_info_.set_max_stage_cnt(max_stage_cnt);
    } else {
      CCLOG("[error] handler 없음");
    }
}

void single_play2_scene::create_ui_top() {
  auto ui_top_bg = Sprite::create("ui/top23.png");
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

  pause_button->setPosition(Vec2(45, center.y + _play_screen_y/2 - _offset_y));

  pause_button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
      if(type == ui::Widget::TouchEventType::BEGAN) {

	if(img_complete_cnt <= 1) {
	  return;
	}

	auto scaleTo = ScaleTo::create(0.1f, 0.6f);
	pause_button->runAction(scaleTo);

      } else if(type == ui::Widget::TouchEventType::ENDED) {

	if(img_complete_cnt <= 1) {
	  return;
	}

	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	pause_button->runAction(scaleTo2);

        //start_pause();
      } else if(type == ui::Widget::TouchEventType::CANCELED) {
	auto scaleTo2 = ScaleTo::create(0.1f, 0.5f);
	pause_button->runAction(scaleTo2);
      }
    });
     
  this->addChild(pause_button, 0);
}

void single_play2_scene::create_ui_timer() {

  timeBar = CCSprite::create("ui/timebar2.png");

  // 10초 동안 게이지 100% 동안 내려옴
  //CCProgressFromTo* progressToZero = CCProgressFromTo::create(10, 100, 0);
  //progressTimeBar_->runAction(progressToZero);
 
  auto timeOutline = CCSprite::create("ui/timeoutline2.png");
  timeOutline->setPosition(Vec2(timeBar->getContentSize().width/2 + 85, center.y + _play_screen_y/2 - _offset_y-1+0));

  timeOutline->setScaleX(0.75f);
  timeOutline->setScaleY(0.7f);
  timeOutline->setVisible(true);
  this->addChild(timeOutline, 0);

  progressTimeBar_ = CCProgressTimer::create(timeBar);

  progressTimeBar_->setPosition(Vec2(timeBar->getContentSize().width/2 + 85, center.y + _play_screen_y/2 - _offset_y-1+4));
  progressTimeBar_->setScaleX(0.75f);
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
  if(!is_playing) return;
  // call 4 times in a sec => 60초에 100%달게 할려면
  // 240번 불러야함
  float timer_sec = 60;
  float cPercentage = progressTimeBar_->getPercentage();
  progressTimeBar_->setPercentage(cPercentage - (100 / (60 * timer_sec)));
  
}

void single_play2_scene::increase_timer(int sec) {
  if(!is_playing) return;
  float cPercentage = progressTimeBar_->getPercentage();
  progressTimeBar_->setPercentage(cPercentage + (60 * sec));
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
  audio->playEffect("sound/Ready_1.wav", false, 1.0f, 1.0f, 1.0f);

  is_playing = true;
}

void single_play2_scene::create_go() {

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

  this->schedule(SEL_SCHEDULE(&single_play2_scene::update_timer), 1/10);
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

  this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::ready_go), 0.5f);
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
    return;
  }

  if(!response->isSucceed()) {
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
    return;
  }

  if(!response->isSucceed()) {
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

  resource_status_font->setString("상대편 이미지 다운로드 기다리는 중");
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
}

void single_play2_scene::end_game() {
  close_block();
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
    this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::complete_stages), 0.2f);
  }
}

void single_play2_scene::complete_stages() {


}

void single_play2_scene::check_end_play() {
  int cPercentage = progressTimeBar_->getPercentage();
  //CCLOG("Percentage: %d", cPercentage);
  if(cPercentage <= 0 && (!is_end_play)) {
    is_end_play = true;

    this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::end_game), 1.2f);

    auto audio = SimpleAudioEngine::getInstance();
    audio->playEffect("sound/YouFailed.wav", false, 1.0f, 1.0f, 1.0f);

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
    timeBar->setColor(Color3B(255, 0, 0));
  }
}

void single_play2_scene::check_user_input(float x, float y) {
  Vec2 img_pos = change_device_to_img_pos(x, y);
  CCLOG("x: %f", img_pos.x);
  CCLOG("y: %f", img_pos.y);

  auto index = check_point(img_pos.x, img_pos.y);

  if(index >= 0) {
    CCLOG("맞춤");
    action_correct(game_stage.hidden_points[index]);
    game_stage.found_indexs.insert(index);

    if(game_stage.hidden_points.size() <= game_stage.found_indexs.size()) {

      is_playing = false;
      auto audio = SimpleAudioEngine::getInstance();
      audio->playEffect("sound/YouWin.wav", false, 1.0f, 1.0f, 1.0f);

      auto youwin = Sprite::create("ui/youwin.png");
      youwin->setScale(2.0f);
      youwin->setPosition(Vec2(visible_size.width + 100.0f, center.y));
      this->addChild(youwin, 0);

      auto moveTo = MoveTo::create(0.8f, Vec2(center.x, center.y));
      auto fadeOut = FadeOut::create(1.2f);
      auto seq = Sequence::create(moveTo, fadeOut, nullptr);
      youwin->runAction(seq);
      this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::win_game), 1.2f);
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
    audio->playEffect("sound/great.wav", false, 1.0f, 1.0f, 1.0f);
  } else if(r==1) {
    audio->playEffect("sound/good.wav", false, 1.0f, 1.0f, 1.0f);
  } else if(r==2) {
    audio->playEffect("sound/cool.wav", false, 1.0f, 1.0f, 1.0f);
  } else {
    audio->playEffect("sound/yeah.wav", false, 1.0f, 1.0f, 1.0f);
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
}

void single_play2_scene::action_incorrect(float x, float y) {
  is_incorrect_action = true;
  this->scheduleOnce(SEL_SCHEDULE(&single_play2_scene::release_incorrect_action), 0.75f);
  
  auto audio = SimpleAudioEngine::getInstance();
  audio->playEffect("sound/incorrect2.wav", false, 1.0f, 1.0f, 1.0f);

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
