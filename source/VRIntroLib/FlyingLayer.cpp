#include "stdafx.h"
#include "FlyingLayer.h"
#include "PrecisionTimer.h"
#include "MathUtility.h"

#include "Primitives.h"
#include "GLController.h"

#include "GLTexture2.h"
#include "GLTexture2Loader.h"
#include "GLShaderLoader.h"

FlyingLayer::FlyingLayer(const EigenTypes::Vector3f& initialEyePos) :
  InteractionLayer(initialEyePos),
  m_PopupShader(Resource<GLShader>("shaders/transparent")),
  m_PopupTexture(Resource<GLTexture2>("images/level4_popup.png")),
  m_GridCenter(initialEyePos),
  m_Velocity(EigenTypes::Vector3f::Zero()),
  m_RotationAA(EigenTypes::Vector3f::Zero()),
  m_GridOrientation(EigenTypes::Matrix4x4f::Identity()),
  m_LineThickness(3.0f),
  m_GridBrightness(80) {

  // Define popup text coordinates
  static const float edges[] = {
    -0.4f, -0.174f, -0.312f, 0, 0,
    -0.4f, +0.210f, -0.6f, 0, 1,
    +0.4f, -0.174f, -0.312f, 1, 0,
    +0.4f, +0.210f, -0.6f, 1, 1,
  };

  m_PopupBuffer.Create(GL_ARRAY_BUFFER);
  m_PopupBuffer.Bind();
  m_PopupBuffer.Allocate(edges, sizeof(edges), GL_STATIC_DRAW);
  m_PopupBuffer.Unbind();
}

void FlyingLayer::Update(TimeDelta real_time_delta) {
  static const float PERIOD_TRANS = 0.0045f;
  static const float PERIOD_ROT = 1.0f;
  static const float FILTER = 0.2f;

  if (m_Palms.size() > 0) {

    EigenTypes::Vector3f positionSum = EigenTypes::Vector3f::Zero();
    EigenTypes::Vector3f rotationAASum = EigenTypes::Vector3f::Zero();
    for (size_t i = 0; i < m_Palms.size(); i++) {
      positionSum += m_GridOrientation.block<3, 3>(0, 0).transpose()*(m_Palms[i] - m_EyePos - m_EyeView.transpose()*EigenTypes::Vector3f(0, -0.15f, -0.05f));
      //rotationAASum += RotationMatrixToVector(m_EyeView.transpose()*m_PalmOrientations[i]*m_EyeView.transpose());
      EigenTypes::Matrix3x3f rot;
      MathUtility::RotationMatrix_VectorToVector(-EigenTypes::Vector3f::UnitZ(), m_EyeView*(m_Palms[i] - m_EyePos) - EigenTypes::Vector3f(0, -0.15f, -0.05f), rot);
      //std::cout << __LINE__ << ":\t       rot = " << (rot) << std::endl;
      rotationAASum += MathUtility::RotationMatrixToVector(rot);
    }
    if (m_Palms.size() == 2) {
      const EigenTypes::Vector3f dir0 = m_EyeView*(m_Palms[0] - m_EyePos) - EigenTypes::Vector3f(0, -0.15f, -0.05f);
      const EigenTypes::Vector3f dir1 = m_EyeView*(m_Palms[1] - m_EyePos) - EigenTypes::Vector3f(0, -0.15f, -0.05f);

      EigenTypes::Matrix3x3f rot;
      MathUtility::RotationMatrix_VectorToVector((dir0.x() < dir1.x() ? 1.0f : -1.0f) * EigenTypes::Vector3f::UnitX(), dir1 - dir0, rot);
      //std::cout << __LINE__ << ":\t     positionSum = " << (positionSum) << std::endl;

      rotationAASum += 2.0f*MathUtility::RotationMatrixToVector(rot);
    }
    m_Velocity = (1 - FILTER)*m_Velocity + FILTER*positionSum/static_cast<float>(m_Palms.size());
    m_RotationAA = (1 - FILTER)*m_RotationAA + FILTER*rotationAASum/static_cast<float>(m_Palms.size());
  } else {
    m_Velocity = (1 - 0.3f*FILTER)*m_Velocity;
    m_RotationAA = (1 - 0.3f*FILTER)*m_RotationAA;
  }

  static const float MAX_VELOCITY_SQNORM = 0.2f;
  static const float MAX_ROTATION_SQNORM = 1.0f;
  float dt = static_cast<float>(real_time_delta);
  m_GridCenter -= m_Velocity*std::min(MAX_VELOCITY_SQNORM, m_Velocity.squaredNorm())*(dt/PERIOD_TRANS);
  const EigenTypes::Matrix3x3f rot = MathUtility::RotationVectorToMatrix((dt/PERIOD_ROT)*m_RotationAA*std::min(MAX_ROTATION_SQNORM, m_RotationAA.squaredNorm()));
  //std::cout << __LINE__ << ":\t   rot = " << (rot) << std::endl;
  //EigenTypes::Matrix3x3f foo = ;
  m_GridOrientation.block<3, 3>(0, 0) = m_EyeView.transpose()*rot.transpose()*m_EyeView*m_GridOrientation.block<3, 3>(0, 0);
}

