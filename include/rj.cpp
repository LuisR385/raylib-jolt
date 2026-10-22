#include "rj.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
//NOTE : ONLY ONE JOLT RUNTIME.
static RayJolt::JoltRuntime g_joltRuntime;

namespace RayJolt {

    JoltRuntime::~JoltRuntime() {
        if (isInitialized)
            End();
    }

    //====================Start declare Struct/Class INFO====================


    bool JoltRuntime::Init() {

        // 同じオブジェクトでの二重初期化を防ぐ

        if (!isInitialized) {
            if (JPH::Factory::sInstance != nullptr)
                return false;
            JPH::RegisterDefaultAllocator();

            JPH::Factory::sInstance = new JPH::Factory();

            JPH::RegisterTypes();

            tempAllocator_ =
                std::make_unique<JPH::TempAllocatorImpl>(
                    10 * 1024 * 1024);

            const unsigned cores = std::thread::hardware_concurrency();
            jobSystem_ =
                std::make_unique<JPH::JobSystemThreadPool>(
                    JPH::cMaxPhysicsJobs,
                    JPH::cMaxPhysicsBarriers,
                    static_cast<int>(cores > 1 ? cores - 1 : 1)
                );

            isInitialized = true;
            TraceLog(LOG_INFO, "ray jolt Initialized Success."); //TODO : call rayjolt version api.
            return true;
        }
        TraceLog(LOG_INFO, "ray jolt Initialize failed.");
        return false;
    }

    void JoltRuntime::End()
    {
        // 初期化していなければ、片付けも不要

        if (isInitialized) {
            jobSystem_.reset();
            tempAllocator_.reset();

            JPH::UnregisterTypes();

            delete JPH::Factory::sInstance;
            JPH::Factory::sInstance = nullptr;
            isInitialized = false;
            TraceLog(LOG_INFO, "rayjolt Process finished successfully.");
            return;
        }
        
        TraceLog(LOG_INFO, "rayjolt Process failed successfully!");
        
    
    }

    PhysicsWorld::~PhysicsWorld() {
        Shutdown();
    }

    bool PhysicsWorld::Init()
    {
        layerPairs_.EnableCollision(ObjectLayers::NON_MOVING, ObjectLayers::MOVING);
        layerPairs_.EnableCollision(ObjectLayers::MOVING, ObjectLayers::MOVING);
        broadPhaseLayers_.MapObjectToBroadPhaseLayer(ObjectLayers::NON_MOVING, BroadPhaseLayers::NON_MOVING);
        broadPhaseLayers_.MapObjectToBroadPhaseLayer(ObjectLayers::MOVING, BroadPhaseLayers::MOVING);
        broadPhaseFilter_ = std::make_unique<JPH::ObjectVsBroadPhaseLayerFilterTable>(
            broadPhaseLayers_, 2, layerPairs_, 2);

        physicsSystem_.Init(1024, 0, 1024, 1024, broadPhaseLayers_, *broadPhaseFilter_, layerPairs_);

        return true;
    }

    void PhysicsWorld::Update(float deltaTime, int collisionStep) {
        RJ_Update(*this, deltaTime, collisionStep);
    }

    void PhysicsWorld::Shutdown() {
        for (auto& character : characters_)
            character->RemoveFromPhysicsSystem();
        characters_.clear();
        jumpingCharacters_.clear();
        JPH::BodyIDVector bodyIDs;
        physicsSystem_.GetBodies(bodyIDs);
        auto& bodies = physicsSystem_.GetBodyInterface();
        for (const auto& id : bodyIDs) {
            if (bodies.IsAdded(id))
                bodies.RemoveBody(id);
            bodies.DestroyBody(id);
        }
    }

    bool PhysicsWorld::ContainsBody(BodyHandle body) const {
        if (!body.IsValid())
            return false;
        JPH::BodyIDVector bodyIDs;
        physicsSystem_.GetBodies(bodyIDs);
        return std::find(bodyIDs.begin(), bodyIDs.end(), body.GetJoltID()) != bodyIDs.end();
    }

