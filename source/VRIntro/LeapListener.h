#pragma once

#include "Leap.h"
#include "EigenTypes.h"
#include <vector>
#include <mutex>
#include <condition_variable>

using namespace Leap;

class LeapListener : public Listener {
public:
  LeapListener();
  virtual void onInit(const Controller&);
  virtual void onConnect(const Controller&);
  virtual void onDisconnect(const Controller&);
  virtual void onExit(const Controller&);
  virtual void onFrame(const Controller&);
  virtual void onFocusGained(const Controller&);
  virtual void onFocusLost(const Controller&);
  virtual void onDeviceChange(const Controller&);
  virtual void onServiceConnect(const Controller&);
  virtual void onServiceDisconnect(const Controller&);
  //std::mutex& getMutex() { return m_Mutex; }
  //const std::mutex& getMutex() const { return m_Mutex; }
  void WaitForFrame();
  double GetFPSEstimate() { return m_FPSEstimate; }

private:
  static const double FPS_SMOOTHING;
  
  void EstimateFPS(double dt);

  std::mutex m_Mutex;
  std::condition_variable m_Cond;

  double m_FPSEstimate;
  double m_FPSIntegral;
  double m_LastTimestamp;
};
