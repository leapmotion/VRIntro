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
  BulletWrapper() { zero(); s_LastHeldBody = NULL; m_FramesTilLastHeldBodyReset = 0; }
  ~BulletWrapper() { destroy(); }

  enum ShapeType
  {
    SHAPE_TYPE_SPHERE,
    SHAPE_TYPE_BOX
  };

  enum CollisionGroups
  {
    COLLISION_GROUP_HAND = 1,
    COLLISION_GROUP_DYNAMIC = 2,
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
  btRigidBody* utilAddFingerSphere(const EigenTypes::Vector3f& position, float radius, bool visible = false);
  void utilAddGround();

  void utilResetScene(const EigenTypes::Vector3f& refPosition);

  void removeAllBodies();
  void removeBody(btRigidBody* body);

  void utilSyncHeadRepresentation(const EigenTypes::Vector3f& headPosition, float deltaTime);
  void utilSyncHandRepresentations(const EigenTypes::Vector3f& headPosition, const std::vector<SkeletonHand>& skeletonHands, float deltaTime);

  void utilBounceBodiesAt2mAway();

  btRigidBody* utilFindClosestBody(const EigenTypes::Vector3f& point, const EigenTypes::Vector3f& acceptedHalfSpaceDirection);

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
    // Hand Id, from Leap API
    int m_Id;

    // Sphere bodies representing the hand's joints.
    std::vector<btRigidBody*> m_Bodies;

    // Held body, if pinching is active. Null otherwise.
    btRigidBody* m_HeldBody;

    // Is this representation actually active. Some invalid hands reported by tracking may be rejected (e.g. when they're too far & we know they're actually unwanted artifacts.)
    bool isActive() const { return m_Bodies.size(); }
  };

  void createHandRepresentation(const EigenTypes::Vector3f& headPosition, const SkeletonHand& skeletonHand, BulletWrapper::BulletHandRepresentation& handRepresentation);
  void destroyHandRepresentationFromWorld(BulletWrapper::BulletHandRepresentation& handRepresentation);

  void updateHandRepresentation(const SkeletonHand& skeletonHand, BulletWrapper::BulletHandRepresentation& handRepresentation, float deltaTime);
  void updateObjectHolding(const SkeletonHand& skeletonHand, BulletHandRepresentation& handRepresentation, float deltaTime);

  
  std::vector<BulletHandRepresentation> m_HandRepresentations;

  btRigidBody* m_HeadRepresentation;

  int m_FramesTilLastHeldBodyReset;
  static btRigidBody* s_LastHeldBody;

  static void MyNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
  {
    const btBroadphaseProxy* proxy = s_LastHeldBody ? s_LastHeldBody->getBroadphaseProxy(): NULL;

    if (collisionPair.m_pProxy0 == proxy || collisionPair.m_pProxy1 == proxy)
    {
      // cancel collision
      return;
    }

    // Do your collision logic here
    // Only dispatch the Bullet collision information if you want the physics to continue
    dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
  }
};

btRigidBody* BulletWrapper::s_LastHeldBody;

EigenTypes::Vector3f FromBullet(const btVector3& v) { return EigenTypes::Vector3f(v.x(), v.y(), v.z()); }
btVector3 ToBullet(const EigenTypes::Vector3f& v) {return btVector3(v.x(), v.y(), v.z()); }

Eigen::Quaternion<float> FromBullet(const btQuaternion& q) { return Eigen::Quaternion<float>(q.getW(), q.getX(), q.getY(), q.getZ()); }
btQuaternion ToBullet(const Eigen::Quaternion<float>& q) { return btQuaternion((btScalar)q.x(), (btScalar)q.y(), (btScalar)q.z(), (btScalar)q.w()); }

void BulletWrapper::init()
{
  assert(!m_Broadphase && !m_CollisionConfiguration && !m_Dispatcher && !m_Solver && !m_DynamicsWorld);

  m_Broadphase = new btDbvtBroadphase();

  m_CollisionConfiguration = new btDefaultCollisionConfiguration();
  m_Dispatcher = new btCollisionDispatcher(m_CollisionConfiguration);

  m_Dispatcher->setNearCallback(MyNearCallback);

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
  const bool notVisible = false;
  m_HeadRepresentation = utilAddFingerSphere(refPosition, 0.2f, notVisible);

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
  m_DynamicsWorld->addRigidBody(bodyData.m_Body, COLLISION_GROUP_DYNAMIC, COLLISION_GROUP_DYNAMIC | COLLISION_GROUP_HAND);
}

btRigidBody* BulletWrapper::utilAddFingerSphere(const EigenTypes::Vector3f& position, float radius, bool visible)
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
  bodyData.m_Visible = visible;
  bodyData.m_Body = new btRigidBody(fallRigidBodyCI);
  bodyData.m_Body->setActivationState(DISABLE_DEACTIVATION);
  m_DynamicsWorld->addRigidBody(bodyData.m_Body, COLLISION_GROUP_HAND, COLLISION_GROUP_DYNAMIC);
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