    bool PhysicsWorld::DestroyBody(BodyHandle& body) {
        if (!ContainsBody(body)) {
            body = BodyHandle{};
            return false;
        }
        auto& bodies = physicsSystem_.GetBodyInterface();
        const auto id = body.GetJoltID();
        for (auto it = characters_.begin(); it != characters_.end(); ++it) {
            if ((*it)->GetBodyID() == id) {
                (*it)->RemoveFromPhysicsSystem();
                characters_.erase(it);
                jumpingCharacters_.erase(std::remove(jumpingCharacters_.begin(), jumpingCharacters_.end(), id), jumpingCharacters_.end());
                body = BodyHandle{};
                return true;
            }
        }
        if (bodies.IsAdded(id))
            bodies.RemoveBody(id);
        bodies.DestroyBody(id);
        body = BodyHandle{};
        return true;
    }

    BodyHandle PhysicsWorld::CreateSphere(const SphereDesc& desc) {
        if (!std::isfinite(desc.radius) || desc.radius <= 0.0f)
            return BodyHandle{};
        JPH::BodyCreationSettings settings(
            new JPH::SphereShape(desc.radius),
            Detail::ToJoltRVec3(desc.position),
            Detail::ToJoltQuaternion(desc.rotation),
            desc.isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
            desc.isStatic ? ObjectLayers::NON_MOVING : ObjectLayers::MOVING);
        settings.mRestitution = desc.restitution;
        settings.mFriction = desc.friction;
        return BodyHandle(physicsSystem_.GetBodyInterface().CreateAndAddBody(settings,
            desc.isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate));
    }

    BodyHandle PhysicsWorld::CreateCapsule(const CapsuleDesc& desc) {
        if (!std::isfinite(desc.radius) || desc.radius <= 0.0f ||
            !std::isfinite(desc.height) || desc.height < 2.0f * desc.radius)
            return BodyHandle{};

        const float halfHeight = desc.height * 0.5f - desc.radius;
        JPH::RefConst<JPH::Shape> shape;
        if (halfHeight == 0.0f)
            shape = new JPH::SphereShape(desc.radius);
        else
            shape = new JPH::CapsuleShape(halfHeight, desc.radius);

        JPH::BodyCreationSettings settings(
            shape.GetPtr(),
            Detail::ToJoltRVec3(desc.position),
            Detail::ToJoltQuaternion(desc.rotation),
            desc.isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
            desc.isStatic ? ObjectLayers::NON_MOVING : ObjectLayers::MOVING);
        settings.mRestitution = desc.restitution;
        settings.mFriction = desc.friction;
        return BodyHandle(physicsSystem_.GetBodyInterface().CreateAndAddBody(settings,
            desc.isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate));
    }

    Vector3 PhysicsWorld::GetPosition(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        return Detail::ToRaylibRVector3(physicsSystem_.GetBodyInterface().GetPosition(body.GetJoltID()));
    }

