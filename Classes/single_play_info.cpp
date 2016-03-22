#include "single_play_info.hpp"

spot_info::spot_info() {
  pos = Vec2(0, 0);
  is_find = false;
}

spot_info::~spot_info() {
  pos = Vec2(0, 0);
  is_find = false;
}

play_info::play_info() {
  play_time_sec = 0;
}

play_info::~play_info() {

}

bool play_info::add_spot_info(float x, float y) {
 
  spot_info si;
  Vec2 pos(x, y);
  si.pos = pos;
  si.is_find = false;

  spot_infos.push_back(si);

  return true;
}

int play_info::check_spot_info(float x, float y) {

  auto index = 0;
  for(auto& spot_info: spot_infos) {
    if(is_spot_info_in_area(x, y, spot_info.pos.x, spot_info.pos.y)) {
      if(spot_info.is_find) {
	return -1;
      }
      spot_info.is_find = true;
      return index;
    }
    index++;
  }

  return -1;
}

bool play_info::is_spot_info_in_area(float ux, float uy, float xc, float yc, float r) {
   return ((ux-xc)*(ux-xc) + (uy-yc)*(uy-yc)) < r*r;
}

spot_info play_info::get_spot_info(int index) {
  if(index > static_cast<int>(spot_infos.size())) {
    
  }
  return spot_infos[index];
}

single_play2_info::single_play2_info() {
  stage_cnt_ = 0;
  max_stage_cnt_ = 0;
}

single_play2_info::~single_play2_info() {

}

void single_play2_info::reset() {
  stage_cnt_ = 0;
  max_stage_cnt_ = 0;
}

int single_play2_info::get_stage_cnt() {
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  stage_cnt_ = pUserDefault->getIntegerForKey("stage_cnt", 0);
  return stage_cnt_; 
}

void single_play2_info::set_stage_cnt(int stage_cnt) { 
  stage_cnt_ = stage_cnt;
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  pUserDefault->setIntegerForKey("stage_cnt", stage_cnt_);
  pUserDefault->flush();
}

bool single_play2_info::increase_stage_cnt() {
  bool r = false;
  if(stage_cnt_ + 1 < max_stage_cnt_) {
    r = true;
  }

  stage_cnt_ += 1;
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  pUserDefault->setIntegerForKey("stage_cnt", stage_cnt_);
  pUserDefault->flush();

  return r;
}

int single_play2_info::get_max_stage_cnt() {
  return max_stage_cnt_; 
}

void single_play2_info::set_max_stage_cnt(int max_stage_cnt) { 
  max_stage_cnt_ = max_stage_cnt; 
}

int single_play2_info::get_retry_cnt() { 
  auto pUserDefault = CCUserDefault::sharedUserDefault();
  int retry_cnt = pUserDefault->getIntegerForKey("retry_cnt", 1);
  pUserDefault->setIntegerForKey("retry_cnt", retry_cnt+1);
  pUserDefault->flush();
  return retry_cnt;
}

int play_info_md::increase_clear_stage(std::string theme) {
  
  auto clear_stage = user_played_infos[theme].clear_stage;
  auto max_stage_cnt = user_played_infos[theme].max_stage_cnt;

  clear_stage = clear_stage + 1;

  bool end_theme = false;
  if(clear_stage >= max_stage_cnt) {
    clear_stage = max_stage_cnt;
    end_theme = true;
  }

  CCUserDefault *def=CCUserDefault::sharedUserDefault();
  def->setIntegerForKey(theme.c_str(), clear_stage);
  def->flush();

  user_played_infos[theme].clear_stage = clear_stage;

  if(end_theme) {
    return -1;
  }

  return clear_stage;
}

stage_info play_info_md::get_stage_info(std::string theme, int clear_stage) {
  return user_played_infos[theme].stage_infos[clear_stage];
}