void BulletWrapper::utilSyncHandRepresentations(const EigenTypes::Vector3f& headPosition, const std::vector<SkeletonHand>& skeletonHands, float deltaTime)
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

  // Reset held body ptr.
  m_FramesTilLastHeldBodyReset = std::max(0, m_FramesTilLastHeldBodyReset - 1);
  if (!m_FramesTilLastHeldBodyReset) {s_LastHeldBody = NULL;}

  for (int hi = 0; hi < numFrameHands; hi++)
  {
    const SkeletonHand& skeletonHand = skeletonHands[hi];
    int handId = skeletonHand.id;
    int idx = (handIdToIdx.find(handId) != handIdToIdx.end()) ? handIdToIdx[handId] : -1;
    if (-1 == idx)
    {
      // Create hand
      BulletHandRepresentation handRepresentation;
      createHandRepresentation(headPosition, skeletonHand, handRepresentation);
      if (handRepresentation.isActive())
      {
        idx = m_HandRepresentations.size();
        handIdToIdx[handId] = idx;
        m_HandRepresentations.push_back(handRepresentation);
      }
    }
    else
    {
      // Update hand
      BulletHandRepresentation& handRepresentation = m_HandRepresentations[idx];
      updateHandRepresentation(skeletonHand, handRepresentation, deltaTime);
      //updateObjectHolding(skeletonHand, handRepresentation, deltaTime);
    }
  }

}

