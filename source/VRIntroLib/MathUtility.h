#pragma once

#include "EigenTypes.h"

class MathUtility {
public:
  static EigenTypes::Matrix3x3f CrossProductMatrix(const EigenTypes::Vector3f& vector);
  static void CrossProductMatrix(const EigenTypes::Vector3f& vector, EigenTypes::Matrix3x3f &retval);
  static EigenTypes::Matrix3x3f RotationVectorToMatrix(const EigenTypes::Vector3f& angle_scaled_axis);
  static EigenTypes::Vector3f RotationMatrixToVector(const EigenTypes::Matrix3x3f& rotationMatrix);
  static void AngleAxisRotationMatrix(float angle, const EigenTypes::Vector3f& axis, EigenTypes::Matrix3x3f &retval);
  static void RotationMatrix_VectorToVector(const EigenTypes::Vector3f& from, const EigenTypes::Vector3f& to, EigenTypes::Matrix3x3f &retval);
  static EigenTypes::Matrix3x3f RotationMatrixLinearInterpolation(const EigenTypes::Matrix3x3f& A0, const EigenTypes::Matrix3x3f& A1, float t);
  static void RotationMatrixSuppress(EigenTypes::Matrix3x3f& A0, float t);
};
