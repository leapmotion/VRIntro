#include "stdafx.h"
#include "WebSocketClient.h"

#include "contrib/easywsclient.hpp"
#ifdef _WIN32
#include <WinSock2.h>
#endif
#include <string>
#include <memory>
#include <iostream>

WebSocketClient::WebSocketClient(const std::string& url, std::function<void(const std::string& message)> recvCallback) :
  m_Callback(recvCallback) {

  using easywsclient::WebSocket;
#ifdef _WIN32
  INT rc;
  WSADATA wsaData;

  rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (rc) {
    std::cout << "ERROR: WSAStartup failed.\n";
    return;
  }
#endif

  m_WebSocket = std::unique_ptr<WebSocket>(WebSocket::from_url(url));
  if (!m_WebSocket) {
    std::cout << "ERROR: Connecting to server " << url << " failed.\n";
    return;
  }

  m_Thread = std::thread(&WebSocketClient::Listen, this);
}

WebSocketClient::~WebSocketClient() {
  if (m_WebSocket && m_WebSocket->getReadyState() != WebSocket::CLOSED) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_WebSocket->close();
  }
  m_Thread.join();
#ifdef _WIN32
  WSACleanup();
#endif
}

bool WebSocketClient::Send(const std::string& message) {
  if (m_WebSocket && m_WebSocket->getReadyState() == WebSocket::OPEN) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_WebSocket->send(message);
    return true;
  } else {
    //std::cout << "ERROR: No connection.\n";
    return false;
  }
}

void WebSocketClient::Listen() {
  while (m_WebSocket->getReadyState() != WebSocket::CLOSED) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_WebSocket->poll();
    m_WebSocket->dispatch(m_Callback);
  }
  std::cout << "Disconnected.\n";
}
