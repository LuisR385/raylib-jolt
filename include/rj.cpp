#include "rj.hpp"
#include <iostream>
#include <stdexcept>
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
        broadPhaseFilter_.reset();
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
        return runtime_.Update(world_->GetPhysicsSystem(), deltaTime, collisionSteps);
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
        return g_joltRuntime.Update(physics.GetPhysicsSystem(), timeStep, collisionSteps);
    }

    void RJ_OptimizeBroadPhase(PhysicsWorld& physics) {
        physics.OptimizeBroadPhase();
    }

}
