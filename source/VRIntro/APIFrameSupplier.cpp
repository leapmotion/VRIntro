#include "stdafx.h"

#include "VRIntroLib/PassthroughLayer.h"
#include "APIFrameSupplier.h"

#include "Leap.h"

APIFrameSupplier::APIFrameSupplier() {
  m_LeapController.addListener(m_LeapListener);
  m_LeapController.setPolicyFlags(static_cast<Leap::Controller::PolicyFlag>(Leap::Controller::POLICY_IMAGES | Leap::Controller::POLICY_OPTIMIZE_HMD));
}

APIFrameSupplier::~APIFrameSupplier() {
  m_LeapController.removeListener(m_LeapListener);
}

void APIFrameSupplier::PopulateInteractionLayer(InteractionLayer& target, const float* worldTransformRaw) const {
  const Leap::Frame& frame = m_LeapController.frame();

  target.m_Tips.clear();
  target.m_TipsLeftRight.clear();
  target.m_TipsExtended.clear();
  target.m_TipsIndex.clear();
  target.m_Palms.clear();
  target.m_PalmOrientations.clear();
  target.m_SkeletonHands.clear();
  EigenTypes::Matrix4x4f worldTransform = EigenTypes::Matrix4x4f(worldTransformRaw);
  EigenTypes::Matrix3x3f rotation = worldTransform.block<3, 3>(0, 0);
  EigenTypes::Vector3f translation = worldTransform.block<3, 1>(0, 3);

  for (int i = 0; i < frame.hands().count(); i++) {
    const Leap::Hand& hand = frame.hands()[i];
    SkeletonHand outHand;
    outHand.id = hand.id();
    outHand.confidence = hand.confidence();
    outHand.grabStrength = hand.grabStrength();

    const EigenTypes::Vector3f palm = rotation*hand.palmPosition().toVector3<EigenTypes::Vector3f>() + translation;
    const EigenTypes::Vector3f palmDir = (rotation*hand.direction().toVector3<EigenTypes::Vector3f>()).normalized();
    const EigenTypes::Vector3f palmNormal = (rotation*hand.palmNormal().toVector3<EigenTypes::Vector3f>()).normalized();
    const EigenTypes::Vector3f palmSide = palmDir.cross(palmNormal).normalized();

    const EigenTypes::Matrix3x3f palmRotation = rotation*(EigenTypes::Matrix3x3f(hand.basis().toArray3x3()))*rotation.inverse();
    EigenTypes::Matrix3x3f palmBasis = rotation*(EigenTypes::Matrix3x3f(hand.basis().toArray3x3()));

    // Remove scale from palmBasis
    const float basisScale = (palmBasis * EigenTypes::Vector3f::UnitX()).norm();
    palmBasis *= 1.0f / basisScale;

    outHand.center = palm;
    outHand.rotationButNotReally = palmBasis;
    target.m_Palms.push_back(palm);
    target.m_PalmOrientations.push_back(palmRotation);
    EigenTypes::Vector3f sumExtended = EigenTypes::Vector3f::Zero();
    int numExtended = 0;

    for (int j = 0; j < 5; j++) {
      const Leap::Finger& finger = hand.fingers()[j];
      target.m_Tips.push_back(rotation*finger.tipPosition().toVector3<EigenTypes::Vector3f>() + translation);
      target.m_TipsExtended.push_back(hand.grabStrength() > 0.9f || finger.isExtended());
      if (target.m_TipsExtended.back()) {
        sumExtended += target.m_Tips.back();
        numExtended++;
      }
      target.m_TipsLeftRight.push_back(hand.isRight());
      target.m_TipsIndex.push_back(j);

      for (int k = 0; k < 3; k++) {
        Leap::Bone bone = finger.bone(static_cast<Leap::Bone::Type>(k + 1));
        outHand.joints[j*3 + k] = rotation*bone.nextJoint().toVector3<EigenTypes::Vector3f>() + translation;
        outHand.jointConnections[j*3 + k] = rotation*bone.prevJoint().toVector3<EigenTypes::Vector3f>() + translation;
      }
    }
    outHand.avgExtended = numExtended == 0 ? palm : (const EigenTypes::Vector3f)(sumExtended/numExtended);

    const float thumbDist = (outHand.jointConnections[0] - palm).norm();
    const EigenTypes::Vector3f wrist = palm - thumbDist*(palmDir*0.8f + static_cast<float>(hand.isLeft() ? -1 : 1)*palmSide*0.5f);

    for (int j = 0; j < 4; j++) {
      outHand.joints[15 + j] = outHand.jointConnections[3 * j];
      outHand.jointConnections[15 + j] = outHand.jointConnections[3 * (j + 1)];
    }
    outHand.joints[19] = outHand.jointConnections[12];
    outHand.jointConnections[19] = wrist;
    outHand.joints[20] = wrist;
    outHand.jointConnections[20] = outHand.jointConnections[0];

    // Arm
    const EigenTypes::Vector3f elbow = rotation*hand.arm().elbowPosition().toVector3<EigenTypes::Vector3f>() + translation;
    outHand.joints[21] = elbow - thumbDist*(hand.isLeft() ? -1 : 1)*palmSide*0.5;
    outHand.jointConnections[21] = wrist;
    outHand.joints[22] = elbow + thumbDist*(hand.isLeft() ? -1 : 1)*palmSide*0.5;
    outHand.jointConnections[22] = outHand.jointConnections[0];

    target.m_SkeletonHands.push_back(outHand);
  }
}

void APIFrameSupplier::PopulatePassthroughLayer(PassthroughLayer& target, int i, bool useLatestImage) const {
  // Set passthrough images
  const Leap::ImageList& images = useLatestImage ? m_LeapController.images() : m_LeapController.frame().images();
  if (images.count() == 2) {
    if (images[i].width() == 640) {
      target.SetImage(images[i].data(), images[i].width(), images[i].height());
    } else {
      target.SetColorImage(images[i].data());
    }
    target.SetDistortion(images[i].distortion());
  }
}

bool APIFrameSupplier::IsDragonfly() const {
  const Leap::ImageList& images = m_LeapController.images();

  return images.count() == 2 && images[0].width() != 640;
}

double APIFrameSupplier::GetFPSEstimate() const {
  return m_LeapController.frame().currentFramesPerSecond();
}
