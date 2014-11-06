#include "stdafx.h"
#include "SpaceLayer.h"

#include "GLController.h"
#include "GLTexture2.h"
#include "GLTexture2Loader.h"
#include "GLShaderLoader.h"

SpaceLayer::SpaceLayer(const EigenTypes::Vector3f& initialEyePos) :
  InteractionLayer(initialEyePos, "shaders/solid"),
  m_PopupShader(Resource<GLShader>("shaders/transparent")),
  m_PopupTexture(Resource<GLTexture2>("images/level3_popup.png")),
  m_OddEven(0),
  m_StarShowMode(0),
  m_StarsToShow(NUM_STARS) {
  m_Buffer.Create(GL_ARRAY_BUFFER);
  m_Buffer.Bind();
  m_Buffer.Allocate(NULL, 12*sizeof(float)*NUM_STARS, GL_DYNAMIC_DRAW);
  m_Buffer.Unbind();

  // Define popup text coordinates
  static const float edges[] = {
    -0.7f, -0.07f, -4.0f, 0, 0,
    -0.7f, +0.07f, -4.0f, 0, 1,
    +0.7f, -0.07f, -4.0f, 1, 0,
    +0.7f, +0.07f, -4.0f, 1, 1,
  };

  m_PopupBuffer.Create(GL_ARRAY_BUFFER);
  m_PopupBuffer.Bind();
  m_PopupBuffer.Allocate(edges, sizeof(edges), GL_STATIC_DRAW);
  m_PopupBuffer.Unbind();

  m_Buf = new float[NUM_STARS*12];
  InitPhysics();
}

SpaceLayer::~SpaceLayer() {
  delete[] m_Buf;
  m_Buffer.Destroy();
}


void SpaceLayer::Update(TimeDelta real_time_delta) {
  m_OddEven ^= 1;
  UpdateAllPhysics();
  for (int i = 6*m_OddEven, j = 0; j < NUM_STARS; j++) {
    const EigenTypes::Vector3f& r = pos[j];
    m_Buf[i++] = r.x();
    m_Buf[i++] = r.y();
    m_Buf[i++] = r.z();
    const EigenTypes::Vector3f& v = vel[j];
    m_Buf[i++] = v.x();
    m_Buf[i++] = v.y();
    m_Buf[i++] = v.z();
    i += 6;
  }
}

void SpaceLayer::Render(TimeDelta real_time_delta) const {
  RenderPopup();
  glDepthMask(GL_FALSE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glLineWidth(1.5);
  glPointSize(1.0);
  int start = SDL_GetTicks();

  // 4 ms per million particles
  m_Shader->Bind();
  GLShaderMatrices::UploadUniforms(*m_Shader, m_ModelView.cast<double>(), m_Projection.cast<double>(), BindFlags::NONE);

  m_Buffer.Bind();
  glEnableVertexAttribArray(m_Shader->LocationOfAttribute("position"));
  glEnableVertexAttribArray(m_Shader->LocationOfAttribute("velocity"));
  glVertexAttribPointer(m_Shader->LocationOfAttribute("position"), 3, GL_FLOAT, GL_TRUE, 6*sizeof(float), 0);
  glVertexAttribPointer(m_Shader->LocationOfAttribute("velocity"), 3, GL_FLOAT, GL_TRUE, 6*sizeof(float), (GLvoid*)(3*sizeof(float)));

  m_Buffer.Write(m_Buf, 12*m_StarsToShow*sizeof(float));
  glDrawArrays(GL_LINES, 0, 2*m_StarsToShow);

  glVertexAttribPointer(m_Shader->LocationOfAttribute("position"), 3, GL_FLOAT, GL_TRUE, 12*sizeof(float), (GLvoid*)((6*m_OddEven)*sizeof(float)));
  glVertexAttribPointer(m_Shader->LocationOfAttribute("velocity"), 3, GL_FLOAT, GL_TRUE, 12*sizeof(float), (GLvoid*)((6*m_OddEven + 3)*sizeof(float)));
  glDrawArrays(GL_POINTS, 0, m_StarsToShow);

  glDisableVertexAttribArray(m_Shader->LocationOfAttribute("position"));
  glDisableVertexAttribArray(m_Shader->LocationOfAttribute("velocity"));
  m_Buffer.Unbind();

  m_Shader->Unbind();
  //std::cout << __LINE__ << ":\t SDL_GetTicks() = " << (SDL_GetTicks() - start) << std::endl;
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_TRUE);
}