void FlyingLayer::Render(TimeDelta real_time_delta) const {
  glDepthMask(GL_FALSE);
  // Draw joystick
  //PrecisionTimer timer;
  //timer.Start();
  m_Shader->Bind();
  const EigenTypes::Vector3f desiredLightPos(0, 1.5, 0.5);
  const EigenTypes::Vector3f lightPos = m_EyeView*desiredLightPos;
  const int lightPosLoc = m_Shader->LocationOfUniform("lightPosition");
  glUniform3f(lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);

  m_Sphere.SetRadius(0.3f);
  m_Sphere.Translation() = (m_EyeView.transpose()*EigenTypes::Vector3f(0, 0, -1.25) + m_EyePos).cast<double>();
  m_Sphere.Material().SetDiffuseLightColor(Color(1.0f, 0.5f, 0.4f, m_Alpha));
  m_Sphere.Material().SetAmbientLightingProportion(0.3f);
  //PrimitiveBase::DrawSceneGraph(m_Sphere, m_Renderer);
  m_Shader->Unbind();

  glBegin(GL_LINES);
  glVertex3fv(EigenTypes::Vector3f::Zero().eval().data());
  glVertex3fv(m_Velocity.eval().data());
  glEnd();

  //glMatrixMode(GL_MODELVIEW);
  EigenTypes::Vector3f centerpoint = m_EyePos - m_GridCenter;
  //glScalef(2.0f, 2.0f, 2.0f);
  glMultMatrixf(m_GridOrientation.eval().data());
  glTranslatef(m_GridCenter.x(), m_GridCenter.y(), m_GridCenter.z());

  int xShift = 2*static_cast<int>(0.5f*centerpoint.x() + 0.5f);
  int yShift = 20*static_cast<int>(0.05f*centerpoint.y() + 0.5f);
  int zShift = 2*static_cast<int>(0.5f*centerpoint.z() + 0.5f);
  glLineWidth(m_LineThickness);
  glBegin(GL_LINES);
  for (int i = -50 + xShift; i < 50 + xShift; i+=2) {
    for (int j = -30 + yShift; j < 50 + yShift; j+=20) {
      for (int k = -50 + zShift; k < 50 + zShift; k+=2) {
        EigenTypes::Vector3f a(static_cast<float>(i), static_cast<float>(j), static_cast<float>(k));
        EigenTypes::Vector3f b(static_cast<float>(i + 2), static_cast<float>(j), static_cast<float>(k));
        //EigenTypes::Vector3f c(static_cast<float>(i), static_cast<float>(j + 2), static_cast<float>(k));
        EigenTypes::Vector3f d(static_cast<float>(i), static_cast<float>(j), static_cast<float>(k + 2));
        float aColor = std::min(1.0f, static_cast<float>(m_GridBrightness)/(20.0f + (a - centerpoint).squaredNorm()));
        float bColor = std::min(1.0f, static_cast<float>(m_GridBrightness)/(20.0f + (b - centerpoint).squaredNorm()));
        //float cColor = std::min(1.1f9 100.02/(10.0f + (c - centerpoint).squaredNorm()));
        float dColor = std::min(1.0f, static_cast<float>(m_GridBrightness)/(20.0f + (d - centerpoint).squaredNorm()));

        glColor4f(1.0f, 1.0f, 1.0f, m_Alpha*aColor);
        glVertex3fv((a).eval().data());

        glColor4f(1.0f, 1.0f, 1.0f, m_Alpha*bColor);
        glVertex3fv((b).eval().data());

        glColor4f(1.0f, 1.0f, 1.0f, m_Alpha*aColor);
        glVertex3fv((a).eval().data());

        glColor4f(1.0f, 1.0f, 1.0f, m_Alpha*dColor);
        glVertex3fv((d).eval().data());
      }
    }
  }
  glEnd();
  //double elapsed = timer.Stop();
  //std::cout << __LINE__ << ":\t   elapsed = " << (elapsed) << std::endl;
  if (m_Palms.size() == 0) {
    RenderPopup();
  }
  glDepthMask(GL_TRUE);
}

void FlyingLayer::RenderPopup() const {
  m_PopupShader->Bind();
  EigenTypes::Matrix4x4f modelView = m_ModelView;
  modelView.block<3, 1>(0, 3) += modelView.block<3, 3>(0, 0)*m_EyePos;
  modelView.block<3, 3>(0, 0) = EigenTypes::Matrix3x3f::Identity();
  GLShaderMatrices::UploadUniforms(*m_PopupShader, modelView.cast<double>(), m_Projection.cast<double>(), BindFlags::NONE);

  glActiveTexture(GL_TEXTURE0 + 0);
  glUniform1i(m_PopupShader->LocationOfUniform("texture"), 0);
  glUniform1f(m_PopupShader->LocationOfUniform("alpha"), 1.0f);

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

EventHandlerAction FlyingLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  switch (ev.keysym.sym) {
  case 'g':
    if (SDL_GetModState() & KMOD_SHIFT) {
      m_GridBrightness = std::min(300, m_GridBrightness + 10);
    } else {
      m_GridBrightness = std::max(10, m_GridBrightness - 10);
    }
    return EventHandlerAction::CONSUME;
  case 'l':
    if (SDL_GetModState() & KMOD_SHIFT) {
      m_LineThickness = std::min(10.0f, m_LineThickness + 0.5f);
    } else {
      m_LineThickness = std::max(0.5f, m_LineThickness - 0.5f);
    }
    return EventHandlerAction::CONSUME;
  default:
    return EventHandlerAction::PASS_ON;
  }
}
