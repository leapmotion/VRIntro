#include "FlyingLayer.h"

#include "Primitives.h"
#include "GLController.h"

FlyingLayer::FlyingLayer(const Vector3f& initialEyePos) :
  InteractionLayer(initialEyePos),
  m_GridCenter(initialEyePos),
  m_Velocity(Vector3f::Zero()),
  m_RotationAA(Vector3f::Zero()),
  m_GridOrientation(Matrix4x4f::Identity()),
  m_LineThickness(3.0f),
  m_GridBrightness(80) {
  // TODO: switch to non-default shader
}

void FlyingLayer::Update(TimeDelta real_time_delta) {
  static const float PERIOD_TRANS = 0.0045f;
  static const float PERIOD_ROT = 1.0f;
  static const float FILTER = 0.2f;

  if (m_Palms.size() > 0) {

    Vector3f positionSum = Vector3f::Zero();
    Vector3f rotationAASum = Vector3f::Zero();
    for (int i = 0; i < m_Palms.size(); i++) {
      positionSum += m_GridOrientation.block<3, 3>(0, 0).transpose()*(m_Palms[i] - m_EyePos - m_EyeView.transpose()*Vector3f(0, -0.15, -0.05));
      //rotationAASum += RotationMatrixToVector(m_EyeView.transpose()*m_PalmOrientations[i]*m_EyeView.transpose());
      Matrix3x3f rot;
      RotationMatrix_VectorToVector(-Vector3f::UnitZ(), m_EyeView*(m_Palms[i] - m_EyePos) - Vector3f(0, -0.15, -0.05), rot);
      //std::cout << __LINE__ << ":\t       rot = " << (rot) << std::endl;
      rotationAASum += RotationMatrixToVector(rot);
    }
    if (m_Palms.size() == 2) {
      const Vector3f dir0 = m_EyeView*(m_Palms[0] - m_EyePos) - Vector3f(0, -0.15, -0.05);
      const Vector3f dir1 = m_EyeView*(m_Palms[1] - m_EyePos) - Vector3f(0, -0.15, -0.05);

      Matrix3x3f rot;
      RotationMatrix_VectorToVector((dir0.x() < dir1.x() ? 1.0f : -1.0f) * Vector3f::UnitX(), dir1 - dir0, rot);
      //std::cout << __LINE__ << ":\t     positionSum = " << (positionSum) << std::endl;

      rotationAASum += 2.0f*RotationMatrixToVector(rot);
    }
    m_Velocity = (1 - FILTER)*m_Velocity + FILTER*positionSum/m_Palms.size();
    m_RotationAA = (1 - FILTER)*m_RotationAA + FILTER*rotationAASum/m_Palms.size();
  } else {
    m_Velocity = (1 - 0.3f*FILTER)*m_Velocity;
    m_RotationAA = (1 - 0.3f*FILTER)*m_RotationAA;
  }

  m_GridCenter -= m_Velocity*m_Velocity.squaredNorm()*(real_time_delta/PERIOD_TRANS);
  const Matrix3x3f rot = RotationVectorToMatrix((real_time_delta/PERIOD_ROT)*m_RotationAA*m_RotationAA.squaredNorm());
  //std::cout << __LINE__ << ":\t   rot = " << (rot) << std::endl;
  //Matrix3x3f foo = ;
  m_GridOrientation.block<3, 3>(0, 0) = m_EyeView.transpose()*rot.transpose()*m_EyeView*m_GridOrientation.block<3, 3>(0, 0);
}