void BulletWrapper::utilBounceBodiesAt2mAway()
{
  btVector3 headPosition = m_HeadRepresentation->getWorldTransform().getOrigin();

  for (size_t i = 0; i < m_BodyDatas.size(); i++)
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

btRigidBody* BulletWrapper::utilFindClosestBody(const EigenTypes::Vector3f& point, const EigenTypes::Vector3f& acceptedHalfSpaceDirection )
{
  btRigidBody* closest = NULL;
  float minDist2 = 10000.0f;

  for (size_t i = 0; i < m_BodyDatas.size(); i++)
  {
    if (m_BodyDatas[i].m_Visible) // skip the hand bodies.
    {
      btRigidBody* body = m_BodyDatas[i].m_Body;
      btVector3 position = body->getWorldTransform().getOrigin();

      btVector3 handToBody = position - ToBullet(point);
      float dot = handToBody.dot(ToBullet(acceptedHalfSpaceDirection));
      float dist2 = handToBody.length2();

      const bool hardCutOff = true;
      if (hardCutOff)
      {
        if (0.0f < dot)
        {
          if (dist2 < minDist2)
          {
            minDist2 = dist2;
            closest = body;
          }
        }
      }
      else
      {
        // Alternative
        if (0.0f < dot)
        {
          float sin2 = std::sqrt(1 - dot * dot);
          float sin8 = sin2 * sin2 * sin2 * sin2;
          float sin32 = sin8 * sin8 *sin8 *sin8;
          dist2 *= 1.0f / (sin32 + 0.000001f);
        }
        if (dist2 < minDist2)
        {
          minDist2 = dist2;
          closest = body;
        }
      }


    }
  }

  return closest;
}

void BulletWrapper::createHandRepresentation(const EigenTypes::Vector3f& headPosition, const SkeletonHand& skeletonHand, BulletWrapper::BulletHandRepresentation& handRepresentation)
{
  handRepresentation.m_Id = skeletonHand.id;

  // Check hand's distance from the head
  float headToPalmDist = (headPosition - skeletonHand.center).norm();

  // Only reflect a SkeletonHand with an active BulletHandRepresentation when it comes closer than a distance to the head.
  if (headToPalmDist < 0.40f)
  {
    for (int i = 0; i < k_NumJoints; i++)
    {
      EigenTypes::Vector3f pos = skeletonHand.joints[i];
      btRigidBody* body = utilAddFingerSphere(pos, 0.01f);
      handRepresentation.m_Bodies.push_back(body);
    }
  }

  handRepresentation.m_HeldBody = NULL;
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



void BulletWrapper::updateObjectHolding(const SkeletonHand& skeletonHand, BulletHandRepresentation& handRepresentation, float deltaTime)
{
  const float strengthThreshold = 1.8f;
  if (skeletonHand.grabStrength >= strengthThreshold && handRepresentation.m_HeldBody == NULL)
  {
    // Find body to pick
    EigenTypes::Vector3f underHandDirection = -skeletonHand.rotationButNotReally * EigenTypes::Vector3f::UnitY();

    btRigidBody* closestBody = utilFindClosestBody(skeletonHand.getManipulationPoint(), underHandDirection);
    handRepresentation.m_HeldBody = closestBody;
  }

  if (skeletonHand.grabStrength < strengthThreshold)
  {
    // Let body go
    handRepresentation.m_HeldBody = NULL;
  }
  

  if (handRepresentation.m_HeldBody != NULL)
  {
    btRigidBody* body = handRepresentation.m_HeldBody;
    // Control held body
    s_LastHeldBody = body;
    m_FramesTilLastHeldBodyReset = 12;

    //handRepresentation.m_HeldBody->setCollisionFlags(0);

    // apply velocities or apply positions
    btVector3 target = ToBullet(skeletonHand.getManipulationPoint());// - skeletonHand.rotationButNotReally * EigenTypes::Vector3f(0.1f, 0.1f, 0.1f));
    btVector3 current = body->getWorldTransform().getOrigin();
    btVector3 targetVelocity = 0.5f * (target - current) / deltaTime;
    body->setLinearVelocity(targetVelocity);

    // convert from-to quaternions to angular velocity in world space
    {
      Eigen::Quaternionf currentRotation = FromBullet(body->getWorldTransform().getRotation());
      Eigen::Quaternionf targetRotation = Eigen::Quaternionf(skeletonHand.arbitraryRelatedRotation()); // breaks for left hand ???

      Eigen::Quaternionf delta = currentRotation.inverse() * targetRotation;
      Eigen::AngleAxis<float> angleAxis(delta);
      EigenTypes::Vector3f axis = angleAxis.axis();
      float angle = angleAxis.angle();
      const float pi = 3.1415926536f;
      if (angle > pi) angle -= 2.0f * pi;
      if (angle < -pi) angle += 2.0f * pi;

      EigenTypes::Vector3f angularVelocity = currentRotation * (axis * angle * 0.5f / deltaTime);
      body->setAngularVelocity(ToBullet(angularVelocity));
    }
  }
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
  if (accumulatedDeltaTime > fixedTimeStep)
  {
    m_BulletWrapper->utilSyncHandRepresentations(m_EyePos, m_SkeletonHands, (float)fixedTimeStep);
    m_BulletWrapper->utilSyncHeadRepresentation(m_EyePos, (float)fixedTimeStep);
    m_BulletWrapper->utilBounceBodiesAt2mAway();
    m_BulletWrapper->step(fixedTimeStep);
    accumulatedDeltaTime -= fixedTimeStep;
  }

  while (accumulatedDeltaTime > 10 * fixedTimeStep) { accumulatedDeltaTime -= fixedTimeStep; }


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
          m_Box.LinearTransformation() = FromBullet(trans.getRotation()).cast<double>().toRotationMatrix();

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
          m_Sphere.LinearTransformation() = FromBullet(trans.getRotation()).cast<double>().toRotationMatrix();
          m_Sphere.Material().SetDiffuseLightColor(Color(1.0f, 1.0f, 1.0f, m_Alpha));
          m_Sphere.Material().SetAmbientLightColor(Color(0.5f, 0.5f, 0.5f, m_Alpha));
          PrimitiveBase::DrawSceneGraph(m_Sphere, m_Renderer);
          break;
        }
      }
    }
  }

  if (false)
  {
    // Debug draw Basis
    for (size_t i = 0; i < m_SkeletonHands.size(); i++)
    {
      const SkeletonHand& hand = m_SkeletonHands[i];
      EigenTypes::Vector3f arrowEnd[3];
      arrowEnd[0] = hand.center + 0.1f * hand.arbitraryRelatedRotation() * EigenTypes::Vector3f::UnitX();
      arrowEnd[1] = hand.center + 0.1f * hand.arbitraryRelatedRotation() * EigenTypes::Vector3f::UnitY();
      arrowEnd[2] = hand.center + 0.1f * hand.arbitraryRelatedRotation() * EigenTypes::Vector3f::UnitZ();

      glLineWidth(1.0f);

      glColor4f(1, 0, 0, 1);
      glBegin(GL_LINES);
      glVertex3f(hand.center.x(), hand.center.y(), hand.center.z());
      glVertex3f(arrowEnd[0].x(), arrowEnd[0].y(), arrowEnd[0].z());
      glEnd();

      glColor4f(0, 1, 0, 1);
      glBegin(GL_LINES);
      glVertex3f(hand.center.x(), hand.center.y(), hand.center.z());
      glVertex3f(arrowEnd[1].x(), arrowEnd[1].y(), arrowEnd[1].z());
      glEnd();

      glColor4f(0, 0, 1, 1);
      glBegin(GL_LINES);
      glVertex3f(hand.center.x(), hand.center.y(), hand.center.z());
      glVertex3f(arrowEnd[2].x(), arrowEnd[2].y(), arrowEnd[2].z());
      glEnd();
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
