#pragma once

#include "RenderableEventHandler.h"
#include "RenderState.h"
#include "Primitives.h"
#include "GLShaderMatrices.h"
#include "GLBuffer.h"

#include "EigenTypes.h"
#include "Leap.h"

#include <memory>

class GLShader;

struct SkeletonHand {
  float confidence;
  Vector3f center;
  //stdvectorV3f tips[5];
  Vector3f joints[21];
  Vector3f jointConnections[21];

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class InteractionLayer : public RenderableEventHandler {
public:
  InteractionLayer(const Vector3f& initialEyePos, const std::string& shaderName = "material");
  void UpdateLeap(const Leap::Frame& frame, const Matrix4x4f& worldTransform);
  void UpdateEyePos(const Vector3f& eyePos) { m_EyePos = eyePos; }
  void UpdateEyeView(const Matrix3x3f& eyeView) { m_EyeView = eyeView; }
  float& Alpha() { return m_Alpha; }
  void SetProjection(const Matrix4x4f& value) { m_Projection = value; m_Renderer.GetProjection().Matrix() = value.cast<double>(); }
  void SetModelView(const Matrix4x4f& value) { m_ModelView = value; m_Renderer.GetModelView().Matrix() = value.cast<double>(); }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

protected:
  void DrawSkeletonHands() const;
  mutable RenderState m_Renderer;
  std::shared_ptr<GLShader> m_Shader;
  mutable Sphere m_Sphere;
  mutable Cylinder m_Cylinder;

  Matrix4x4f m_Projection;
  Matrix4x4f m_ModelView;
  Vector3f m_EyePos;
  Matrix3x3f m_EyeView;
  stdvectorV3f m_Palms;
  std::vector<Matrix3x3f, Eigen::aligned_allocator<Matrix3x3f>> m_PalmOrientations;
  stdvectorV3f m_Tips;
  std::vector<bool> m_TipsLeftRight;
  std::vector<bool> m_TipsExtended;
  std::vector<int> m_TipsIndex;
  float m_Alpha;

private:
  void DrawSkeletonHand(const SkeletonHand& hand, float alpha) const;
  void DrawCylinder(const Vector3f& p0, const Vector3f& p1, float radius, float alpha) const;
  void DrawSphere(const Vector3f& p0, float radius, float alpha) const;

  std::vector<SkeletonHand> m_SkeletonHands;
};
