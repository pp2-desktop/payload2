#include "SimpleAudioEngine.h"
#include "assets_scene.hpp"

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
    
  auto closeItem = MenuItemImage::create(
					 "CloseNormal.png",
					 "CloseSelected.png",
					 CC_CALLBACK_1(assets_scene::menuCloseCallback, this));
  closeItem->setScale(2.0f, 2.0f);
  closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2-20, origin.y + closeItem->getContentSize().height/2+15));

  



  std::string manifestPath = "./project.manifest";
  //d::string storagePath = "/home/pp/workspace/tmp4/Resources/tmp";
  std::string storagePath = FileUtils::getInstance()->getWritablePath() + "res";


  
  if (!FileUtils::getInstance()->isDirectoryExist(storagePath)) {
    FileUtils::getInstance()->createDirectory(storagePath);
  }

  CCLOG("path: %s", storagePath.c_str());
  _am = AssetsManagerEx::create(manifestPath, storagePath);
  // As the process is asynchronies, you need to retain the assets manager to make sure it won't be released before the process is ended.
  _am->retain();

  if (!_am->getLocalManifest()->isLoaded()) {
    CCLOG("Fail to update assets, step skipped.");
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
	    //this->onLoadEnd();
	  }
	  break;

	case EventAssetsManagerEx::EventCode::UPDATE_PROGRESSION:
	  {
	    std::string assetId = event->getAssetId();
	    float percent = event->getPercent();
	    std::string str;
	    if (assetId == AssetsManagerEx::VERSION_ID)
	      {
		str = StringUtils::format("Version file: %.2f", percent) + "%";
	      }
	    else if (assetId == AssetsManagerEx::MANIFEST_ID)
	      {
		str = StringUtils::format("Manifest file: %.2f", percent) + "%";
	      }
	    else
	      {
		str = StringUtils::format("%.2f", percent) + "%";
		CCLOG("%.2f Percent", percent);
	      }

	    CCLOG("%s", str.c_str());
	    CCLOG("update progression");
	    /*
	    if (this->_progress != nullptr)
	      this->_progress->setString(str);
	    */
	  }
	  break;

	case EventAssetsManagerEx::EventCode::ERROR_DOWNLOAD_MANIFEST:
	case EventAssetsManagerEx::EventCode::ERROR_PARSE_MANIFEST:
	  {
	    CCLOG("Fail to download manifest file, update skipped.");
	    //this->onLoadEnd();
	  }
	  break;

	case EventAssetsManagerEx::EventCode::ALREADY_UP_TO_DATE:
	case EventAssetsManagerEx::EventCode::UPDATE_FINISHED:
	  {
	    CCLOG("Update finished. %s", event->getMessage().c_str());
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

void assets_scene::menuCloseCallback(Ref* pSender) {
  Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}

void assets_scene::update(float dt) {

  
  
  CCLOG("updating");
}