    void PhysicsWorld::SetPosition(BodyHandle body, Vector3 value) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        physicsSystem_.GetBodyInterface().SetPosition(body.GetJoltID(), Detail::ToJoltRVec3(value), JPH::EActivation::Activate);
    }

    Quaternion PhysicsWorld::GetRotation(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        return Detail::ToRaylibQuaternion(physicsSystem_.GetBodyInterface().GetRotation(body.GetJoltID()));
    }

    void PhysicsWorld::SetRotation(BodyHandle body, Quaternion value) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        physicsSystem_.GetBodyInterface().SetRotation(body.GetJoltID(), Detail::ToJoltQuaternion(value), JPH::EActivation::Activate);
    }

    Vector3 PhysicsWorld::GetLinearVelocity(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        return Detail::ToRaylibVector3(physicsSystem_.GetBodyInterface().GetLinearVelocity(body.GetJoltID()));
    }

    void PhysicsWorld::SetLinearVelocity(BodyHandle body, Vector3 value) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        physicsSystem_.GetBodyInterface().SetLinearVelocity(body.GetJoltID(), Detail::ToJoltVec3(value));
    }

    Vector3 PhysicsWorld::GetAngularVelocity(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        return Detail::ToRaylibVector3(physicsSystem_.GetBodyInterface().GetAngularVelocity(body.GetJoltID()));
    }

    void PhysicsWorld::SetAngularVelocity(BodyHandle body, Vector3 value) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        physicsSystem_.GetBodyInterface().SetAngularVelocity(body.GetJoltID(), Detail::ToJoltVec3(value));
    }

    void PhysicsWorld::AddForce(BodyHandle body, Vector3 force) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        physicsSystem_.GetBodyInterface().AddForce(body.GetJoltID(), Detail::ToJoltVec3(force));
    }

    void PhysicsWorld::AddImpulse(BodyHandle body, Vector3 impulse) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        physicsSystem_.GetBodyInterface().AddImpulse(body.GetJoltID(), Detail::ToJoltVec3(impulse));
    }

    BodyHandle PhysicsWorld::CreateCharacter(const CharacterDesc& desc) {
        if (!std::isfinite(desc.radius) || desc.radius <= 0.0f ||
            !std::isfinite(desc.height) || desc.height <= 2.0f * desc.radius ||
            !std::isfinite(desc.mass) || desc.mass <= 0.0f ||
            !std::isfinite(desc.position.x) || !std::isfinite(desc.position.y) ||
            !std::isfinite(desc.position.z) ||
            physicsSystem_.GetNumBodies() >= physicsSystem_.GetMaxBodies())
            return BodyHandle{};

        const float halfCylinderHeight = desc.height * 0.5f - desc.radius;
        JPH::CharacterSettings settings;
        settings.mShape = new JPH::CapsuleShape(halfCylinderHeight, desc.radius);
        settings.mMass = desc.mass;
        settings.mLayer = ObjectLayers::MOVING;
        settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), halfCylinderHeight);
        JPH::Ref<JPH::Character> character = new JPH::Character(
            &settings, Detail::ToJoltRVec3(desc.position), JPH::Quat::sIdentity(), 0, &physicsSystem_);
        characters_.push_back(character);
        character->AddToPhysicsSystem();
        return BodyHandle(character->GetBodyID());
    }

    bool PhysicsWorld::IsGrounded(BodyHandle body) const {
        for (const auto& character : characters_) {
            if (character->GetBodyID() == body.GetJoltID())
                return character->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
        }
        throw std::invalid_argument("Body is not a character in this world");
    }

    bool PhysicsWorld::Jump(BodyHandle body, float speed) {
        for (auto& character : characters_) {
            if (character->GetBodyID() != body.GetJoltID())
                continue;
            if (!std::isfinite(speed) || speed <= 0.0f)
                return false;
            const auto id = body.GetJoltID();
            if (std::find(jumpingCharacters_.begin(), jumpingCharacters_.end(), id) != jumpingCharacters_.end())
                return false;
            character->PostSimulation(0.05f);
            if (character->GetGroundState() != JPH::CharacterBase::EGroundState::OnGround)
                return false;
            auto velocity = character->GetLinearVelocity();
            velocity.SetY(character->GetGroundVelocity().GetY() + speed);
            jumpingCharacters_.push_back(id);
            character->SetLinearVelocity(velocity);
            character->Activate();
            return true;
        }
        throw std::invalid_argument("Body is not a character in this world");
    }

    void PhysicsWorld::UpdateCharacterContacts() {
        for (auto& character : characters_) {
            character->PostSimulation(0.05f);
            if (character->GetGroundState() != JPH::CharacterBase::EGroundState::OnGround ||
                character->GetLinearVelocity().GetY() <= character->GetGroundVelocity().GetY()) {
                const auto id = character->GetBodyID();
                jumpingCharacters_.erase(std::remove(jumpingCharacters_.begin(), jumpingCharacters_.end(), id), jumpingCharacters_.end());
            }
        }
    }

    bool PhysicsWorld::CastRay(const Ray& ray, float maxDistance, RayHit& hit) const {
        hit = {};
        if (!std::isfinite(maxDistance) || maxDistance <= 0.0f ||
            !std::isfinite(ray.position.x) || !std::isfinite(ray.position.y) ||
            !std::isfinite(ray.position.z) || !std::isfinite(ray.direction.x) ||
            !std::isfinite(ray.direction.y) || !std::isfinite(ray.direction.z))
            return false;

        const double length = std::hypot(double(ray.direction.x), double(ray.direction.y), double(ray.direction.z));
        if (length == 0.0)
            return false;
        const JPH::Vec3 displacement(
            float(ray.direction.x / length * maxDistance),
            float(ray.direction.y / length * maxDistance),
            float(ray.direction.z / length * maxDistance));
        const JPH::RRayCast joltRay(Detail::ToJoltRVec3(ray.position), displacement);
        JPH::RayCastResult result;
        if (!physicsSystem_.GetNarrowPhaseQuery().CastRay(joltRay, result))
            return false;

        JPH::BodyLockRead lock(physicsSystem_.GetBodyLockInterface(), result.mBodyID);
        if (!lock.Succeeded())
            return false;
        const auto point = joltRay.GetPointOnRay(result.mFraction);
        hit.body = BodyHandle(result.mBodyID);
        hit.point = Detail::ToRaylibRVector3(point);
        hit.normal = Detail::ToRaylibVector3(
            lock.GetBody().GetWorldSpaceSurfaceNormal(result.mSubShapeID2, point));
        hit.distance = result.mFraction * maxDistance;
        return true;
    }

    BodyHandle PhysicsWorld::CreateBox(const BoxDesc& desc) {
        //put int halfExtents
        JPH::Vec3 halfExtents(desc.size.x * 0.5f, desc.size.y * 0.5f, desc.size.z * 0.5f);

        //Set motionType / Object of layer
        JPH::EMotionType motionType = desc.isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
        JPH::ObjectLayer layer = desc.isStatic ? ObjectLayers::NON_MOVING : ObjectLayers::MOVING;

        //setting
        JPH::BodyCreationSettings settings(
            new JPH::BoxShape(halfExtents),
            Detail::ToJoltRVec3(desc.position),
            Detail::ToJoltQuaternion(desc.rotation),
            motionType,
            layer
        );

        settings.mRestitution = desc.restitution;
        settings.mFriction = desc.friction;

        //NOTE : isStatic->true = DontActivate.
        JPH::EActivation activation = desc.isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;

        JPH::BodyID jphId = physicsSystem_.GetBodyInterface().CreateAndAddBody(settings, activation);
        return BodyHandle(jphId);
    }

    //====================End declare Struct/Class INFO====================

    JPH::EPhysicsUpdateError JoltRuntime::Update(JPH::PhysicsSystem& physics, float timeStep, int collisionSteps)
    {
        JPH_ASSERT(isInitialized);
        return physics.Update(timeStep, collisionSteps, tempAllocator_.get(), jobSystem_.get());
    }

    bool RayJoltContext::Init() {
        if (world_)
            return true;
        if (!runtime_.Init())
            return false;

        auto world = std::make_unique<PhysicsWorld>();
        if (!world->Init()) {
            world.reset();
            runtime_.End();
            return false;
        }
        world_ = std::move(world);
        return true;
    }

    void RayJoltContext::End() {
        if (!world_)
            return;
        world_.reset();
        runtime_.End();
    }

    JPH::EPhysicsUpdateError RayJoltContext::Step(float deltaTime, int collisionSteps) {
        if (!world_)
            throw std::logic_error("RayJoltContext::Init() must succeed before Step()");
        const auto result = runtime_.Update(world_->GetPhysicsSystem(), deltaTime, collisionSteps);
        world_->UpdateCharacterContacts();
        return result;
    }

    PhysicsWorld& RayJoltContext::GetWorld() {
        if (!world_)
            throw std::logic_error("RayJoltContext::Init() must succeed before GetWorld()");
        return *world_;
    }

    const char* RJ_Version()
    {
        return 
            TextFormat("version %d.%d.%d",
            RAYJOLT_VERSION_MAJOR,
            RAYJOLT_VERSION_MINOR,
            RAYJOLT_VERSION_PATCH);
    }

    bool RJ_Initialize() {
        std::cout << "[INFO]Starting Rayjolt (" << RJ_Version() << ")..." << std::endl;

        return g_joltRuntime.Init();
    }

    void RJ_RegisterDefaultAllocator() {
        JPH::RegisterDefaultAllocator();
    }

    void RJ_End() {
        g_joltRuntime.End();
    }

    JPH::EPhysicsUpdateError RJ_Update(PhysicsWorld& physics, float timeStep, int collisionSteps) {
        const auto result = g_joltRuntime.Update(physics.GetPhysicsSystem(), timeStep, collisionSteps);
        physics.UpdateCharacterContacts();
        return result;
    }

    void RJ_OptimizeBroadPhase(PhysicsWorld& physics) {
        physics.OptimizeBroadPhase();
    }

}
