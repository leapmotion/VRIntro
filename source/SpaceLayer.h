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
  static const int STARS_PER = 10000;
  static const int NUM_STARS = STARS_PER*NUM_GALAXIES;

  void InitPhysics();
  Vector3f GenerateVector(const Vector3f& center, float radius);
  Vector3f InitialVelocity(float mass, const Vector3f& normal, const Vector3f& dr);
  void UpdateV(int type, const Vector3f& p, Vector3f& v, int galaxy);
  void UpdateAllPhysics();
  void RenderPopup() const;

  mutable GLBuffer m_Buffer;
  mutable GLBuffer m_PopupBuffer;
  std::shared_ptr<GLTexture2> m_PopupTexture;
  std::shared_ptr<GLShader> m_PopupShader;

  Vector3f m_GalaxyPos[NUM_GALAXIES];
  Vector3f m_GalaxyVel[NUM_GALAXIES];
  Vector3f m_GalaxyNormal[NUM_GALAXIES];
  float m_GalaxyMass[NUM_GALAXIES];

  stdvectorV3f pos;
  stdvectorV3f vel;

  float *m_Buf;

  static float buf[NUM_STARS];
  int m_OddEven;
};
