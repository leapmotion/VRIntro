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

    ShapeType m_ShapeType; // unused now
    bool m_Visible;
  };

  void init();
  void destroy();

  void step(TimeDelta deltaTime);

  void utilSetupScene(const EigenTypes::Vector3f& refPosition);

  void utilAddCube(const EigenTypes::Vector3f& position, const EigenTypes::Vector3f& halfExtents);
  btRigidBody* utilAddFingerSphere(const EigenTypes::Vector3f& position, float radius);
  void utilAddGround();

  void utilResetScene(const EigenTypes::Vector3f& refPosition);

  void removeAllBodies();
  void removeBody(btRigidBody* body);

  void utilSyncHeadRepresentation(const EigenTypes::Vector3f& headPosition, float deltaTime);
  void utilSyncHandRepresentations(const std::vector<SkeletonHand>& m_SkeletonHands, float deltaTime);

  void utilBounceBodiesAt2mAway();

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

  btRigidBody* m_HeadRepresentation;
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
  for (unsigned int i = 0; i < m_HandRepresentations.size(); i++)
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

  float halfSize = 0.05f;
  EigenTypes::Vector3f boxHalfExtents(halfSize, halfSize, halfSize);

  float spacing = 2.f * halfSize;

  float zDist = 0.5f;
  // for Dragonfly
  zDist = 0.6f * zDist;

  int halfCountX = std::min(4, int((zDist-halfSize)/spacing));
  int halfCountY = 3;


  for (int y = -halfCountY; y <= halfCountY; y++)
    for (int x = -halfCountX; x <= halfCountX; x++)
    {
      utilAddCube(refPosition + EigenTypes::Vector3f(spacing * x, spacing * y, -zDist), boxHalfExtents);

      utilAddCube(refPosition + EigenTypes::Vector3f(spacing * x, spacing * y, zDist), boxHalfExtents);

      utilAddCube(refPosition + EigenTypes::Vector3f(zDist, spacing * y, spacing * x), boxHalfExtents);

      utilAddCube(refPosition + EigenTypes::Vector3f(-zDist, spacing * y, spacing * x), boxHalfExtents);
    }

  // Add head representation
  m_HeadRepresentation = utilAddFingerSphere(refPosition, 0.1f);

}

void BulletWrapper::utilAddCube(const EigenTypes::Vector3f& position, const EigenTypes::Vector3f& halfExtents)
{
  m_BodyDatas.resize(m_BodyDatas.size() + 1);
  BodyData& bodyData = m_BodyDatas.back();

  bodyData.m_SharedShape.reset(new btBoxShape(ToBullet(halfExtents)));

  btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x(), position.y(), position.z())));

  btScalar mass = 1;
  btVector3 fallInertia(0, 0, 0);
  bodyData.m_SharedShape->calculateLocalInertia(mass, fallInertia);
  btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, bodyData.m_SharedShape.get(), fallInertia);
  bodyData.m_Body = new btRigidBody(fallRigidBodyCI);
  bodyData.m_ShapeType = SHAPE_TYPE_BOX;
  bodyData.m_Visible = true;
  bodyData.m_Body->setActivationState(DISABLE_DEACTIVATION);
  m_DynamicsWorld->addRigidBody(bodyData.m_Body);
}

btRigidBody* BulletWrapper::utilAddFingerSphere(const EigenTypes::Vector3f& position, float radius)
{
  m_BodyDatas.resize(m_BodyDatas.size() + 1);
  BodyData& bodyData = m_BodyDatas.back();

  bodyData.m_SharedShape.reset(new btSphereShape(radius));

  btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x(), position.y(), position.z())));
  btScalar mass = 1;
  btVector3 fallInertia(0, 0, 0);
  bodyData.m_SharedShape->calculateLocalInertia(mass, fallInertia);
  btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, bodyData.m_SharedShape.get(), fallInertia);
  bodyData.m_ShapeType = SHAPE_TYPE_SPHERE;
  bodyData.m_Visible = false;
  bodyData.m_Body = new btRigidBody(fallRigidBodyCI);
  bodyData.m_Body->setActivationState(DISABLE_DEACTIVATION);
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

