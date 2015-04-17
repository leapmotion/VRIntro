#pragma once

class InteractionLayer;
class PassthroughLayer;

class IFrameSupplier {
public:
  virtual void PopulateInteractionLayer(InteractionLayer& target, const float* worldTransformRaw) const = 0;
  virtual void PopulatePassthroughLayer(PassthroughLayer& target, int i, bool useLatestImage = true) const = 0;
  virtual bool IsDragonfly() const = 0;
  virtual double GetFPSEstimate() const = 0;
  virtual void Lock() = 0;
  virtual void Unlock() = 0;
};
