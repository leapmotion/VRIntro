#include "stdafx.h"
#include "FractalLayer.h"

#include "GLController.h"
#include "Resource.h"
#include "GLTexture2.h"
#include "GLTexture2Loader.h"

FractalLayer::FractalLayer(const EigenTypes::Vector3f& initialEyePos) :
  InteractionLayer(EigenTypes::Vector3f::Zero(), "shaders/fractal"),
  m_Texture(Resource<GLTexture2>("images/random.png")),
  m_AvgPalm(EigenTypes::Vector3f::Zero()),
  m_Time(0) {

  static const float edges[] = {
    // Shader rect
    -2, -2, -1.5, 0, 0,
    -2, +2, -1.5, 0, 1,
    +2, -2, -1.5, 1, 0,
    +2, +2, -1.5, 1, 1,
  };

  m_Buffer.Create(GL_ARRAY_BUFFER);
  m_Buffer.Bind();
  m_Buffer.Allocate(edges, sizeof(edges), GL_STATIC_DRAW);
  m_Buffer.Unbind();

  m_Texture->Bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  m_Texture->Unbind();
}

void FractalLayer::Update(TimeDelta real_time_delta) {
  m_Time += real_time_delta;
  static const float FILTER = 0.2f;

  if (m_Palms.size() > 0) {

    EigenTypes::Vector3f positionSum = EigenTypes::Vector3f::Zero();
    for (size_t i = 0; i < m_Palms.size(); i++) {
      positionSum += m_EyeView*(m_Palms[i] - m_EyePos);
    }
    m_AvgPalm = (1 - FILTER)*m_AvgPalm + FILTER*(EigenTypes::Vector3f(-0.8f, .156f, 0)+0.2f*positionSum/static_cast<float>(m_Palms.size()));
  }
}

void FractalLayer::Render(TimeDelta real_time_delta) const {
  glDepthMask(GL_FALSE);

  m_Shader->Bind();
  EigenTypes::Matrix4x4f modelView = m_ModelView;
  modelView.block<3, 1>(0, 3) += modelView.block<3, 3>(0, 0)*m_EyePos;
  modelView.block<3, 3>(0, 0) = EigenTypes::Matrix3x3f::Identity();
  GLShaderMatrices::UploadUniforms(*m_Shader, modelView.cast<double>(), m_Projection.cast<double>(), BindFlags::NONE);

  glActiveTexture(GL_TEXTURE0 + 0);
  glUniform1i(m_Shader->LocationOfUniform("texture"), 0);
  glUniform1f(m_Shader->LocationOfUniform("time"), static_cast<float>(m_Time));
  glUniform2fv(m_Shader->LocationOfUniform("c_in"), 1, m_AvgPalm.data());

  m_Buffer.Bind();
  glEnableVertexAttribArray(m_Shader->LocationOfAttribute("position"));
  glEnableVertexAttribArray(m_Shader->LocationOfAttribute("texcoord"));
  glVertexAttribPointer(m_Shader->LocationOfAttribute("position"), 3, GL_FLOAT, GL_TRUE, 5*sizeof(float), (GLvoid*)0);
  glVertexAttribPointer(m_Shader->LocationOfAttribute("texcoord"), 2, GL_FLOAT, GL_TRUE, 5*sizeof(float), (GLvoid*)(3*sizeof(float)));

  m_Texture->Bind();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisableVertexAttribArray(m_Shader->LocationOfAttribute("position"));
  glDisableVertexAttribArray(m_Shader->LocationOfAttribute("texcoord"));
  m_Buffer.Unbind();

  m_Shader->Unbind();
  glDepthMask(GL_TRUE);
}

EventHandlerAction FractalLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  //switch (ev.keysym.sym) {
  //default:
  return EventHandlerAction::PASS_ON;
  //}
}
