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

  enum ShapeType
  {
    SHAPE_TYPE_SPHERE,
    SHAPE_TYPE_BOX
  };

  struct BodyData
  {
    btRigidBody* m_Body;
    std::shared_ptr<btCollisionShape> m_SharedShape;

    ShapeType m_ShapeType;
  };

  void init();
  void destroy();

  void step(TimeDelta deltaTime);

  void utilSetupScene(const EigenTypes::Vector3f& refPosition);

  void utilAddCube(const EigenTypes::Vector3f& position);
  btRigidBody* utilAddFingerSphere(const EigenTypes::Vector3f& position);
  void utilAddGround();

  void utilResetScene(const EigenTypes::Vector3f& refPosition);

  void removeAllBodies();
  void removeBody(btRigidBody* body);

  void utilSyncHandRepresentations(const std::vector<SkeletonHand>& m_SkeletonHands, float deltaTime);

  const std::vector<BodyData>& getBodyDatas() const { return m_BodyDatas; }

private:
  void zero();

  btBroadphaseInterface* m_Broadphase;
  btDefaultCollisionConfiguration* m_CollisionConfiguration;
  btCollisionDispatcher* m_Dispatcher;
  btSequentialImpulseConstraintSolver* m_Solver;
  btDiscreteDynamicsWorld* m_DynamicsWorld;

  std::vector<BodyData> m_BodyDatas;

  const int k_NumJoints = 23;

public:
  // Hack

  struct BulletHandRepresentation
  {
    int m_Id;
    std::vector<btRigidBody*> m_Bodies;
  };

  void createHandRepresentation(const SkeletonHand& skeletonHand, BulletWrapper::BulletHandRepresentation& handRepresentation);
  void updateHandRepresentation(const SkeletonHand& skeletonHand, BulletWrapper::BulletHandRepresentation& handRepresentation, float deltaTime);

  void destroyHandRepresentationFromWorld(BulletWrapper::BulletHandRepresentation& handRepresentation);
  
  std::vector<BulletHandRepresentation> m_HandRepresentations;
};


EigenTypes::Vector3f FromBullet(const btVector3& v) { return EigenTypes::Vector3f(v.x(), v.y(), v.z()); }
btVector3 ToBullet(const EigenTypes::Vector3f& v) {return btVector3(v.x(), v.y(), v.z()); }

Eigen::Quaternion<double> FromBullet(const btQuaternion& q) { return Eigen::Quaternion<double>(q.getW(), q.getX(), q.getY(), q.getZ()); }
btQuaternion ToBullet(const Eigen::Quaternion<double>& q) { return btQuaternion((btScalar)q.x(), (btScalar)q.y(), (btScalar)q.z(), (btScalar)q.w()); }

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

void BulletWrapper::utilResetScene(const EigenTypes::Vector3f& refPosition)
{
  for (int i = 0; i < m_HandRepresentations.size(); i++)
  {
    destroyHandRepresentationFromWorld(m_HandRepresentations[i]);
  }
  m_HandRepresentations.clear();

  removeAllBodies();

  utilSetupScene(refPosition);
}

void BulletWrapper::step(TimeDelta deltaTime)
{
  m_DynamicsWorld->stepSimulation((btScalar)deltaTime);
}

void BulletWrapper::utilSetupScene(const EigenTypes::Vector3f& refPosition)
{
   // setup walls 

  for (int y = 0; y < 7; y++)
    for (int x = 0; x < 9; x++)
    {
      utilAddCube(refPosition + EigenTypes::Vector3f(-0.2f + 0.05f * x, -0.15 + 0.05f * y, -0.5f));

      utilAddCube(refPosition + EigenTypes::Vector3f(-0.2f + 0.05f * x, -0.15 + 0.05f * y, 0.5f));

      utilAddCube(refPosition + EigenTypes::Vector3f(0.5f, - 0.15 + 0.05f * y, -0.2f + 0.05f * x));

      utilAddCube(refPosition + EigenTypes::Vector3f(-0.5f, -0.15 + 0.05f * y, -0.2f + 0.05f * x));
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
  bodyData.m_ShapeType = SHAPE_TYPE_BOX;
  m_DynamicsWorld->addRigidBody(bodyData.m_Body);
}

btRigidBody* BulletWrapper::utilAddFingerSphere(const EigenTypes::Vector3f& position)
{
  m_BodyDatas.resize(m_BodyDatas.size() + 1);
  BodyData& bodyData = m_BodyDatas.back();

  bodyData.m_SharedShape.reset(new btSphereShape(0.01f));

  btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x(), position.y(), position.z())));
  btScalar mass = 1;
  btVector3 fallInertia(0, 0, 0);
  bodyData.m_SharedShape->calculateLocalInertia(mass, fallInertia);
  btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, bodyData.m_SharedShape.get(), fallInertia);
  bodyData.m_ShapeType = SHAPE_TYPE_SPHERE;
  bodyData.m_Body = new btRigidBody(fallRigidBodyCI);
  m_DynamicsWorld->addRigidBody(bodyData.m_Body);
  return bodyData.m_Body;
}

