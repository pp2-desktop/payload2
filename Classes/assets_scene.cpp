#include "SimpleAudioEngine.h"
#include "assets_scene.hpp"
#include "lobby_scene.hpp"
#include "resource_md.hpp"

using namespace CocosDenshion;

Scene* assets_scene::createScene() {

  auto scene = Scene::create();
  auto layer = assets_scene::create();

  scene->addChild(layer);

  return scene;
}

// on "init" you need to initialize your instance
bool assets_scene::init() {
  //////////////////////////////
  // 1. super init first
  if ( !Layer::init() )
    {
      return false;
    }

  CCLOG("assets scene init called");
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  center_ = Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y);
    
  // 리소스 매니져 초기화
  resource_md::get().init();

  auto loading = Sprite::create("ui/loading.png");
  loading->setPosition(Vec2(center_.x, center_.y));
  this->addChild(loading, 0);


  tmp = Label::createWithTTF("FUCK", "fonts/nanumb.ttf", 40);
  tmp->setPosition(Vec2(center_.x, center_.y + 50));
tmp->setColor( Color3B( 255, 255, 255));
  this->addChild(tmp, 1);


#if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX) 
  std::string manifestPath = "./project.manifest";
#else
  std::string manifestPath = "project.manifest";
#endif

  std::string storagePath = FileUtils::getInstance()->getWritablePath() + "res";
  resource_md::get().path = storagePath + "/";
  
  if (!FileUtils::getInstance()->isDirectoryExist(storagePath)) {
    FileUtils::getInstance()->createDirectory(storagePath);
  }

  CCLOG("path: %s", storagePath.c_str());
  _am = AssetsManagerEx::create(manifestPath, storagePath);
  // As the process is asynchronies, you need to retain the assets manager to make sure it won't be released before the process is ended.
  _am->retain();

  if (!_am->getLocalManifest()->isLoaded()) {
    CCLOG("Fail to update assets, step skipped.");

    tmp->setString("000000000000000000000000");
    //AssetsManagerExTestScene *scene = new AssetsManagerExTestScene(backgroundPaths[currentId]);
    //Director::getInstance()->replaceScene(scene);
    //scene->release();
  }
  else {

    _amListener = cocos2d::extension::EventListenerAssetsManagerEx::create(_am, [this] (EventAssetsManagerEx* event) {
	static int failCount = 0;

	switch (event->getEventCode()) {

	case EventAssetsManagerEx::EventCode::ERROR_NO_LOCAL_MANIFEST:
	  {
	    CCLOG("No local manifest file found, skip assets update.");
	    tmp->setString("No local manifest file found, skip assets update.");
	    //this->onLoadEnd();
	  }
	  break;

	case EventAssetsManagerEx::EventCode::UPDATE_PROGRESSION:
	  {
	    std::string assetId = event->getAssetId();
	    float percent = event->getPercent();
	    std::string percent_str;
	    if (assetId == AssetsManagerEx::VERSION_ID)
	      {
		percent_str = StringUtils::format("Version file: %.2f", percent) + "%";
		
		tmp->setString("111111111111111111");
	      }
	    else if (assetId == AssetsManagerEx::MANIFEST_ID)
	      {
		percent_str = StringUtils::format("Manifest file: %.2f", percent) + "%";

		tmp->setString("22222222222222222");
	      }
	    else
	      {
		percent_str = StringUtils::format("%.2f", percent) + "%";
		CCLOG("assetId: %s", assetId.c_str());
		CCLOG("%.2f Percent", percent);

		tmp->setString(percent_str);


	      }

	 
	    CCLOG("%s", percent_str.c_str());
	    CCLOG("update progression");
	  
	  }
	  break;

	case EventAssetsManagerEx::EventCode::ERROR_DOWNLOAD_MANIFEST:
	case EventAssetsManagerEx::EventCode::ERROR_PARSE_MANIFEST:
	  {
	    CCLOG("Fail to download manifest file, update skipped.");
	    resource_md::get().set_is_resource_load(false);
	    this->replace_lobby_scene();
	    //this->onLoadEnd();
	  }
	  break;

	case EventAssetsManagerEx::EventCode::ALREADY_UP_TO_DATE:
	case EventAssetsManagerEx::EventCode::UPDATE_FINISHED:
	  {
	    CCLOG("Update finished. %s", event->getMessage().c_str());
	    resource_md::get().set_is_resource_load(true);
	    this->replace_lobby_scene();
	    //this->onLoadEnd();
	  }
	  break;

	case EventAssetsManagerEx::EventCode::UPDATE_FAILED:
	  {
	    CCLOG("Update failed. %s", event->getMessage().c_str());

	    failCount++;
	    if (failCount < 5)
	      {
		_am->downloadFailedAssets();
	      }
	    else
	      {
		CCLOG("Reach maximum fail count, exit update process");
		failCount = 0;
		resource_md::get().set_is_resource_load(false);
		this->replace_lobby_scene();
		//this->onLoadEnd();
	      }
	  }
	  break;

	case EventAssetsManagerEx::EventCode::ERROR_UPDATING:
	  {
	    CCLOG("Asset %s : %s", event->getAssetId().c_str(), event->getMessage().c_str());
	  }
	  break;

	case EventAssetsManagerEx::EventCode::ERROR_DECOMPRESS:
	  {
	    CCLOG("%s", event->getMessage().c_str());
	  }
	  break;

	default:
	  break;
	}

      });

    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_amListener, 1);
    _am->update();

  }

  this->scheduleUpdate();
    
  return true;
}

void assets_scene::update(float dt) {

  
  
  //CCLOG("updating");
}

void assets_scene::replace_lobby_scene() {
  auto lobby_scene = lobby_scene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.0f, lobby_scene, Color3B(0,255,255)));
  
}
