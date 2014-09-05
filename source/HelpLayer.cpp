#include "HelpLayer.h"

#include "GLController.h"
#include "Resource.h"
#include "GLTexture2.h"
#include "GLTexture2Loader.h"

HelpLayer::HelpLayer(const Vector3f& initialEyePos) :
  InteractionLayer(Vector3f::Zero(), "transparent"),
  m_texture(Resource<GLTexture2>("help.png")) {
  // TODO: switch to non-default shader
}

void HelpLayer::Update(TimeDelta real_time_delta) {

}

void HelpLayer::Render(TimeDelta real_time_delta) const {
  glClear(GL_DEPTH_BUFFER_BIT);

  m_Shader->Bind();
  m_Renderer.GetModelView().Matrix().block<3, 1>(0, 3) = Vector3::Zero();
  m_Renderer.UploadMatrices();

  glUniform2f(m_Shader->LocationOfUniform("ray_scale"), 1.25f, 1.0606f);
  glUniform2f(m_Shader->LocationOfUniform("ray_offset"), 0.5f, 0.5f);
  glUniform1i(m_Shader->LocationOfUniform("texture"), 0);

  glActiveTexture(GL_TEXTURE0 + 0);
  m_texture->Bind();
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(-.2, -0.235715, -0.5);
  glVertex3f(-.2, 0.235715, -0.5);
  glVertex3f(.2, -0.235715, -0.5);
  glVertex3f(.2, 0.235715, -0.5);
  glEnd();
  m_texture->Unbind();

  m_Shader->Unbind();
}

EventHandlerAction HelpLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  switch (ev.keysym.sym) {
  default:
    return EventHandlerAction::PASS_ON;
  }
}
