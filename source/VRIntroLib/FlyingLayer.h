#pragma once

#include "Interactionlayer.h"

class GLShader;

class FlyingLayer : public InteractionLayer {
public:
  FlyingLayer(const EigenTypes::Vector3f& initialEyePos);
  //virtual ~FlyingLayer ();

  EigenTypes::Matrix3x3f CrossProductMatrix(const EigenTypes::Vector3f& vector) const;
  void CrossProductMatrix(const EigenTypes::Vector3f& vector, EigenTypes::Matrix3x3f &retval) const;
  EigenTypes::Matrix3x3f RotationVectorToMatrix(const EigenTypes::Vector3f& angle_scaled_axis) const;
  EigenTypes::Vector3f RotationMatrixToVector(const EigenTypes::Matrix3x3f& rotationMatrix) const;
  void AngleAxisRotationMatrix(float angle, const EigenTypes::Vector3f& axis, EigenTypes::Matrix3x3f &retval) const;
  void RotationMatrix_VectorToVector(const EigenTypes::Vector3f& from, const EigenTypes::Vector3f& to, EigenTypes::Matrix3x3f &retval) const;
  EigenTypes::Matrix3x3f RotationMatrixLinearInterpolation(const EigenTypes::Matrix3x3f& A0, const EigenTypes::Matrix3x3f& A1, float t) const;
  void RotationMatrixSuppress(EigenTypes::Matrix3x3f& A0, float t) const;

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  mutable GLBuffer m_PopupBuffer;
  std::shared_ptr<GLTexture2> m_PopupTexture;
  std::shared_ptr<GLShader> m_PopupShader;

  void RenderPopup() const;

  EigenTypes::Vector3f m_GridCenter;
  //EigenTypes::Vector3f m_AveragePalm;
  EigenTypes::Vector3f m_Velocity;
  EigenTypes::Vector3f m_RotationAA;
  EigenTypes::Matrix4x4f m_GridOrientation;
  float m_LineThickness;
  int m_GridBrightness;
};
