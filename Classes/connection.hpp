#ifndef __connection_HPP__
#define __connection_HPP__

#include <string>
#include <deque>
#include "cocos2d.h"
#include "network/WebSocket.h"
#include "json11.hpp"

using namespace json11;
using std::string;

class connection: public cocos2d::network::WebSocket::Delegate {
private:
  std::string url_;
  cocos2d::network::WebSocket* _websocket;
  bool is_connected;

public:
  std::function<void(std::string message)> onMessageReceived;
  std::function<void()> onConnectionClosed;
  std::function<void(const cocos2d::network::WebSocket::ErrorCode &error)> onErrorOccurred;
  std::function<void()> onConnectionOpened;
    
  connection();
  ~connection();
    
  void connect();
  void create(std::string url);
    
  virtual void onOpen(cocos2d::network::WebSocket* ws);
  virtual void onMessage(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::Data& data);
  virtual void onError(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::ErrorCode& error);
  virtual void onClose(cocos2d::network::WebSocket* ws);
    
  void close();
  void send(std::string mesage);
  void send2(Json payload);


  std::deque<Json> q;

  static connection& get() {
    static connection obj;
    return obj;
  }

  bool get_is_connected() { return is_connected; }
};

#endif
