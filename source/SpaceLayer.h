#pragma once

#include "Interactionlayer.h"

class GLShader;

class SpaceLayer : public InteractionLayer {
public:
  SpaceLayer(const Vector3f& initialEyePos);
  virtual ~SpaceLayer();

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  static const int NUM_GALAXIES = 1;
  static const int STARS_PER = 8000;
  static const int NUM_STARS = STARS_PER*NUM_GALAXIES;

  void InitPhysics();
  Vector3 GenerateVector(const Vector3& center, double radius);
  Vector3 InitialVelocity(double mass, const Vector3& normal, const Vector3& dr);
  void UpdateV(int type, const Vector3& p, Vector3& v, int galaxy);
  void UpdateAllPhysics();
  void RenderPopup() const;

  mutable GLBuffer m_Buffer;
  mutable GLBuffer m_PopupBuffer;
  std::shared_ptr<GLTexture2> m_PopupTexture;
  std::shared_ptr<GLShader> m_PopupShader;

  Vector3 m_GalaxyPos[NUM_GALAXIES];
  Vector3 m_GalaxyVel[NUM_GALAXIES];
  Vector3 m_GalaxyNormal[NUM_GALAXIES];
  double m_GalaxyMass[NUM_GALAXIES];

  stdvectorV3 pos;
  stdvectorV3 vel;

  float *m_Buf;

  static float buf[NUM_STARS];
  int m_OddEven;
};
