#include "vs_room_scene.hpp"
#include "connection.hpp"
#include "user_info.hpp"

using namespace ui;

Scene* vs_room_scene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = vs_room_scene::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool vs_room_scene::init()
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

    /////////////////////////////
    //
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(vs_room_scene::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2, origin.y + closeItem->getContentSize().height/2));
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);  
   
    this->scheduleUpdate();   

    return true;
}


void vs_room_scene::menuCloseCallback(Ref* pSender) {
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

void vs_room_scene::update(float dt) {

  if(!connection::get().q.empty()) {
    handle_payload(dt);
  } 
  
  // next
  

  //CCLOG("update");
}

void vs_room_scene::handle_payload(float dt) {

    Json payload = connection::get().q.front();
    connection::get().q.pop_front();

    std::string type = payload["type"].string_value();

    if(type == "connection_notify") {

    } else if(type == "disconnection_notify") {
      CCLOG("[debug] 접속 큰킴");

    } else if(type == "login_res") {

    } else {
      CCLOG("[error] handler 없음");
    }
}
