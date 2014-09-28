#include "stdafx.h"
#include "PassthroughLayer.h"
#include "GLController.h"
#include "GLShader.h"

#include "GLTexture2.h"
#include "GLTexture2Loader.h"
#include "GLShaderLoader.h"

PassthroughLayer::PassthroughLayer() :
  InteractionLayer(EigenTypes::Vector3f::Zero(), "shaders/passthrough"),
  m_image(GLTexture2Params(640, 240, GL_LUMINANCE), GLTexture2PixelDataReference(GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL, 0)),
  m_colorimage(GLTexture2Params(672, 496, GL_RGBA), GLTexture2PixelDataReference(GL_RGBA, GL_UNSIGNED_BYTE, NULL, 0)),
  m_distortion(GLTexture2Params(64, 64, GL_RG32F), GLTexture2PixelDataReference(GL_RG, GL_FLOAT, NULL, 0)),
  m_PopupShader(Resource<GLShader>("shaders/transparent")),
  m_PopupTexture(Resource<GLTexture2>("images/no_images.png")),
  m_Gamma(0.8f),
  m_Brightness(1.0f),
  m_HasData(false) {
  m_Buffer.Create(GL_ARRAY_BUFFER);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  m_image.Bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  m_image.Unbind();

  m_colorimage.Bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  m_colorimage.Unbind();

  m_distortion.Bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  m_distortion.Unbind();

  // Define popup text coordinates
  static const float edges[] = {
    -0.4f, -0.3f, -0.6f, 0, 0,
    -0.4f, +0.3f, -0.6f, 0, 1,
    +0.4f, -0.3f, -0.6f, 1, 0,
    +0.4f, +0.3f, -0.6f, 1, 1,
  };

  m_PopupBuffer.Create(GL_ARRAY_BUFFER);
  m_PopupBuffer.Bind();
  m_PopupBuffer.Allocate(edges, sizeof(edges), GL_STATIC_DRAW);
  m_PopupBuffer.Unbind();
}

PassthroughLayer::~PassthroughLayer() {
  m_Buffer.Destroy();
}

void PassthroughLayer::SetImage(const unsigned char* data, int width, int height) {
  GLTexture2Params params = m_image.Params();

  // We have to resize our texture when image sizes change
  if (width != params.Width() || height != params.Height()) {
    m_image.Bind();
    params.SetWidth(width);
    params.SetHeight(height);
    glTexImage2D(params.Target(),
                 0,
                 params.InternalFormat(),
                 params.Width(),
                 params.Height(),
                 0,
                 GL_LUMINANCE, // HACK because we don't have api-level access for glTexImage2D after construction, only glTexSubImage2D
                 GL_UNSIGNED_BYTE,
                 data);
    m_image.Unbind();
  } else {
    m_image.UpdateTexture(data);
  }
  m_UseColor = false;
  m_HasData = true;
}

void PassthroughLayer::SetColorImage(const unsigned char* data) {
  m_colorimage.UpdateTexture(data);
  m_UseColor = true;
  m_HasData = true;
}

void PassthroughLayer::SetDistortion(const float* data) {
  m_distortion.UpdateTexture(data);
}

void PassthroughLayer::Render(TimeDelta real_time_delta) const {
  if (m_HasData) {
    m_Shader->Bind();
    GLShaderMatrices::UploadUniforms(*m_Shader, EigenTypes::Matrix4x4::Identity(), m_Projection.cast<double>(), BindFlags::NONE);

    glActiveTexture(GL_TEXTURE0 + 0);
    if (m_UseColor) {
      m_colorimage.Bind();
    } else {
      m_image.Bind();
    }
    glActiveTexture(GL_TEXTURE0 + 1);
    m_distortion.Bind();

    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniform2f(m_Shader->LocationOfUniform("ray_scale"), 0.125f, 0.125f);
    glUniform2f(m_Shader->LocationOfUniform("ray_offset"), 0.5f, 0.5f);
    glUniform1i(m_Shader->LocationOfUniform("texture"), 0);
    glUniform1i(m_Shader->LocationOfUniform("distortion"), 1);
    glUniform1f(m_Shader->LocationOfUniform("gamma"), m_Gamma*(m_UseColor ? 0.72f : 1.0f));
    glUniform1f(m_Shader->LocationOfUniform("brightness"), m_Alpha*m_Brightness);
    glUniform1f(m_Shader->LocationOfUniform("use_color"), m_UseColor ? 1.0f : 0.0f);

#if 0
    const float edges[] = {-4, -4, -1, -4, 4, -1, 4, -4, -1, 4, 4, -1};
    // Why the fuck doesn't this work?
    m_Renderer.EnablePositionAttribute();
    m_Buffer.Bind();
    m_Buffer.Allocate(&edges[0], sizeof(edges), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_Buffer.Release();
    m_Renderer.DisablePositionAttribute();
#else
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(-4, -4, -1);
    glVertex3f(-4, 4, -1);
    glVertex3f(4, -4, -1);
    glVertex3f(4, 4, -1);
    glEnd();
#endif

    if (m_UseColor) {
      m_colorimage.Unbind();
    } else {
      m_image.Unbind();
    }
    m_distortion.Unbind();
    m_Shader->Unbind();
    glClear(GL_DEPTH_BUFFER_BIT);
  } else {
    glDepthMask(GL_FALSE);
    RenderPopup();
    glDepthMask(GL_TRUE);
  }
}

void PassthroughLayer::RenderPopup() const {
  m_PopupShader->Bind();
  GLShaderMatrices::UploadUniforms(*m_PopupShader, EigenTypes::Matrix4x4::Identity(), m_Projection.cast<double>(), BindFlags::NONE);

  glActiveTexture(GL_TEXTURE0 + 0);
  glUniform1i(m_PopupShader->LocationOfUniform("texture"), 0);

  m_PopupBuffer.Bind();
  glEnableVertexAttribArray(m_PopupShader->LocationOfAttribute("position"));
  glEnableVertexAttribArray(m_PopupShader->LocationOfAttribute("texcoord"));
  glVertexAttribPointer(m_PopupShader->LocationOfAttribute("position"), 3, GL_FLOAT, GL_TRUE, 5*sizeof(float), (GLvoid*)0);
  glVertexAttribPointer(m_PopupShader->LocationOfAttribute("texcoord"), 2, GL_FLOAT, GL_TRUE, 5*sizeof(float), (GLvoid*)(3*sizeof(float)));

  m_PopupTexture->Bind();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindTexture(GL_TEXTURE_2D, 0); // Unbind

  glDisableVertexAttribArray(m_PopupShader->LocationOfAttribute("position"));
  glDisableVertexAttribArray(m_PopupShader->LocationOfAttribute("texcoord"));
  m_PopupBuffer.Unbind();

  m_PopupShader->Unbind();
}

EventHandlerAction PassthroughLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  switch (ev.keysym.sym) {
  case '[':
    m_Gamma = std::max(0.f, m_Gamma - 0.04f);
    return EventHandlerAction::CONSUME;
  case ']':
    m_Gamma = std::min(1.2f, m_Gamma + 0.04f);
    return EventHandlerAction::CONSUME;
  case SDLK_INSERT:
    m_Brightness = std::max(0.f, m_Brightness + 0.02f);
    return EventHandlerAction::CONSUME;
  case SDLK_DELETE:
    m_Brightness = std::min(2.f, m_Brightness - 0.02f);
    return EventHandlerAction::CONSUME;
  default:
    return EventHandlerAction::PASS_ON;
  }
}
