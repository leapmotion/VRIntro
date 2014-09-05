#pragma once

#include "Interactionlayer.h"

class GLShader;

class FlyingLayer : public InteractionLayer {
public:
  FlyingLayer(const Vector3f& initialEyePos);
  //virtual ~FlyingLayer ();

  Matrix3x3f CrossProductMatrix(const Vector3f& vector) const;
  void CrossProductMatrix(const Vector3f& vector, Matrix3x3f &retval) const;
  Matrix3x3f RotationVectorToMatrix(const Vector3f& angle_scaled_axis) const;
  Vector3f RotationMatrixToVector(const Matrix3x3f& rotationMatrix) const;
  void AngleAxisRotationMatrix(float angle, const Vector3f& axis, Matrix3x3f &retval) const;
  void RotationMatrix_VectorToVector(const Vector3f& from, const Vector3f& to, Matrix3x3f &retval) const;
  Matrix3x3f RotationMatrixLinearInterpolation(const Matrix3x3f& A0, const Matrix3x3f& A1, float t) const;
  void RotationMatrixSuppress(Matrix3x3f& A0, float t) const;

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  Vector3f m_GridCenter;
  //Vector3f m_AveragePalm;
  Vector3f m_Velocity;
  Vector3f m_RotationAA;
  Matrix4x4f m_GridOrientation;
  float m_LineThickness;
  int m_GridBrightness;
};
