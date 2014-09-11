#include "SpheresLayer.h"

#include "GLController.h"

SpheresLayer::SpheresLayer(const Vector3f& initialEyePos) : 
  InteractionLayer(initialEyePos),
  m_Pos(NUM_SPHERES),
  m_Disp(NUM_SPHERES, Vector3f::Zero()),
  m_Vel(NUM_SPHERES, Vector3f::Zero()),
  m_Colors(NUM_SPHERES),
  m_Mono(NUM_SPHERES),
  m_Radius(NUM_SPHERES),
  m_Spring(1.0f),
  m_Damp(1.0f),
  m_Well(-1.0f) {
  for (int i = 0; i < NUM_SPHERES; i++) {
    float z = (float)rand() / RAND_MAX * 2.0f - 1.0f;
    float theta = (float)rand() / RAND_MAX * 6.28318530718f;
    float xy = std::sqrt(1.0f - z*z);
    float x = xy*cos(theta);
    float y = xy*sin(theta);

    float r = (float)rand() / RAND_MAX;
    float g = (float)rand() / RAND_MAX;
    float b = (float)rand() / RAND_MAX;
    float dist = 0.55f + (float)rand() / RAND_MAX * 0.2f;
    m_Radius[i] = 0.025f + (float)rand() / RAND_MAX * 0.030f;
    m_Pos[i] = Vector3f(0.0f, 1.7f, -5.0f) + Vector3f(x, y, z)*dist;
    m_Colors[i] = Vector3f(r, g, b).normalized();
    m_Mono[i] = Vector3f::Ones()*m_Colors[i].sum()*0.33f;
  }
}

void SpheresLayer::Update(TimeDelta real_time_delta) {
  ComputePhysics(real_time_delta);
}

void SpheresLayer::Render(TimeDelta real_time_delta) const {
  glEnable(GL_BLEND);
  m_Shader->Bind();
  const Vector3f desiredLightPos(0, 1.5, 0.5);
  const Vector3f lightPos = m_EyeView*desiredLightPos;
  const int lightPosLoc = m_Shader->LocationOfUniform("light_position");
  glUniform3f(lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);

  for (size_t j = 0; j < NUM_SPHERES; j++) {
    float desaturation = 0.005f / (0.005f + m_Disp[j].squaredNorm());
    Vector3f color = m_Colors[j]*(1.0 - desaturation) + m_Mono[j]*desaturation;

    m_Sphere.SetRadius(m_Radius[j]);
    m_Sphere.Translation() = (m_Pos[j] + m_Disp[j]).cast<double>();
    m_Sphere.Material().SetDiffuseLightColor(Color(color.x(), color.y(), color.z(), m_Alpha));
    m_Sphere.Material().SetAmbientLightColor(Color(color.x(), color.y(), color.z(), m_Alpha));
    m_Sphere.Material().SetAmbientLightingProportion(0.2f);
    PrimitiveBase::DrawSceneGraph(m_Sphere, m_Renderer);
  }
  m_Shader->Unbind();
  RenderGrid();
}

void SpheresLayer::RenderGrid() const {
  const int divTheta = 22;
  const int divPhi = 40;
  const float radius = 0.7;

  glTranslatef(m_EyePos.x(), m_EyePos.y(), m_EyePos.z());
  glColor4f(0.2f, 0.6f, 1.0f, m_Alpha*0.5f);
  glLineWidth(1.0f);
  glBegin(GL_LINES);
  for (int i = 0; i < divPhi; i++) {
    float phi0 = M_PI*(i/static_cast<float>(divPhi) - 0.5f);
    float phi1 = M_PI*((i + 1)/static_cast<float>(divPhi) - 0.5f);
    for (int j = 0; j < divTheta; j++) {
      float theta0 = 2*M_PI*(j/static_cast<float>(divTheta));
      float theta1 = 2*M_PI*((j + 1)/static_cast<float>(divTheta));
      glVertex3f(radius*cos(phi0)*cos(theta0), radius*sin(phi0), radius*cos(phi0)*sin(theta0));
      glVertex3f(radius*cos(phi0)*cos(theta1), radius*sin(phi0), radius*cos(phi0)*sin(theta1));
      glVertex3f(radius*cos(phi0)*cos(theta0), radius*sin(phi0), radius*cos(phi0)*sin(theta0));
      glVertex3f(radius*cos(phi1)*cos(theta0), radius*sin(phi1), radius*cos(phi1)*sin(theta0));
    }
  }
  glEnd();
}

EventHandlerAction SpheresLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  switch (ev.keysym.sym) {
  default:
    return EventHandlerAction::PASS_ON;
  }
}

void SpheresLayer::ComputePhysics(TimeDelta real_time_delta) {
  const int num_tips = static_cast<int>(m_Tips.size());
  // std::cout << __LINE__ << ":\t   deltaT = " << (deltaT) << std::endl;
  for (int i = 0; i < NUM_SPHERES; i++) {
    static const float K = 10;
    static const float D = 3;
    static const float A = 0.003;
    static const float AA = 0.0001;

    Vector3f accel = -K*m_Spring*m_Disp[i] -D*m_Damp*m_Vel[i];
    // std::cout << __LINE__ << ":\t     num_tips = " << (num_tips) << std::endl;
    for (int j = 0; j < num_tips; j++) {

      // std::cout << __LINE__ << ":\t       (positions[i] - tips[j]).squaredNorm() = " << ((positions[i] - tips[j]).squaredNorm()) << std::endl;
      const Vector3f diff = m_Tips[j] - (m_Pos[i] + m_Disp[i]);
      float distSq = diff.squaredNorm();
      accel += A*m_Well*diff/(AA + distSq*distSq);
      // accel += A*m_Well*diff/(AA + distSq*diff.norm());
    }

    m_Disp[i] += 0.5*m_Vel[i]*real_time_delta;
    m_Vel[i] += accel*real_time_delta;
    m_Disp[i] += 0.5*m_Vel[i]*real_time_delta;
  }
}