void BulletWrapper::removeAllBodies()
{
  for (unsigned int i = 0; i < m_BodyDatas.size(); i++)
  {
    btRigidBody* body = m_BodyDatas[i].m_Body;

    m_DynamicsWorld->removeRigidBody(body);
    delete body->getMotionState();
    delete body;

    // Shape is automatically released via the shared pointer
  }

  m_BodyDatas.clear();
}

void BulletWrapper::removeBody(btRigidBody* bodyToRemove)
{
  for (unsigned int i = 0; i < m_BodyDatas.size(); i++)
  {
    btRigidBody* body = m_BodyDatas[i].m_Body;

    if (body == bodyToRemove)
    {
      m_DynamicsWorld->removeRigidBody(body);
      delete body->getMotionState();
      delete body;
      m_BodyDatas.erase(m_BodyDatas.begin()+i);
      break;
    }

    // Shape is automatically released via the shared pointer
  }
}

void BulletWrapper::utilSyncHandRepresentations(const std::vector<SkeletonHand>& skeletonHands, float deltaTime)
{
  // Add new, remove unneded, update persisting HandRepresentations

  std::map<int, int> handIdToIdx;
  std::map<int, int> handIdsVisible;

  int numFrameHands = skeletonHands.size();
  for (int hi = 0; hi < numFrameHands; hi++)
  {
    int handId = skeletonHands[hi].id;
    handIdsVisible[handId] = 1;
  }

  for (unsigned hi = 0; hi < m_HandRepresentations.size(); hi++)
  {
    BulletHandRepresentation& handRepresentation = m_HandRepresentations[hi];
    //lmPtr<lmHand> leapHand = m_leapHands[hi];
    int visible = handIdsVisible[handRepresentation.m_Id];
    if (visible)
    {
      handIdToIdx[handRepresentation.m_Id] = hi;
    }
    else
    {
      // destroy hand
      destroyHandRepresentationFromWorld(handRepresentation);
      m_HandRepresentations.erase(m_HandRepresentations.begin() + hi);
      hi--;
    }
  }

  for (int hi = 0; hi < numFrameHands; hi++)
  {
    const SkeletonHand& skeletonHand = skeletonHands[hi];
    int handId = skeletonHand.id;
    int idx = (handIdToIdx.find(handId) != handIdToIdx.end()) ? handIdToIdx[handId] : -1;
    if (-1 == idx)
    {
      // Create hand
      BulletHandRepresentation handRepresentation;
      createHandRepresentation(skeletonHand, handRepresentation);

      idx = m_HandRepresentations.size();
      handIdToIdx[handId] = idx;
      m_HandRepresentations.push_back(handRepresentation);

      //addHandRepresentationToWorld(handRepresentation);
    }
    else
    {
      // Update hand
      BulletHandRepresentation& handRepresentation = m_HandRepresentations[idx];
      updateHandRepresentation(skeletonHand, handRepresentation, deltaTime);

    }
  }

}

void BulletWrapper::createHandRepresentation(const SkeletonHand& skeletonHand, BulletWrapper::BulletHandRepresentation& handRepresentation)
{
  handRepresentation.m_Id = skeletonHand.id;

  for (int i = 0; i < k_NumJoints; i++)
  {
    EigenTypes::Vector3f pos = skeletonHand.joints[i];
    btRigidBody* body = utilAddFingerSphere(pos);
    handRepresentation.m_Bodies.push_back(body);
  }
}

