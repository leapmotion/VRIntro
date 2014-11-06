#pragma once

#include "RenderableEventHandler.h"
#include "RenderState.h"
#include "Primitives.h"

#include "EigenTypes.h"

#include <memory>

class GLShader;

struct SkeletonHand {

  // Hand Id from Leap API
  int id;

  float confidence;

  // Pinch strength, from API
  float grabStrength;

  // Palm's position
  EigenTypes::Vector3f center;

  // Palm's rotation/basis -- it's reversed for the left hand
  EigenTypes::Matrix3x3f rotationButNotReally;

  //EigenTypes::stdvectorV3f tips[5];
  EigenTypes::Vector3f joints[23];
  EigenTypes::Vector3f jointConnections[23];
  EigenTypes::Vector3f avgExtended;

  EigenTypes::Vector3f getManipulationPoint() const { return 0.5f * (joints[0] + joints[3]); }

  EigenTypes::Matrix3x3f arbitraryRelatedRotation() const;


  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class InteractionLayer : public RenderableEventHandler {
public:
  InteractionLayer(const EigenTypes::Vector3f& initialEyePos, const std::string& shaderName = "material");
  void UpdateEyePos(const EigenTypes::Vector3f& eyePos) { m_EyePos = eyePos; }
  void UpdateEyeView(const EigenTypes::Matrix3x3f& eyeView) { m_EyeView = eyeView; }
  float& Alpha() { return m_Alpha; }
  void SetProjection(const EigenTypes::Matrix4x4f& value) { m_Projection = value; m_Renderer.GetProjection().Matrix() = value.cast<double>(); }
  void SetModelView(const EigenTypes::Matrix4x4f& value) { m_ModelView = value; m_Renderer.GetModelView().Matrix() = value.cast<double>(); }
  void SetFingerRadius(float value) { m_FingerRadius = value; }
  
  virtual void OnSelected() {}

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  // Hack: Making these public for now for easier access
  std::vector<SkeletonHand> m_SkeletonHands;
  EigenTypes::stdvectorV3f m_Palms;
  std::vector<EigenTypes::Matrix3x3f, Eigen::aligned_allocator<EigenTypes::Matrix3x3f>> m_PalmOrientations;
  EigenTypes::stdvectorV3f m_Tips;
  std::vector<bool> m_TipsLeftRight;
  std::vector<bool> m_TipsExtended;
  std::vector<int> m_TipsIndex;

protected:
  void DrawSkeletonHands(bool capsuleMode = false) const;
  mutable RenderState m_Renderer;
  std::shared_ptr<GLShader> m_Shader;
  mutable Sphere m_Sphere;
  mutable Cylinder m_Cylinder;
  mutable Box m_Box;
  float m_FingerRadius;

  EigenTypes::Matrix4x4f m_Projection;
  EigenTypes::Matrix4x4f m_ModelView;
  EigenTypes::Vector3f m_EyePos;
  EigenTypes::Matrix3x3f m_EyeView;

  float m_Alpha;

private:
  void DrawSkeletonHand(const SkeletonHand& hand, float alpha) const;
  void DrawCylinder(const EigenTypes::Vector3f& p0, const EigenTypes::Vector3f& p1, float radius, float alpha) const;
  void DrawSphere(const EigenTypes::Vector3f& p0, float radius, float alpha) const;
};
