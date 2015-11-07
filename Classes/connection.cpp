#include "connection.hpp"

USING_NS_CC;
using namespace cocos2d::network;


void connection::create(std::string url) {
  url_ = url;
}

connection::connection() {
  is_connected = false;
};

void connection::connect() {
  _websocket = new WebSocket();
  _websocket->init(*this, url_.c_str());
}

void connection::close() {

  if (_websocket->getReadyState() == WebSocket::State::OPEN) {
    _websocket->close();
  }
}

connection::~connection() {
  _websocket->close();
}

void connection::send(std::string message) {
  //CCLOG("[debug] send");
  if (_websocket->getReadyState() == WebSocket::State::OPEN) {
    _websocket->send(message);
  }
}

void connection::send2(Json payload) {
  //CCLOG("[debug] send2");
  if (_websocket->getReadyState() == WebSocket::State::OPEN) {
    std::string send_msg = payload.dump();
    send(send_msg);
  } else {
    CCLOG("[error] 접속 상태 아님");
  }
}

void connection::onOpen(WebSocket* ws) {
  CCLOG("WebSocket connection opened: %s", url_.c_str());
  is_connected = true;

  auto connected_notify = Json::object {
    { "type", "connection_notify" }
  };

  q.push_back(connected_notify);

  if ( onConnectionOpened ) {
    onConnectionOpened();
  }
}

void connection::onMessage(WebSocket* ws, const WebSocket::Data &data) {

  std::string message_str(data.bytes);
  CCLOG("msg: %s", message_str.c_str());


   string err;
   auto payload = Json::parse(message_str, err);

   if (!err.empty()) {
     CCLOG("[error] fail to parse json: %s",  err.c_str());
   } else {
     //CCLOG("[debug] before q size: %u",  q.size());
     q.push_back(payload);
     //CCLOG("[debug] after q size: %u",  q.size());
   }

  if ( onMessageReceived ) {
    onMessageReceived(data.bytes);
  }
    
}

void connection::onError(WebSocket* ws, const WebSocket::ErrorCode& error) {
  CCLOG("[error] onError 발생");

  if ( onErrorOccurred ) {
    onErrorOccurred(error);
  }
}

void connection::onClose(WebSocket* ws) {

  CCLOG("[debug] onClose 발생");

  is_connected = false;
  auto disconnected_notify = Json::object {
    { "type", "disconnection_notify" }
  };

  q.push_back(disconnected_notify);

  if ( onConnectionClosed ) {
    onConnectionClosed();
  }
}
