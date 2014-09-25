#include "stdafx.h"
#include "GridLayer.h"

#include "GLController.h"
#include <math.h>

GridLayer::GridLayer(const EigenTypes::Vector3f& initialEyePos) :
  InteractionLayer(initialEyePos),
  m_DivTheta(22),
  m_DivPhi(40),
  m_Radius(0.7f) {

}

void GridLayer::Update(TimeDelta real_time_delta) {

}

void GridLayer::Render(TimeDelta real_time_delta) const {
  glTranslatef(m_EyePos.x(), m_EyePos.y(), m_EyePos.z());
  glColor4f(0.2f, 0.6f, 1.0f, m_Alpha*0.5f);
  glLineWidth(1.0f);
  glBegin(GL_LINES);
  for (int i = 0; i < m_DivPhi; i++) {
    float phi0 = (float)M_PI*(i/static_cast<float>(m_DivPhi) - 0.5f);
    float phi1 = (float)M_PI*((i + 1)/static_cast<float>(m_DivPhi) - 0.5f);
    for (int j = 0; j < m_DivTheta; j++) {
      float theta0 = 2*(float)M_PI*(j/static_cast<float>(m_DivTheta));
      float theta1 = 2*(float)M_PI*((j + 1)/static_cast<float>(m_DivTheta));
      glVertex3f(m_Radius*cos(phi0)*cos(theta0), m_Radius*sin(phi0), m_Radius*cos(phi0)*sin(theta0));
      glVertex3f(m_Radius*cos(phi0)*cos(theta1), m_Radius*sin(phi0), m_Radius*cos(phi0)*sin(theta1));
      glVertex3f(m_Radius*cos(phi0)*cos(theta0), m_Radius*sin(phi0), m_Radius*cos(phi0)*sin(theta0));
      glVertex3f(m_Radius*cos(phi1)*cos(theta0), m_Radius*sin(phi1), m_Radius*cos(phi1)*sin(theta0));
    }
  }
  glEnd();
}

EventHandlerAction GridLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  if (ev.type == SDL_KEYDOWN) {
    switch (ev.keysym.sym) {
    case 'p':
      if (SDL_GetModState() & KMOD_SHIFT) {
        m_DivPhi = std::min(100, m_DivPhi + 2);
      } else {
        m_DivPhi = std::max(4, m_DivPhi - 2);
      }
      return EventHandlerAction::CONSUME;
    case 't':
      if (SDL_GetModState() & KMOD_SHIFT) {
        m_DivTheta = std::min(200, m_DivTheta + 4);
      } else {
        m_DivTheta = std::max(8, m_DivTheta - 4);
      }
      return EventHandlerAction::CONSUME;
    case 'r':
      if (SDL_GetModState() & KMOD_SHIFT) {
        m_Radius = std::min(20.0f, m_Radius * 1.03f);
      } else {
        m_Radius = std::max(0.1f, m_Radius * 0.97087378f);
      }
      return EventHandlerAction::CONSUME;
    default:
      return EventHandlerAction::PASS_ON;
    }
  }
  return EventHandlerAction::PASS_ON;
}
