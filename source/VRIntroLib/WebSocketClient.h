#pragma once

#include <thread>
#include <string>
#include <memory>
#include <mutex>

// This class wraps dhbaird's websocket client class, "easywsclient", to make the interface even simpler

namespace easywsclient {
class WebSocket;
};
using easywsclient::WebSocket;

class WebSocketClient {
public:
  WebSocketClient(const std::string& url, std::function<void(const std::string& message)> recvCallback);
  ~WebSocketClient();
  bool Send(const std::string& message);

private:
  void Listen();

  std::function<void(const std::string& message)> m_Callback;
  std::thread m_Thread;
  std::unique_ptr<WebSocket> m_WebSocket;

  std::mutex m_Mutex;
};
