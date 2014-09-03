#pragma once

#include "Interactionlayer.h"

class GLShader;

class SpaceLayer : public InteractionLayer {
public:
  SpaceLayer();
  //virtual ~SpaceLayer ();

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  static const int NUM_GALAXIES = 3;
  static const int STARS_PER = 5000;
  static const int NUM_STARS = STARS_PER*NUM_GALAXIES;

  Vector3 m_GalaxyPos[NUM_GALAXIES];
  Vector3 m_GalaxyVel[NUM_GALAXIES];
  Vector3 m_GalaxyNormal[NUM_GALAXIES];
  double m_GalaxyMass[NUM_GALAXIES];

  stdvectorV3 pos;
  stdvectorV3 vel;

  void InitPhysics();
  Vector3 GenerateVector(const Vector3& center, double radius);
  Vector3 InitialVelocity(double mass, const Vector3& normal, const Vector3& dr);
  void UpdateV(const Vector3& p, Vector3& v, int galaxy);
  void UpdateAllPhysics();
};
