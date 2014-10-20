#include "stdafx.h"
#include "PhysicsLayer.h"

#include "GLController.h"
#include "Resource.h"
#include "GLTexture2.h"
#include "GLTexture2Loader.h"

#include <iostream>

#include <btBulletDynamicsCommon.h>

class BulletWrapper
{
public:
  BulletWrapper() { zero(); }
  ~BulletWrapper() { destroy(); }

  struct BodyData
  {
    btRigidBody* m_Body;
    std::shared_ptr<btCollisionShape> m_SharedShape;
  };

  void init();
  void destroy();

  void step(TimeDelta deltaTime);

  void utilSetupScene(const EigenTypes::Vector3f& refPosition);

  void utilAddCube(const EigenTypes::Vector3f& position);
  void utilAddGround();

  void removeAllBodies();

  const std::vector<BodyData>& getBodyDatas() const { return m_BodyDatas; }

private:
  void zero();

  btBroadphaseInterface* m_Broadphase;
  btDefaultCollisionConfiguration* m_CollisionConfiguration;
  btCollisionDispatcher* m_Dispatcher;
  btSequentialImpulseConstraintSolver* m_Solver;
  btDiscreteDynamicsWorld* m_DynamicsWorld;

  std::vector<BodyData> m_BodyDatas;
};

EigenTypes::Vector3f FromBullet(const btVector3& v) { return EigenTypes::Vector3f(v.x(), v.y(), v.z()); }
btVector3 ToBullet(const EigenTypes::Vector3f& v) {return btVector3(v.x(), v.y(), v.z()); }

Eigen::Quaternion<double> FromBullet(const btQuaternion& q) { return Eigen::Quaternion<double>(q.getW(), q.getX(), q.getY(), q.getZ()); }
btQuaternion ToBullet(const Eigen::Quaternion<double>& q) { return btQuaternion(q.x(), q.y(), q.z(), q.w()); }

void BulletWrapper::init()
{
  assert(!m_Broadphase && !m_CollisionConfiguration && !m_Dispatcher && !m_Solver && !m_DynamicsWorld);

  m_Broadphase = new btDbvtBroadphase();

  m_CollisionConfiguration = new btDefaultCollisionConfiguration();
  m_Dispatcher = new btCollisionDispatcher(m_CollisionConfiguration);

  m_Solver = new btSequentialImpulseConstraintSolver;

  m_DynamicsWorld = new btDiscreteDynamicsWorld(m_Dispatcher, m_Broadphase, m_Solver, m_CollisionConfiguration);

  //m_DynamicsWorld->setGravity(btVector3(0, -10, 0));
  m_DynamicsWorld->setGravity(btVector3(0, 0, 0));

}

void BulletWrapper::destroy()
{
  removeAllBodies();

  delete m_DynamicsWorld;
  delete m_Solver;
  delete m_CollisionConfiguration;
  delete m_Dispatcher;
  delete m_Broadphase;

  zero();
}

void BulletWrapper::utilAddGround()
{
  m_BodyDatas.resize(m_BodyDatas.size() + 1);
  BodyData& bodyData = m_BodyDatas.back();

  // Create shape
  bodyData.m_SharedShape.reset(new btStaticPlaneShape(btVector3(0, 1, 0), 1));

  // Create motion
  btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));

  // Create body
  btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, bodyData.m_SharedShape.get(), btVector3(0, 0, 0));
  bodyData.m_Body = new btRigidBody(groundRigidBodyCI);
  
  m_DynamicsWorld->addRigidBody(bodyData.m_Body);
}

void BulletWrapper::step(TimeDelta deltaTime)
{
  m_DynamicsWorld->stepSimulation(deltaTime);
}

void BulletWrapper::utilSetupScene(const EigenTypes::Vector3f& refPosition)
{
   // setup walls 
  for (int i = 0; i < 5; i++)
  {
    utilAddCube(refPosition + EigenTypes::Vector3f(-0.1f + 0.05f * i, 0.0f, -0.5f));
  }
}

void BulletWrapper::utilAddCube(const EigenTypes::Vector3f& position)
{
  m_BodyDatas.resize(m_BodyDatas.size() + 1);
  BodyData& bodyData = m_BodyDatas.back();

  bodyData.m_SharedShape.reset(new btBoxShape(btVector3(0.025f, 0.025f, 0.025f)));

  btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x(), position.y(), position.z())));
  btScalar mass = 1;
  btVector3 fallInertia(0, 0, 0);
  bodyData.m_SharedShape->calculateLocalInertia(mass, fallInertia);
  btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, bodyData.m_SharedShape.get(), fallInertia);
  bodyData.m_Body = new btRigidBody(fallRigidBodyCI);
  m_DynamicsWorld->addRigidBody(bodyData.m_Body);
}

