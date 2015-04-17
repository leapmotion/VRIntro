#pragma once

#include "VRIntroLib/IFrameSupplier.h"
#include "LeapListener.h"
#include "Leap.h"

class APIFrameSupplier : public virtual IFrameSupplier {
public:
  APIFrameSupplier();
  ~APIFrameSupplier();
  virtual void PopulateInteractionLayer(InteractionLayer& target, const float* worldTransformRaw) const override;
  virtual void PopulatePassthroughLayer(PassthroughLayer& target, int i, bool useLatestImage = true) const override;
  virtual bool IsDragonfly() const override;
  virtual double GetFPSEstimate() const override;

  virtual void Lock() override {}
  virtual void Unlock() override {}

private:
  LeapListener m_LeapListener;
  Leap::Controller m_LeapController;
};