void BulletWrapper::utilSyncHeadRepresentation(const EigenTypes::Vector3f& headPosition, float deltaTime)
{
  // apply velocities or apply positions
  btVector3 target = ToBullet(headPosition);
  btVector3 current = m_HeadRepresentation->getWorldTransform().getOrigin();
  btVector3 targetVelocity = (target - current) / deltaTime;
  m_HeadRepresentation->setLinearVelocity(targetVelocity);
  m_HeadRepresentation->setAngularVelocity(btVector3(0, 0, 0));
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

void BulletWrapper::utilBounceBodiesAt2mAway()
{
  btVector3 headPosition = m_HeadRepresentation->getWorldTransform().getOrigin();

  for (int i = 0; i < m_BodyDatas.size(); i++)
  {
    btRigidBody* body = m_BodyDatas[i].m_Body;
    btVector3 position = body->getWorldTransform().getOrigin();

    if ((position - headPosition).length2() > 2.0f * 2.0f)
    {
      // ensure velocity is not away from center
      btVector3 velocity = body->getLinearVelocity();
      btVector3 toCenter = (headPosition - position).normalized();

      float dot = velocity.dot(toCenter);
      if (dot < 0.0f)
      {
        velocity -= 2.0f * dot * toCenter;
        body->setLinearVelocity(velocity);
      }
    }
  }
}

void BulletWrapper::createHandRepresentation(const SkeletonHand& skeletonHand, BulletWrapper::BulletHandRepresentation& handRepresentation)
{
  handRepresentation.m_Id = skeletonHand.id;

  for (int i = 0; i < k_NumJoints; i++)
  {
    EigenTypes::Vector3f pos = skeletonHand.joints[i];
    btRigidBody* body = utilAddFingerSphere(pos, 0.01f);
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
    m_BulletWrapper->utilSyncHeadRepresentation(m_EyePos, (float)fixedTimeStep);
    m_BulletWrapper->utilBounceBodiesAt2mAway();
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
    const BulletWrapper::BodyData& bodyData = bodyDatas[bi];

    if (bodyData.m_Visible)
    {
      btTransform trans;
      bodyData.m_Body->getMotionState()->getWorldTransform(trans);

      btCollisionShape* shape = bodyData.m_Body->getCollisionShape();
      switch (shape->getShapeType())
      {
      case BOX_SHAPE_PROXYTYPE:
        {
          const btBoxShape* boxShape = static_cast<btBoxShape*>(shape);
          m_Box.SetSize(2.0f * FromBullet(boxShape->getHalfExtentsWithMargin()).cast<double>());
          m_Box.Translation() = FromBullet(trans.getOrigin()).cast<double>();
          m_Box.LinearTransformation() = FromBullet(trans.getRotation()).toRotationMatrix();

          m_Box.Material().SetDiffuseLightColor(Color(1.0f, 1.0f, 1.0f, m_Alpha));
          m_Box.Material().SetAmbientLightColor(Color(0.5f, 0.5f, 0.5f, m_Alpha));
          PrimitiveBase::DrawSceneGraph(m_Box, m_Renderer);
          break;
        }
      case SPHERE_SHAPE_PROXYTYPE:
        {
          const btSphereShape* sphereShape = static_cast<btSphereShape*>(shape);
          m_Sphere.SetRadius(sphereShape->getRadius());
          m_Sphere.Translation() = FromBullet(trans.getOrigin()).cast<double>();
          m_Sphere.LinearTransformation() = FromBullet(trans.getRotation()).toRotationMatrix();
          m_Sphere.Material().SetDiffuseLightColor(Color(1.0f, 1.0f, 1.0f, m_Alpha));
          m_Sphere.Material().SetAmbientLightColor(Color(0.5f, 0.5f, 0.5f, m_Alpha));
          PrimitiveBase::DrawSceneGraph(m_Sphere, m_Renderer);
          break;
        }
      }
    }
  }



  m_Shader->Unbind();
}

EventHandlerAction PhysicsLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {

  EventHandlerAction resultingEventHandlerAction = EventHandlerAction::CONSUME;

  switch (ev.keysym.sym) {
    case 'r':
      if (m_BulletWrapper) m_BulletWrapper->utilResetScene(m_EyePos);
      break;
    default:
      resultingEventHandlerAction = EventHandlerAction::PASS_ON;
  }

  return resultingEventHandlerAction;
}
