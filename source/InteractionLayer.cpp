#include "InteractionLayer.h"

#include "Primitives.h"
#include "Resource.h"
#include "GLShader.h"
#include "GLShaderLoader.h"

InteractionLayer::InteractionLayer(const std::string& shaderName) :
  m_Shader(Resource<GLShader>(shaderName)),
  m_Alpha(0.0f) {
  m_Renderer.SetShader(m_Shader);
}

void InteractionLayer::UpdateLeap(const Leap::Frame& frame, const Matrix4x4f& worldTransform) {
  m_Tips.clear();
  m_Palms.clear();
  m_SkeletonHands.clear();
  Matrix3x3f rotation = worldTransform.block<3, 3>(0, 0);
  Vector3f translation = worldTransform.block<3, 1>(0, 3);

  for (int i = 0; i < frame.hands().count(); i++) {
    SkeletonHand outHand;
    const Leap::Hand& hand = frame.hands()[i];
    const Vector3f palm = rotation*hand.palmPosition().toVector3<Vector3f>() + translation;
    const Vector3f palmDir = rotation*hand.direction().toVector3<Vector3f>();
    const Vector3f palmNormal = rotation*hand.palmNormal().toVector3<Vector3f>();
    const Vector3f palmSide = palmDir.cross(palmNormal).normalized();
    m_Palms.push_back(palm);

    for (int j = 0; j < 5; j++) {
      const Leap::Finger& finger = hand.fingers()[j];
      m_Tips.push_back(rotation*finger.tipPosition().toVector3<Vector3f>() + translation);

      for (int k = 0; k < 3; k++) {
        Leap::Bone bone = finger.bone(static_cast<Leap::Bone::Type>(k + 1));
        outHand.joints[j*3 + k] = rotation*bone.nextJoint().toVector3<Vector3f>() + translation;
        outHand.jointConnections[j*3 + k] = rotation*bone.prevJoint().toVector3<Vector3f>() + translation;
      }
    }
    const float thumbDist = (outHand.jointConnections[0] - palm).norm();
    //const Vector3f wrist = palm - thumbDist*(palmDir*0.90 + (hand.isLeft() ? -1 : 1)*palmSide*0.5);
    const Vector3f wrist = rotation*hand.fingers()[4].bone(static_cast<Leap::Bone::Type>(0)).prevJoint().toVector3<Vector3f>() + translation;

    for (int j = 0; j < 4; j++) {
      outHand.joints[15 + j] = outHand.jointConnections[3 * j];
      outHand.jointConnections[15 + j] = outHand.jointConnections[3 * (j + 1)];
    }
    outHand.joints[19] = outHand.jointConnections[12];
    outHand.jointConnections[19] = wrist;
    outHand.joints[20] = wrist;
    outHand.jointConnections[20] = outHand.jointConnections[0];

    m_SkeletonHands.push_back(outHand);
  }
}

void InteractionLayer::DrawSkeletonHands() const {
  m_Shader->Bind();
  const Vector3f desiredLightPos(0, 10, 10);
  const Vector3f lightPos = desiredLightPos - m_EyePos.cast<float>();
  const int lightPosLoc = m_Shader->LocationOfUniform("lightPosition");
  glUniform3f(lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);

  for (size_t i = 0; i < m_SkeletonHands.size(); i++) {
    DrawSkeletonHand(m_SkeletonHands[i]);
  }
  m_Shader->Unbind();
}

void InteractionLayer::DrawSkeletonHand(const SkeletonHand& hand) const {
  for (int i = 0; i < 21; i++) {
    DrawSphere(hand.joints[i], 10e-3f);
    DrawCylinder(hand.joints[i], hand.jointConnections[i], 7e-3f);
  }
}

//std::shared_ptr<GLShader> m_Shader;
//mutable RenderState m_Renderer;
//Vector3f m_EyePos;

void InteractionLayer::DrawCylinder(const Vector3f& p0, const Vector3f& p1, float radius) const {
  Cylinder cylinder;
  cylinder.SetRadius(static_cast<double>(radius));
  cylinder.Translation() = 0.5*(p0 + p1).cast<double>();

  Vector3f direction = p1 - p0;
  const float length = direction.norm();
  cylinder.SetHeight(length);
  direction /= length;

  Vector3f Y = direction;
  Vector3f X = Y.cross(Vector3f::UnitZ()).normalized();
  Vector3f Z = Y.cross(X).normalized();

  Matrix3x3f basis;
  basis << X, Y, Z;
  cylinder.LinearTransformation() = basis.cast<double>();

  cylinder.SetDiffuseColor(Color(0.85f, 0.85f, 0.85f, m_Alpha));
  cylinder.SetAmbientFactor(0.3f);
  PrimitiveBase::DrawSceneGraph(cylinder, m_Renderer);
}

void InteractionLayer::DrawSphere(const Vector3f& p0, float radius) const {
  Sphere sphere;
  sphere.SetRadius(static_cast<double>(radius));
  sphere.Translation() = p0.cast<double>();
  sphere.SetDiffuseColor(Color(0.4f, 0.6f, 1.0f, m_Alpha));
  sphere.SetAmbientFactor(0.3f);
  PrimitiveBase::DrawSceneGraph(sphere, m_Renderer);
}