void SpaceLayer::RenderPopup() const {
  m_PopupShader->Bind();
  EigenTypes::Matrix4x4f modelView = m_ModelView;
  modelView.block<3, 1>(0, 3) += modelView.block<3, 3>(0, 0)*m_EyePos;
  //modelView.block<3, 3>(0, 0) = EigenTypes::Matrix3x3f::Identity();
  GLShaderMatrices::UploadUniforms(*m_PopupShader, modelView.cast<double>(), m_Projection.cast<double>(), BindFlags::NONE);

  glActiveTexture(GL_TEXTURE0 + 0);
  glUniform1i(m_PopupShader->LocationOfUniform("texture"), 0);
  glUniform1f(m_PopupShader->LocationOfUniform("alpha"), 0.7f);

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
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

EigenTypes::Vector3f SpaceLayer::GenerateVector(const EigenTypes::Vector3f& center, float radius) {
  const float rand_max(RAND_MAX);
  float dx, dy, dz;
  EigenTypes::Vector3f dr;
  do {
    dx = (2.0f*static_cast<float>(rand())/rand_max - 1.0f)*radius;
    dy = (2.0f*static_cast<float>(rand())/rand_max - 1.0f)*radius;
    dz = (2.0f*static_cast<float>(rand())/rand_max - 1.0f)*radius;
    dr = EigenTypes::Vector3f(dx, dy, dz);
  } while (dr.squaredNorm() > radius*radius);
  return center + dr * (dr.squaredNorm()/(radius*radius)) * (dr.squaredNorm()/(radius*radius));
}

EigenTypes::Vector3f SpaceLayer::InitialVelocity(float mass, const EigenTypes::Vector3f& normal, const EigenTypes::Vector3f& dr) {
  return std::sqrt(mass/dr.norm())*normal.cross(dr).normalized();
}

void SpaceLayer::InitPhysics() {
  pos.resize(NUM_STARS);
  vel.resize(NUM_STARS);

  for (int i = 0; i < NUM_GALAXIES; i++) {
    m_GalaxyPos[i] = GenerateVector(1.2f*m_EyePos.cast<float>(), 1.0f);
    if (i == 0) {
      m_GalaxyPos[i] = m_EyePos.cast<float>() + m_EyeView.transpose().cast<float>()*EigenTypes::Vector3f(0, -0.2f, -1.2f);
    }
    m_GalaxyVel[i] = GenerateVector(-1e-3f*(m_GalaxyPos[i] - 1.1f*m_EyePos.cast<float>()), 0.0004f);
    m_GalaxyMass[i] = static_cast<float>(STARS_PER)*5e-11f;
    m_GalaxyNormal[i] = GenerateVector(EigenTypes::Vector3f::Zero(), 1.0).normalized();

    for (int j = 0; j < STARS_PER; j++) {
      const size_t index = i*STARS_PER + j;
      EigenTypes::Vector3f dr = GenerateVector(EigenTypes::Vector3f::Zero(), 0.5);

      // Project to disc
      dr -= 0.4f*atan(dr.dot(m_GalaxyNormal[i])/0.5f)*m_GalaxyNormal[i];

      pos[index] = m_GalaxyPos[i] + dr;
      vel[index] = m_GalaxyVel[i] + InitialVelocity(m_GalaxyMass[i], m_GalaxyNormal[i], dr);
    }
  }
}

void SpaceLayer::UpdateV(int type, const EigenTypes::Vector3f& p, EigenTypes::Vector3f& v, int galaxy) {
  if (galaxy < NUM_GALAXIES) {
    const EigenTypes::Vector3f dr = m_GalaxyPos[galaxy] - p;
    v += m_GalaxyMass[galaxy]*dr.normalized()/(0.3e-3f + dr.squaredNorm());
  } else {
    const EigenTypes::Vector3f dr = m_SkeletonHands[galaxy - NUM_GALAXIES].avgExtended.cast<float>() - (p + (0.1f + static_cast<float>(type)*0.00003f)*v);
    const EigenTypes::Vector3f dv = 1e-3f*dr/(1e-2f + dr.squaredNorm());
    v += dv;
  }
}

void SpaceLayer::UpdateAllPhysics() {
  // Update stars
  for (int i = 0; i < m_StarsToShow; i++) {
    int type = static_cast<int>(static_cast<float>(i)*NUM_STARS/m_StarsToShow);
    EigenTypes::Vector3f tempV = vel[i];
    for (size_t j = 0; j < NUM_GALAXIES + m_SkeletonHands.size(); j++) {
      UpdateV(type, pos[i], tempV, j);
    }
    const EigenTypes::Vector3f tempP = pos[i] + 0.667f*tempV;
    for (size_t j = 0; j < NUM_GALAXIES + m_SkeletonHands.size(); j++) {
      UpdateV(type, tempP, vel[i], j);
    }
    pos[i] += 0.25*tempV + 0.75*vel[i];

    if ((pos[i] - m_EyePos.cast<float>()).squaredNorm() > 50) {
      pos[i] = m_EyePos.cast<float>() - 10*vel[i] + m_EyeView.transpose().cast<float>()*EigenTypes::Vector3f(0, 0.5, 0);
      vel[i].setZero();
    }
  }

  // Update galaxies
  for (size_t i = 0; i < NUM_GALAXIES; i++) {
    EigenTypes::Vector3f tempV = m_GalaxyVel[i];
    for (size_t j = 0; j < NUM_GALAXIES + m_SkeletonHands.size(); j++) {
      if (i != j) { // Galaxy does not affect itself
        UpdateV(0, m_GalaxyPos[i], m_GalaxyVel[i], j);
      }
    }
    const EigenTypes::Vector3f tempP = m_GalaxyPos[i] + 0.667f*tempV;
    for (size_t j = 0; j < NUM_GALAXIES + m_SkeletonHands.size(); j++) {
      if (i != j) { // Galaxy does not affect itself
        UpdateV(0, tempP, m_GalaxyVel[i], j);
      }
    }
    m_GalaxyPos[i] += 0.25*tempV + 0.75*m_GalaxyVel[i];
  }
}

EventHandlerAction SpaceLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  if (ev.type == SDL_KEYDOWN) {
    switch (ev.keysym.sym) {
    case 's':
      m_StarShowMode++;
      m_StarsToShow = static_cast<int>(0.5f + NUM_STARS*exp(-0.4*static_cast<float>(m_StarShowMode % 10)));
      return EventHandlerAction::CONSUME;
    case SDLK_SPACE:
      InitPhysics();
      return EventHandlerAction::CONSUME;
    default:
      return EventHandlerAction::PASS_ON;
    }
  }
  return EventHandlerAction::PASS_ON;
}
