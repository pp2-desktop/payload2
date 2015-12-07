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
  play_time_sec = 10;
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

int play_info_md::increase_clear_stage(std::string theme) {
  
  auto clear_stage = user_played_infos[theme].clear_stage;
  auto max_stage_cnt = user_played_infos[theme].max_stage_cnt;

  clear_stage = clear_stage + 1;

  if(clear_stage > max_stage_cnt) {
    clear_stage = max_stage_cnt;
  }

  CCUserDefault *def=CCUserDefault::sharedUserDefault();
  def->setIntegerForKey(theme.c_str(), clear_stage);
  def->flush();

  user_played_infos[theme].clear_stage = clear_stage;


  return clear_stage;
}
