#pragma once

#include "EigenTypes.h"

class Interactionlayer;
class PassthroughLayer;

class IFrameSupplier {
public:
  virtual void PopulateInteractionLayer(InteractionLayer& target, const Matrix4x4f& worldTransform) const = 0;
  virtual void PopulatePassthroughLayer(PassthroughLayer& target, int i) const = 0;
  virtual bool IsDragonfly() const = 0;
  virtual void Lock() = 0;
  virtual void Unlock() = 0;
};
