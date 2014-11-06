#include "stdafx.h"
#include "QuadsLayer.h"
#include "GLController.h"

#include <float.h>

float Pane::m_Gap = 0.25f;
float Pane::m_Stride = 16.0f;
float Pane::m_Radius = 0.40f;
EigenTypes::Vector2f Pane::m_Pan = EigenTypes::Vector2f::Zero();
EigenTypes::Matrix3x3f Pane::m_HeadTilt = EigenTypes::Matrix3x3f::Identity();
std::vector<TextureVertex> Pane::m_RenderBuffer;

EigenTypes::Vector3f Pane::Warp(const EigenTypes::Vector3f& a) {
  EigenTypes::Matrix2x2f mat;
  mat << m_Stride, 1, -1, m_Stride;
  EigenTypes::Vector2f bb = 2*static_cast<float>(M_PI)/(1 + m_Stride*m_Stride)*mat*a.block<2, 1>(0, 0);
  bb += m_Pan;
  EigenTypes::Vector3f b(sin(bb.x()), bb.y(), -cos(bb.x()));

  float c_phi = 2*(atan(exp(b.y())) - 0.25f*static_cast<float>(M_PI));
  return m_Radius*m_HeadTilt*EigenTypes::Vector3f(cos(c_phi)*b.x(), sin(c_phi), cos(c_phi)*b.z());
}

EigenTypes::Vector2f Pane::UnwarpToYTheta(const EigenTypes::Vector3f& c) {
  float c_phi = asin(c.y()/m_Radius);
  EigenTypes::Vector2f b = EigenTypes::Vector2f(c.x(), c.z()).normalized();
  float bb_y = log(tan(0.5f*c_phi + 0.25f*static_cast<float>(M_PI)));
  if (!(bb_y <= FLT_MAX && bb_y >= -FLT_MAX)) {
    bb_y = 0;
  }
  return EigenTypes::Vector2f(atan2(b.x(), -b.y()), bb_y);
}

std::vector<std::string> get_all_files_names_within_folder(std::string folder) {
  std::vector<std::string> names;
    
#if _WIN32
  char search_path[200];
  sprintf(search_path, "%s\\*.*", folder.c_str());
  WIN32_FIND_DATA fd;
  HANDLE hFind = ::FindFirstFile(search_path, &fd);

  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      // read all (real) files in current folder
      // , delete '!' read other 2 default folder . and ..
      if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        std::string filename(fd.cFileName);
        std::string ext = filename.substr(filename.length() - 4);
        names.push_back(folder + "\\" + fd.cFileName);
      }
    } while (::FindNextFile(hFind, &fd));
    ::FindClose(hFind);
  }
#endif
  return names;
}

QuadsLayer::QuadsLayer(const EigenTypes::Vector3f& initialEyePos) :
  InteractionLayer(EigenTypes::Vector3f::Zero(), "shaders/transparent"),
  m_LastYTheta(EigenTypes::Vector2f::Zero()),
  m_DeltaYTheta(EigenTypes::Vector2f::Zero()),
  m_StripWidth(0.0f) {

  std::vector<std::string> filenames = get_all_files_names_within_folder("gallery");
  for (unsigned int i = 0; i < filenames.size(); i++) {
    try {
      m_Panes.push_back(std::shared_ptr<Pane>(new Pane(i, m_StripWidth, filenames[i].c_str())));
    } catch (std::runtime_error &e) {
      std::cout << "Caught a runtime_error exception: " << e.what() << '\n';
    }
  }

  Pane::m_Pan = EigenTypes::Vector2f(0.5*m_StripWidth, 0);
  m_Buffer.Create(GL_ARRAY_BUFFER);
  m_Buffer.Bind();
  m_Buffer.Allocate(NULL, 4*sizeof(TextureVertex)*filenames.size(), GL_DYNAMIC_DRAW);
  m_Buffer.Unbind();
}

