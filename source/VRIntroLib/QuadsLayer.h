#pragma once

#include "Interactionlayer.h"
#include "MathUtility.h"

#include "GLTexture2.h"
#include "GLTexture2Loader.h"

class GLShader;

struct TextureVertex {
  float x, y, z, u, v;
};

struct Pane {
  Pane(int index, float& offset, const char* filename) :
    m_Index(index),
    m_Texture(Resource<GLTexture2>(filename)) {
    if (index*4 >= static_cast<int>(m_RenderBuffer.size())) {
      m_RenderBuffer.resize((index + 1)*4);
    }
    m_RenderBuffer[index*4].u = 0;
    m_RenderBuffer[index*4].v = 0;
    m_RenderBuffer[index*4 + 1].u = 0;
    m_RenderBuffer[index*4 + 1].v = 1;
    m_RenderBuffer[index*4 + 2].u = 1;
    m_RenderBuffer[index*4 + 2].v = 0;
    m_RenderBuffer[index*4 + 3].u = 1;
    m_RenderBuffer[index*4 + 3].v = 1;

    float w = static_cast<float>(m_Texture->Params().Width());
    float h = static_cast<float>(m_Texture->Params().Height());

    float l = offset;
    float t = 1 - m_Gap;
    float r = offset + t*w/h;
    float b = 0;

    offset = r + m_Gap;

    m_Vertices[0] = EigenTypes::Vector3f(l, b, -1);
    m_Vertices[1] = EigenTypes::Vector3f(l, t, -1);
    m_Vertices[2] = EigenTypes::Vector3f(r, b, -1);
    m_Vertices[3] = EigenTypes::Vector3f(r, t, -1);
  }

  static EigenTypes::Vector3f Warp(const EigenTypes::Vector3f& a);
  static EigenTypes::Vector2f UnwarpToYTheta(const EigenTypes::Vector3f& c);

  void Update() {
    EigenTypes::Vector3f warpedVertices[4];
    for (int i = 0; i < 4; i++) {
      warpedVertices[i] = Warp(m_Vertices[i]);
    }

    // Make panes more rectangular looking. True correction would be 0.25, but that falls prey to optical illusion
    EigenTypes::Vector3f shift = 0.19f*(warpedVertices[1] + warpedVertices[2] - warpedVertices[3] - warpedVertices[0]);

    warpedVertices[0] += shift;
    warpedVertices[1] -= shift;
    warpedVertices[2] -= shift;
    warpedVertices[3] += shift;

    for (int i = 0; i < 4; i++) {
      Eigen::Map<EigenTypes::Vector3f>(&m_RenderBuffer[4*m_Index + i].x) = warpedVertices[i];
    }
  }

  void Render() {
    m_Texture->Bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 4*m_Index, 4);
    m_Texture->Unbind();
  }

  EigenTypes::Vector3f m_Vertices[4];

  static float m_Gap;
  static float m_Stride;
  static float m_Radius;
  static EigenTypes::Vector2f m_Pan;
  static EigenTypes::Matrix3x3f m_HeadTilt;

  bool m_Engaged;
  std::shared_ptr<GLTexture2> m_Texture;

  static std::vector<TextureVertex> m_RenderBuffer;
  int m_Index;
};

class QuadsLayer : public InteractionLayer {
public:
  QuadsLayer(const EigenTypes::Vector3f& initialEyePos);
  //virtual ~QuadsLayer ();

  virtual void Update(TimeDelta real_time_delta) override;
  virtual void Render(TimeDelta real_time_delta) const override;
  EventHandlerAction HandleKeyboardEvent(const SDL_KeyboardEvent &ev) override;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
  GLTexture2* m_image[300];

  mutable GLBuffer m_Buffer;
  std::vector<std::shared_ptr<Pane>> m_Panes;

  EigenTypes::Vector2f m_LastYTheta;
  EigenTypes::Vector2f m_DeltaYTheta;
  float m_StripWidth;
};