void BulletWrapper::updateHandRepresentation(const SkeletonHand& skeletonHand, BulletWrapper::BulletHandRepresentation& handRepresentation, float deltaTime)
{
  for (unsigned int i = 0; i < handRepresentation.m_Bodies.size(); i++)
  {
    EigenTypes::Vector3f jointPos = skeletonHand.joints[i];
    btRigidBody* body = handRepresentation.m_Bodies[i];

    // apply velocities or apply positions
    btVector3 target = ToBullet(jointPos);
    btVector3 current = body->getWorldTransform().getOrigin();
    btVector3 targetVelocity = (target - current) / deltaTime;
    body->setLinearVelocity(targetVelocity);
    body->setAngularVelocity(btVector3(0,0,0));
  }
}

void BulletWrapper::destroyHandRepresentationFromWorld(BulletWrapper::BulletHandRepresentation& handRepresentation)
{
  for (unsigned int i = 0; i < handRepresentation.m_Bodies.size(); i++)
  {
    removeBody(handRepresentation.m_Bodies[i]);
  }
  handRepresentation.m_Bodies.clear();
  handRepresentation.m_Id = -1;
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

void PhysicsLayer::OnSelected()
{
  if (m_BulletWrapper) m_BulletWrapper->utilResetScene(m_EyePos);
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
  static float accumulatedDeltaTime = 0.0f;
  float fixedTimeStep = 1.0f / 60.0f;
  accumulatedDeltaTime += (float)real_time_delta;
  while (accumulatedDeltaTime > fixedTimeStep)
  {
    m_BulletWrapper->utilSyncHandRepresentations(m_SkeletonHands, (float)fixedTimeStep);
    m_BulletWrapper->step(fixedTimeStep);
    accumulatedDeltaTime -= fixedTimeStep;
  }


  //static std::vector<float> times;
  //times.push_back(real_time_delta);
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
  m_Sphere.Material().SetAmbientLightingProportion(0.3f);

  // Render dynamics
  const std::vector<BulletWrapper::BodyData>& bodyDatas = m_BulletWrapper->getBodyDatas();
  for (unsigned int bi = 0; bi < bodyDatas.size(); bi++)
  {
    btTransform trans;
    bodyDatas[bi].m_Body->getMotionState()->getWorldTransform(trans);

    switch (bodyDatas[bi].m_Body->getCollisionShape()->getShapeType())
    {
    case BOX_SHAPE_PROXYTYPE:
      m_Box.SetSize(EigenTypes::Vector3f(0.05f, 0.05f, 0.05f).cast<double>());
      m_Box.Translation() = FromBullet(trans.getOrigin()).cast<double>();
      //m_Box.LinearTransformation() = Eigen::AngleAxis<double>(0.25f * M_PI, EigenTypes::Vector3::UnitZ()).toRotationMatrix();
      m_Box.LinearTransformation() = FromBullet(trans.getRotation()).toRotationMatrix();

      m_Box.Material().SetDiffuseLightColor(Color(1.0f, 1.0f, 1.0f, m_Alpha));
      m_Box.Material().SetAmbientLightColor(Color(0.5f, 0.5f, 0.5f, m_Alpha));
      PrimitiveBase::DrawSceneGraph(m_Box, m_Renderer);
      break;
    case SPHERE_SHAPE_PROXYTYPE:
      if (false)
      {
        m_Sphere.SetRadius(0.01f);// SetSize(EigenTypes::Vector3f(0.05f, 0.05f, 0.05f).cast<double>());
        m_Sphere.Translation() = FromBullet(trans.getOrigin()).cast<double>();
        m_Sphere.LinearTransformation() = FromBullet(trans.getRotation()).toRotationMatrix();
        m_Sphere.Material().SetDiffuseLightColor(Color(1.0f, 1.0f, 1.0f, m_Alpha));
        m_Sphere.Material().SetAmbientLightColor(Color(0.5f, 0.5f, 0.5f, m_Alpha));
        PrimitiveBase::DrawSceneGraph(m_Sphere, m_Renderer);
      }
      break;
    }
//

  }



  m_Shader->Unbind();
}

EventHandlerAction PhysicsLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  switch (ev.keysym.sym) {
    case 'r':
      if (m_BulletWrapper) m_BulletWrapper->utilResetScene(m_EyePos);
      break;
    default:
      return EventHandlerAction::PASS_ON;
  }
}
