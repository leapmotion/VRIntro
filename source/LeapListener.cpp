#include "stdafx.h"
#include "LeapListener.h"

//#include "PrecisionTimer.h"

#include <iostream>
#include <string.h>
#include "Leap.h"

const double LeapListener::FPS_SMOOTHING = 0.1;

using namespace Leap;

LeapListener::LeapListener() :
  m_LastTimestamp(DBL_MAX),
  m_FPSIntegral(1/(std::exp(FPS_SMOOTHING/100) - 1)) {
}

void LeapListener::onInit(const Controller& controller) {
  std::cout << "Initialized" << std::endl;
}

void LeapListener::onConnect(const Controller& controller) {
  std::cout << "Connected" << std::endl;
}

void LeapListener::onDisconnect(const Controller& controller) {
  std::cout << "Disconnected" << std::endl;
}

void LeapListener::onExit(const Controller& controller) {
  std::cout << "Exited" << std::endl;
}

void LeapListener::onFrame(const Controller& controller) {
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Cond.notify_all();
  
  double t = static_cast<double>(controller.frame().timestamp()) * 1e-6;
  //double t = PrecisionTimer::GetTime();

  m_LastTimestamp = std::min(m_LastTimestamp, t);
  EstimateFPS(t - m_LastTimestamp);
  m_LastTimestamp = t;
  //std::cout << __LINE__ << ":\t   m_FPSEstimate = " << (m_FPSEstimate) << std::endl;
}

void LeapListener::onFocusGained(const Controller& controller) {
  std::cout << "Focus Gained" << std::endl;
  m_LastTimestamp = DBL_MAX;
}

void LeapListener::onFocusLost(const Controller& controller) {
  std::cout << "Focus Lost" << std::endl;
}

void LeapListener::onDeviceChange(const Controller& controller) {
  std::cout << "Device Changed" << std::endl;
  const DeviceList devices = controller.devices();

  for (int i = 0; i < devices.count(); ++i) {
    std::cout << "id: " << devices[i].toString() << std::endl;
    std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
  }
}

void LeapListener::onServiceConnect(const Controller& controller) {
  std::cout << "Service Connected" << std::endl;
}

void LeapListener::onServiceDisconnect(const Controller& controller) {
  std::cout << "Service Disconnected" << std::endl;
}

void LeapListener::WaitForFrame() {
  std::unique_lock<std::mutex> lock(m_Mutex);
  m_Cond.wait(lock);
}

void LeapListener::EstimateFPS(double dt) {
  m_FPSIntegral = (m_FPSIntegral + 1)*std::exp(-FPS_SMOOTHING*dt);
  m_FPSEstimate = m_FPSIntegral == 0 ? 0 : FPS_SMOOTHING/std::log(1 + 1/m_FPSIntegral);
}