void FlyingLayer::Render(TimeDelta real_time_delta) const {
  // Draw joystick
  m_Shader->Bind();
  const Vector3f desiredLightPos(0, 10, -10);
  const Vector3f lightPos = m_EyeView.transpose()*desiredLightPos + m_EyePos;
  const int lightPosLoc = m_Shader->LocationOfUniform("lightPosition");
  glUniform3f(lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);

  Sphere sphere;
  sphere.SetRadius(0.3f);
  sphere.Translation() = (m_EyeView.transpose()*Vector3f(0, 0, 1.25) + m_EyePos).cast<double>();
  sphere.SetDiffuseColor(Color(1.0f, 0.5f, 0.4f, m_Alpha));
  sphere.SetAmbientFactor(0.3f);
  PrimitiveBase::DrawSceneGraph(sphere, m_Renderer);
  m_Shader->Unbind();

  glBegin(GL_LINES);
  glVertex3fv(Vector3f::Zero().eval().data());
  glVertex3fv(m_Velocity.eval().data());
  glEnd();

  //glMatrixMode(GL_MODELVIEW);
  Vector3f centerpoint = m_EyePos - m_GridCenter;
  //glScalef(2.0f, 2.0f, 2.0f);
  glMultMatrixf(m_GridOrientation.eval().data());
  glTranslatef(m_GridCenter.x(), m_GridCenter.y(), m_GridCenter.z());

  int xShift = 2.0f*static_cast<int>(0.5f*centerpoint.x() + 0.5f);
  int yShift = 20.0f*static_cast<int>(0.05f*centerpoint.y() + 0.5f);
  int zShift = 2.0f*static_cast<int>(0.5f*centerpoint.z() + 0.5f);
  glLineWidth(m_LineThickness);
  glBegin(GL_LINES);
  for (int i = -60 + xShift; i < 60 + xShift; i+=2) {
    for (int j = -50 + yShift; j < 70 + yShift; j+=20) {
      for (int k = -60 + zShift; k < 60 + zShift; k+=2) {
        Vector3f a(static_cast<float>(i), static_cast<float>(j), static_cast<float>(k));
        Vector3f b(static_cast<float>(i + 2), static_cast<float>(j), static_cast<float>(k));
        //Vector3f c(static_cast<float>(i), static_cast<float>(j + 2), static_cast<float>(k));
        Vector3f d(static_cast<float>(i), static_cast<float>(j), static_cast<float>(k + 2));
        float aColor = std::min(1.0f, static_cast<float>(m_GridBrightness)/(20.0f + (a - centerpoint).squaredNorm()));
        float bColor = std::min(1.0f, static_cast<float>(m_GridBrightness)/(20.0f + (b - centerpoint).squaredNorm()));
        //float cColor = std::min(1.1f9 100.02/(10.0f + (c - centerpoint).squaredNorm()));
        float dColor = std::min(1.0f, static_cast<float>(m_GridBrightness)/(20.0f + (d - centerpoint).squaredNorm()));

        glColor4f(1.0f, 1.0f, 1.0f, m_Alpha*aColor);
        glVertex3fv((a).eval().data());

        glColor4f(1.0f, 1.0f, 1.0f, m_Alpha*bColor);
        glVertex3fv((b).eval().data());

        //glColor4f(1.0f, 1.0f, 1.0f, aColor);
        //glVertex3fv((a).eval().data());

        //glColor4f(1.0f, 1.0f, 1.0f, cColor);
        //glVertex3fv((c).eval().data());

        glColor4f(1.0f, 1.0f, 1.0f, m_Alpha*aColor);
        glVertex3fv((a).eval().data());

        glColor4f(1.0f, 1.0f, 1.0f, m_Alpha*dColor);
        glVertex3fv((d).eval().data());
      }
    }
  }
  glEnd();
}

EventHandlerAction FlyingLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  switch (ev.keysym.sym) {
  case 'g':
    if (SDL_GetModState() & KMOD_SHIFT) {
      m_GridBrightness = std::min(300, m_GridBrightness + 10);
    } else {
      m_GridBrightness = std::max(10, m_GridBrightness - 10);
    }
    return EventHandlerAction::CONSUME;
  case 'l':
    if (SDL_GetModState() & KMOD_SHIFT) {
      m_LineThickness = std::min(10.0f, m_LineThickness + 0.5f);
    } else {
      m_LineThickness = std::max(0.5f, m_LineThickness - 0.5f);
    }
    return EventHandlerAction::CONSUME;
  default:
    return EventHandlerAction::PASS_ON;
  }
}

