/*==================================================================================================================

    Copyright (c) 2010 - 2012 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

/// <summary>
/// Class for accurate timing
/// </summary>
/// <remarks>
/// This is a high-precision timing class. Whereas the resolution of the c++ standard clock() function is about 10ms,
/// this class has a resolution of about one clock cycle (using the PrecisionTimer).
///
/// </remarks>

#ifndef __PrecisionTimer_h__
#define __PrecisionTimer_h__

#include <chrono>
#include <thread>

template<typename T>
class GeneralTimer
{
public:
  GeneralTimer():
    m_Start(epoch())
  {}

  void Start() {
    m_Start = T::now();
  }

  std::chrono::nanoseconds StopNanoseconds() const {
    return T::now() - m_Start;
  }
  double Stop() const {
    return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(StopNanoseconds()).count())*1e-6;
  }

  std::chrono::nanoseconds StopAndStartNanoseconds() {
    const std::chrono::nanoseconds retVal = StopNanoseconds();
    Start();
    return retVal;
  }
  double StopAndStart() {
    const double retVal = Stop();
    Start();
    return retVal;
  }

  static double GetTime() {
    const std::chrono::time_point<T> end = T::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - epoch()).count() * 1e-9;
  }

  /// <summary>
  /// Returns the current time according to this timer type
  /// </summary>
  static std::chrono::steady_clock::time_point Now() {
    return T::now();
  }

  static void Sleep(int milliseconds) {
    std::this_thread::sleep(std::posix_time::milliseconds(milliseconds));
  }

private:
  static std::chrono::time_point<T>& epoch() {
    static std::chrono::time_point<T> s_Epoch = T::now();
    return s_Epoch;
  }

  std::chrono::time_point<T> m_Start;

};

typedef GeneralTimer<std::chrono::high_resolution_clock> PrecisionTimer;

#endif