void BulletWrapper::removeAllBodies()
{
  for (int i = 0; i < m_BodyDatas.size(); i++)
  {
    btRigidBody* body = m_BodyDatas[i].m_Body;

    m_DynamicsWorld->removeRigidBody(body);
    delete body->getMotionState();
    delete body;

    // Shape is automatically released via the shared pointer
  }

  m_BodyDatas.clear();
}

void BulletWrapper::zero()
{
  m_Broadphase = NULL;
  m_CollisionConfiguration = NULL;
  m_Dispatcher = NULL;
  m_Solver = NULL;
  m_DynamicsWorld = NULL;
}


PhysicsLayer::PhysicsLayer(const EigenTypes::Vector3f& initialEyePos) :
  InteractionLayer(EigenTypes::Vector3f::Zero()) {

  m_BulletWrapper = NULL;
}

PhysicsLayer::~PhysicsLayer() {
  delete m_BulletWrapper;
}

void PhysicsLayer::Update(TimeDelta real_time_delta) {
  // Late physics setup (to get valid reference position)
  if (!m_BulletWrapper)
  {
    m_BulletWrapper = new BulletWrapper();
    m_BulletWrapper->init();
    m_BulletWrapper->utilSetupScene(m_EyePos);
  }

  // Run physics
  m_BulletWrapper->step(real_time_delta);
}

void PhysicsLayer::Render(TimeDelta real_time_delta) const {
  glEnable(GL_BLEND);
  m_Shader->Bind();

  const EigenTypes::Vector3f desiredLightPos(0, 1.5, 0.5);
  const EigenTypes::Vector3f lightPos = m_EyeView*desiredLightPos;
  const int lightPosLoc = m_Shader->LocationOfUniform("light_position");
  glUniform3f(lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);

  // Common property
  m_Box.Material().SetAmbientLightingProportion(0.3f);

  //{
  //  // Process all hands
  //  for (auto it = m_SkeletonHands.begin(); it != m_SkeletonHands.end(); it++)
  //  {
  //    const SkeletonHand& hand = *it;
  //    // Process all joints
  //    for (auto it = hand.joints; it != hand.joints + 23; it++)
  //    {
  //      const EigenTypes::Vector3f& joint = *it;

  //      m_Box.SetSize(EigenTypes::Vector3f(0.1f, 0.1f, 0.1f).cast<double>());
  //      m_Box.Translation() = joint.cast<double>();
  //      m_Box.LinearTransformation() = Eigen::AngleAxis<double>(0.25f * M_PI, EigenTypes::Vector3::UnitZ()).toRotationMatrix();
  //      
  //      m_Box.Material().SetDiffuseLightColor(Color(1.0f, 1.0f, 1.0f, m_Alpha));
  //      m_Box.Material().SetAmbientLightColor(Color(1.0f, 1.0f, 1.0f, m_Alpha));
  //      PrimitiveBase::DrawSceneGraph(m_Box, m_Renderer);
  //    }
  //  }
  //}

  // Render dynamics
  const std::vector<BulletWrapper::BodyData>& bodyDatas = m_BulletWrapper->getBodyDatas();
  for (int bi = 0; bi < bodyDatas.size(); bi++)
  {
    btTransform trans;
    bodyDatas[bi].m_Body->getMotionState()->getWorldTransform(trans);

//
    m_Box.SetSize(EigenTypes::Vector3f(0.05f, 0.05f, 0.05f).cast<double>());
    m_Box.Translation() = FromBullet(trans.getOrigin()).cast<double>();
    //m_Box.LinearTransformation() = Eigen::AngleAxis<double>(0.25f * M_PI, EigenTypes::Vector3::UnitZ()).toRotationMatrix();
    m_Box.LinearTransformation() = FromBullet(trans.getRotation()).toRotationMatrix();

    m_Box.Material().SetDiffuseLightColor(Color(1.0f, 1.0f, 1.0f, m_Alpha));
    m_Box.Material().SetAmbientLightColor(Color(1.0f, 1.0f, 1.0f, m_Alpha));
    PrimitiveBase::DrawSceneGraph(m_Box, m_Renderer);

  }



  m_Shader->Unbind();
}

EventHandlerAction PhysicsLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  //switch (ev.keysym.sym) {
  //default:
  return EventHandlerAction::PASS_ON;
  //}
}