Matrix3x3f FlyingLayer::CrossProductMatrix(const Vector3f& vector) const {
  Matrix3x3f result;
  CrossProductMatrix(vector, result);
  return result;
}

void FlyingLayer::CrossProductMatrix(const Vector3f& vector, Matrix3x3f &retval) const {
  retval << 0.0, -vector.z(), vector.y(),
         vector.z(), 0.0, -vector.x(),
         -vector.y(), vector.x(), 0.0;
}

Matrix3x3f FlyingLayer::RotationVectorToMatrix(const Vector3f& angle_scaled_axis) const {
  float angle_squared = angle_scaled_axis.squaredNorm();
  if (angle_squared < 1e-10f) {
    return Matrix3x3f::Identity();
  }
  const float angle = std::sqrt(angle_squared);
  Matrix3x3f retval;
  AngleAxisRotationMatrix(angle, angle_scaled_axis, retval);
  return retval;
}

Vector3f FlyingLayer::RotationMatrixToVector(const Matrix3x3f& rotationMatrix) const {
  static const float epsilon = 1e-6;
  const float cs = (rotationMatrix.trace() - 1.0)*0.5;
  if (cs > 1.0 - epsilon) {
    return Vector3f::Zero();
  } else if (cs < epsilon - 1.0) {
    Eigen::SelfAdjointEigenSolver<Matrix3x3f> evals(rotationMatrix, Eigen::ComputeEigenvectors);
    Vector3f rotVector = evals.eigenvectors().col(2).transpose();
    return rotVector.normalized()*M_PI;
  } else {
    const float sn = std::sqrt(1.0 - cs*cs);
    const float angle = std::acos(cs);
    const float multiplier = angle * 0.5 / sn;
    return Vector3f((rotationMatrix(2, 1) - rotationMatrix(1, 2))*multiplier,
                    (rotationMatrix(0, 2) - rotationMatrix(2, 0))*multiplier,
                    (rotationMatrix(1, 0) - rotationMatrix(0, 1))*multiplier);
  }
}

void FlyingLayer::AngleAxisRotationMatrix(float angle, const Vector3f& axis, Matrix3x3f &retval) const {
  Vector3f axisVec = axis.normalized();

  // use Rodrigues' formula to create the matrix
  float cos_angle = std::cos(angle);
  float sin_angle = std::sin(angle);
  // start with the cross product term, calculating it in-place
  CrossProductMatrix(axisVec*sin_angle, retval);
  // add Matrix3x3f::Identity()*cos_angle to retval
  retval(0, 0) += cos_angle;
  retval(1, 1) += cos_angle;
  retval(2, 2) += cos_angle;
  // add the outer product term -- multiply the scalar factor before forming the outer product
  retval += ((1.0 - cos_angle)*axisVec) * axisVec.transpose();
}

void FlyingLayer::RotationMatrix_VectorToVector(const Vector3f& from, const Vector3f& to, Matrix3x3f &retval) const {
  Vector3f fromVec = from.normalized();
  Vector3f toVec = to.normalized();
  Vector3f axis(fromVec.cross(toVec));
  if (axis.squaredNorm() < 1e-10f) {
    retval.setIdentity();
  } else {
    float angle = std::acos(fromVec.dot(toVec));
    AngleAxisRotationMatrix(angle, axis.normalized(), retval);
  }
}

Matrix3x3f FlyingLayer::RotationMatrixLinearInterpolation(const Matrix3x3f& A0, const Matrix3x3f& A1, float t) const {
  Vector3f dA = RotationMatrixToVector(A0.transpose()*A1);
  float angle = std::fmod(t*dA.norm(), M_PI);
  Matrix3x3f At = A0*RotationVectorToMatrix(angle*dA.normalized());
  return At;
}

void FlyingLayer::RotationMatrixSuppress(Matrix3x3f& A0, float t) const {
  assert(t >= 0.0 && t <= 1.0);
  const Vector3f dA = RotationMatrixToVector(A0);
  A0 = RotationVectorToMatrix(dA * t);
}