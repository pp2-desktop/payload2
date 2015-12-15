#ifndef __RESOURCE_MD_HPP__
#define __RESOURCE_MD_HPP__

#include <memory>
#include "cocos2d.h"

class resource_md {

public:

  resource_md();
  ~resource_md();

  void init();

  static resource_md& get() {
    static resource_md obj;
    return obj;
  }
  
  bool get_is_resource_load() { return is_resource_load_; }
  void set_is_resource_load(bool v) { 
    CCLOG("called resource load called");
    is_resource_load_ = v;
  }

  std::string path;
private:
  bool is_resource_load_;
};

#endif
