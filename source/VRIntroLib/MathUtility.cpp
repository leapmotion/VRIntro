#include "stdafx.h"
#include "MathUtility.h"

EigenTypes::Matrix3x3f MathUtility::CrossProductMatrix(const EigenTypes::Vector3f& vector) {
  EigenTypes::Matrix3x3f result;
  CrossProductMatrix(vector, result);
  return result;
}

void MathUtility::CrossProductMatrix(const EigenTypes::Vector3f& vector, EigenTypes::Matrix3x3f &retval) {
  retval << 0.0, -vector.z(), vector.y(),
         vector.z(), 0.0, -vector.x(),
         -vector.y(), vector.x(), 0.0;
}

EigenTypes::Matrix3x3f MathUtility::RotationVectorToMatrix(const EigenTypes::Vector3f& angle_scaled_axis) {
  float angle_squared = angle_scaled_axis.squaredNorm();
  if (angle_squared < 1e-10f) {
    return EigenTypes::Matrix3x3f::Identity();
  }
  const float angle = std::sqrt(angle_squared);
  EigenTypes::Matrix3x3f retval;
  AngleAxisRotationMatrix(angle, angle_scaled_axis, retval);
  return retval;
}

EigenTypes::Vector3f MathUtility::RotationMatrixToVector(const EigenTypes::Matrix3x3f& rotationMatrix) {
  static const float epsilon = 1e-6f;
  const float cs = (rotationMatrix.trace() - 1.0f)*0.5f;
  if (cs > 1.0f - epsilon) {
    return EigenTypes::Vector3f::Zero();
  } else if (cs < epsilon - 1.0f) {
    Eigen::SelfAdjointEigenSolver<EigenTypes::Matrix3x3f> evals(rotationMatrix, Eigen::ComputeEigenvectors);
    EigenTypes::Vector3f rotVector = evals.eigenvectors().col(2).transpose();
    return rotVector.normalized()*(float)M_PI;
  } else {
    const float sn = std::sqrt(1.0f - cs*cs);
    const float angle = std::acos(cs);
    const float multiplier = angle * 0.5f / sn;
    return EigenTypes::Vector3f((rotationMatrix(2, 1) - rotationMatrix(1, 2))*multiplier,
                    (rotationMatrix(0, 2) - rotationMatrix(2, 0))*multiplier,
                    (rotationMatrix(1, 0) - rotationMatrix(0, 1))*multiplier);
  }
}

void MathUtility::AngleAxisRotationMatrix(float angle, const EigenTypes::Vector3f& axis, EigenTypes::Matrix3x3f &retval) {
  EigenTypes::Vector3f axisVec = axis.normalized();

  // use Rodrigues' formula to create the EigenTypes::Matrix
  float cos_angle = std::cos(angle);
  float sin_angle = std::sin(angle);
  // start with the cross product term, calculating it in-place
  CrossProductMatrix(axisVec*sin_angle, retval);
  // add EigenTypes::Matrix3x3f::Identity()*cos_angle to retval
  retval(0, 0) += cos_angle;
  retval(1, 1) += cos_angle;
  retval(2, 2) += cos_angle;
  // add the outer product term -- multiply the scalar factor before forming the outer product
  retval += ((1.0f - cos_angle)*axisVec) * axisVec.transpose();
}

void MathUtility::RotationMatrix_VectorToVector(const EigenTypes::Vector3f& from, const EigenTypes::Vector3f& to, EigenTypes::Matrix3x3f &retval) {
  EigenTypes::Vector3f fromVec = from.normalized();
  EigenTypes::Vector3f toVec = to.normalized();
  EigenTypes::Vector3f axis(fromVec.cross(toVec));
  if (axis.squaredNorm() < 1e-10f) {
    retval.setIdentity();
  } else {
    float angle = std::acos(fromVec.dot(toVec));
    AngleAxisRotationMatrix(angle, axis.normalized(), retval);
  }
}

EigenTypes::Matrix3x3f MathUtility::RotationMatrixLinearInterpolation(const EigenTypes::Matrix3x3f& A0, const EigenTypes::Matrix3x3f& A1, float t) {
  EigenTypes::Vector3f dA = RotationMatrixToVector(A0.transpose()*A1);
  float angle = std::fmod(t*dA.norm(), (float)M_PI);
  EigenTypes::Matrix3x3f At = A0*RotationVectorToMatrix(angle*dA.normalized());
  return At;
}

void MathUtility::RotationMatrixSuppress(EigenTypes::Matrix3x3f& A0, float t) {
  const EigenTypes::Vector3f dA = RotationMatrixToVector(A0);
  A0 = RotationVectorToMatrix(dA * t);
}