#include "AppDelegate.h"
#include "lobby_scene.hpp"
#include "user_info.hpp"
//#include "assets_scene.hpp"
#include "multi_lobby_scene.hpp"
USING_NS_CC;

static cocos2d::Size designResolutionSize = cocos2d::Size(1334, 750);
static cocos2d::Size eeResolutionSize = cocos2d::Size(1334, 750);

static cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
static cocos2d::Size mediumResolutionSize = cocos2d::Size(1024, 768);
static cocos2d::Size largeResolutionSize = cocos2d::Size(2048, 1536);

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
}

//if you want a different context,just modify the value of glContextAttrs
//it will takes effect on all platforms
void AppDelegate::initGLContextAttrs()
{
    //set OpenGL context attributions,now can only set six attributions:
    //red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

// If you want to use packages manager to install more packages, 
// don't modify or remove this function
static int register_all_packages()
{
    return 0; //flag for packages manager
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)

      //glview = GLViewImpl::createWithRect("payload2", Rect(0, 0, designResolutionSize.width/2, designResolutionSize.height/2));
      glview = GLViewImpl::createWithRect("payload2", Rect(0, 0, 2048/4, 1536/4));
      //glview = GLViewImpl::createWithRect("payload2", Rect(0, 0, 1, 1));
#else
        glview = GLViewImpl::create("payload2");
#endif
        director->setOpenGLView(glview);
    }

    // turn on display FPS
    director->setDisplayStats(false);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0 / 60);

    

    // Set the design resolution
    //glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::SHOW_ALL);
    glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::FIXED_WIDTH);

    /*
    Size frameSize = glview->getFrameSize();
    // if the frame's height is larger than the height of medium size.
    if (frameSize.height > mediumResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(lResolutionSize.height/designResolutionSize.height, largeResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is larger than the height of small size.
    else if (frameSize.height > smallResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(mediumResolutionSize.height/designResolutionSize.height, mediumResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is smaller than the height of medium size.
    else
    {        
        director->setContentScaleFactor(MIN(smallResolutionSize.height/designResolutionSize.height, smallResolutionSize.width/designResolutionSize.width));
    }
    */

    director->setContentScaleFactor(MIN(eeResolutionSize.height/designResolutionSize.height, eeResolutionSize.width/designResolutionSize.width));

    register_all_packages();

    // create a scene. it's an autorelease object
    auto scene = lobby_scene::createScene();
    //auto scene = assets_scene::createScene();
    //auto scene = multi_lobby_scene::createScene();
    user_info::get().init();
    // run
    director->runWithScene(scene);
    cocos2d::Device::setKeepScreenOn(true);
  
#if (CC_TARGET_PLATFORM != CC_PLATFORM_LINUX) 
    sdkbox::IAP::init();
    sdkbox::PluginAdColony::init();
    //sdkbox::IAP::refresh();
#endif
  
    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
