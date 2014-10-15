#pragma once

#include "Interactionlayer.h"

class GLShader;

class SpaceLayer : public InteractionLayer {
public:
  SpaceLayer(const EigenTypes::Vector3f& initialEyePos);
  virtual ~SpaceLayer();

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  static const int NUM_GALAXIES = 1;
  static const int STARS_PER = 30000;
  static const int NUM_STARS = STARS_PER*NUM_GALAXIES;

  void InitPhysics();
  EigenTypes::Vector3f GenerateVector(const EigenTypes::Vector3f& center, float radius);
  EigenTypes::Vector3f InitialVelocity(float mass, const EigenTypes::Vector3f& normal, const EigenTypes::Vector3f& dr);
  void UpdateV(int type, const EigenTypes::Vector3f& p, EigenTypes::Vector3f& v, int galaxy);
  void UpdateAllPhysics();
  void RenderPopup() const;

  mutable GLBuffer m_Buffer;
  mutable GLBuffer m_PopupBuffer;
  std::shared_ptr<GLTexture2> m_PopupTexture;
  std::shared_ptr<GLShader> m_PopupShader;

  EigenTypes::Vector3f m_GalaxyPos[NUM_GALAXIES];
  EigenTypes::Vector3f m_GalaxyVel[NUM_GALAXIES];
  EigenTypes::Vector3f m_GalaxyNormal[NUM_GALAXIES];
  float m_GalaxyMass[NUM_GALAXIES];

  EigenTypes::stdvectorV3f pos;
  EigenTypes::stdvectorV3f vel;

  float *m_Buf;

  static float buf[NUM_STARS];
  int m_OddEven;

  int m_StarShowMode;
  int m_StarsToShow;
};
