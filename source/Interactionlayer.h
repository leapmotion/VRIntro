#pragma once

#include "RenderableEventHandler.h"
#include "RenderState.h"
#include "EigenTypes.h"
#include "Leap.h"

#include <memory>

class GLShader;

struct SkeletonHand {
  //stdvectorV3f tips[5];
  Vector3f joints[21];
  Vector3f jointConnections[21];
};

class InteractionLayer : public RenderableEventHandler {
public:
  InteractionLayer(const std::string& shaderName = "lighting");
  RenderState& GetRenderState() { return m_Renderer; }
  void UpdateLeap(const Leap::Frame& frame, const Matrix4x4f& worldTransform);
  void UpdateEyePos(const Vector3f& eyePos) { m_EyePos = eyePos; }
  float& Alpha() { return m_Alpha; }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

protected:
  void DrawSkeletonHands() const;
  std::shared_ptr<GLShader> m_Shader;
  mutable RenderState m_Renderer;
  Vector3f m_EyePos;
  stdvectorV3f m_Palms;
  stdvectorV3f m_Tips;
  float m_Alpha;

private:
  void DrawSkeletonHand(const SkeletonHand& hand) const;
  void DrawCylinder(const Vector3f& p0, const Vector3f& p1, float radius) const;
  void DrawSphere(const Vector3f& p0, float radius) const;

  std::vector<SkeletonHand> m_SkeletonHands;
};
