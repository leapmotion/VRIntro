#include "HelpLayer.h"

#include "GLController.h"
#include "Resource.h"
#include "GLTexture2.h"
#include "GLTexture2Loader.h"

HelpLayer::HelpLayer(const Vector3f& initialEyePos) :
  InteractionLayer(Vector3f::Zero(), "transparent"),
  m_HelpTexture(Resource<GLTexture2>("help.png")),
  m_LowFPSTexture(Resource<GLTexture2>("help.png")) {

  static const float edges[] = {
    // Help menu
    -0.224f, -0.264f, -0.5f, 0, 0,
    -0.224f, +0.264f, -0.5f, 0, 1,
    +0.224f, -0.264f, -0.5f, 1, 0,
    +0.224f, +0.264f, -0.5f, 1, 1,

    // Low FPS warning
    -0.224f, -0.264f, -0.5f, 0, 0,
    -0.224f, +0.264f, -0.5f, 0, 1,
    +0.224f, -0.264f, -0.5f, 1, 0,
    +0.224f, +0.264f, -0.5f, 1, 1,
  };

  m_Buffer.Create(GL_ARRAY_BUFFER);
  m_Buffer.Bind();
  m_Buffer.Allocate(edges, sizeof(edges), GL_STATIC_DRAW);
  m_Buffer.Release();

  m_Visible[0] = true;
  for (int i = 1; i < NUM_MESSAGES; i++) {
    m_Visible[i] = false;
  }
}

void HelpLayer::Render(TimeDelta real_time_delta) const {
  glDepthMask(GL_FALSE);

  m_Shader->Bind();
  m_Renderer.GetModelView().Matrix() = Matrix4x4::Identity();
  m_Renderer.UploadMatrices();

  glActiveTexture(GL_TEXTURE0 + 0);
  glUniform1i(m_Shader->LocationOfUniform("texture"), 0);

  m_Buffer.Bind();
  glEnableVertexAttribArray(m_Shader->LocationOfAttribute("position"));
  glEnableVertexAttribArray(m_Shader->LocationOfAttribute("texcoord"));
  glVertexAttribPointer(m_Shader->LocationOfAttribute("position"), 3, GL_FLOAT, GL_TRUE, 5*sizeof(float), (GLvoid*)0);
  glVertexAttribPointer(m_Shader->LocationOfAttribute("texcoord"), 2, GL_FLOAT, GL_TRUE, 5*sizeof(float), (GLvoid*)(3*sizeof(float)));

  for (int i = 0; i < NUM_MESSAGES; i++) {
    if (m_Visible[i]) {
      DrawMessage(0);
    }
  }
  //DrawMessage(1);

  glDisableVertexAttribArray(m_Shader->LocationOfAttribute("position"));
  glDisableVertexAttribArray(m_Shader->LocationOfAttribute("texcoord"));
  m_Buffer.Release();

  m_Shader->Unbind();
  glDepthMask(GL_TRUE);
}

void HelpLayer::DrawMessage(int index) const {
  switch (index) {
  case 0:
    m_HelpTexture->Bind();
    break;
  case 1:
    m_LowFPSTexture->Bind();
    break;
  default:
    return;
  }
  glDrawArrays(GL_TRIANGLE_STRIP, 4*index, 4);
  glBindTexture(GL_TEXTURE_2D, 0); // Unbind
}

EventHandlerAction HelpLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  switch (ev.keysym.sym) {
  default:
    return EventHandlerAction::PASS_ON;
  }
}