void QuadsLayer::Update(TimeDelta real_time_delta) {
  static const float FADE = 0.97f;

  // Find the farther hand and use it
  EigenTypes::Vector3f wand = EigenTypes::Vector3f::Zero();
  float confidence;

  for (size_t i = 0; i < m_SkeletonHands.size(); i++) {
    const EigenTypes::Vector3f &thisHand = Pane::m_HeadTilt*(m_SkeletonHands[i].avgExtended.cast<float>() - m_EyePos);
    if (thisHand.squaredNorm() > wand.squaredNorm()) {
      wand = thisHand;
      confidence = m_SkeletonHands[i].confidence;
    }
  }
  EigenTypes::Vector2f clutch = wand.isZero() ? EigenTypes::Vector2f::Zero() : Pane::UnwarpToYTheta(wand);

  float clutchStrength = exp(std::min(0.0f, 25.f*(wand.norm() + 0.005f - Pane::m_Radius)));
  EigenTypes::Vector2f clutchMovement = clutch - m_LastYTheta;

  if (clutchMovement.x() > static_cast<float>(M_PI)) {
    clutchMovement.x() -= 2*static_cast<float>(M_PI);
  } else if (clutchMovement.x() < -static_cast<float>(M_PI)) {
    clutchMovement.x() += 2*static_cast<float>(M_PI);
  }

  m_DeltaYTheta = clutchStrength*clutchMovement + (1 - clutchStrength)*(FADE*m_DeltaYTheta + 0.025f*(1-FADE)*EigenTypes::Vector2f(-Pane::m_Stride, 1)/(1 + Pane::m_Stride*Pane::m_Stride));

//  m_DeltaYTheta = FADE*m_DeltaYTheta + (1-FADE)*(clutchStrength*clutchMovement + (1 - clutchStrength)*(0.025f*EigenTypes::Vector2f(-Pane::m_Stride, 1)/(1 + Pane::m_Stride*Pane::m_Stride)));
  Pane::m_Pan += m_DeltaYTheta;
  m_LastYTheta = clutch;
  float panYLimit = 2*static_cast<float>(M_PI)*m_StripWidth/(1 + Pane::m_Stride*Pane::m_Stride);
  Pane::m_Pan.y() = std::min(1.1f*panYLimit, std::max(-0.1f*panYLimit, Pane::m_Pan.y()));

  for (auto it = m_Panes.begin(); it != m_Panes.end(); ++it) {
    Pane &pane = **it;
    pane.Update();
  }
}

void QuadsLayer::Render(TimeDelta real_time_delta) const {
  m_Shader->Bind();
  EigenTypes::Matrix4x4f modelView = m_ModelView;
  modelView.block<3, 1>(0, 3) += modelView.block<3, 3>(0, 0)*m_EyePos;

  Pane::m_HeadTilt = MathUtility::RotationVectorToMatrix(atan(1/Pane::m_Stride)*EigenTypes::Vector3f(modelView(2, 0), 0, modelView(2, 2)));

  //modelView.block<3, 3>(0, 0) = EigenTypes::Matrix3x3f::Identity();
  GLShaderMatrices::UploadUniforms(*m_Shader, modelView.cast<double>(), m_Projection.cast<double>(), BindFlags::NONE);

  glActiveTexture(GL_TEXTURE0 + 0);
  glUniform1i(m_Shader->LocationOfUniform("texture"), 0);
  glUniform1f(m_Shader->LocationOfUniform("alpha"), 0.8f);

  m_Buffer.Bind();
  m_Buffer.Write(Pane::m_RenderBuffer.data(), 4*sizeof(TextureVertex)*m_Panes.size());

  glEnableVertexAttribArray(m_Shader->LocationOfAttribute("position"));
  glEnableVertexAttribArray(m_Shader->LocationOfAttribute("texcoord"));
  glVertexAttribPointer(m_Shader->LocationOfAttribute("position"), 3, GL_FLOAT, GL_TRUE, 5*sizeof(float), (GLvoid*)0);
  glVertexAttribPointer(m_Shader->LocationOfAttribute("texcoord"), 2, GL_FLOAT, GL_TRUE, 5*sizeof(float), (GLvoid*)(3*sizeof(float)));

  for (auto it = m_Panes.begin(); it != m_Panes.end(); ++it) {
    Pane &pane = **it;
    pane.Render();
  }

  glDisableVertexAttribArray(m_Shader->LocationOfAttribute("position"));
  glDisableVertexAttribArray(m_Shader->LocationOfAttribute("texcoord"));
  m_Buffer.Unbind();
  m_Shader->Unbind();
}

EventHandlerAction QuadsLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  //switch (ev.keysym.sym) {
  //default:
  return EventHandlerAction::PASS_ON;
  //}
}
