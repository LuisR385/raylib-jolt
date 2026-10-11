#include "rj.hpp"
#include <iostream>
//NOTE : ONLY ONE JOLT RUNTIME.
static RayJolt::JoltRuntime g_joltRuntime;

namespace RayJolt {

    bool JoltRuntime::Init() {
        // 同じオブジェクトでの二重初期化を防ぐ

        if (!isInitialized) {
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

    JPH::EPhysicsUpdateError JoltRuntime::Update(JPH::PhysicsSystem& physics, float timeStep, int collisionSteps)
    {
        JPH_ASSERT(isInitialized);
        return physics.Update(timeStep, collisionSteps, tempAllocator_.get(), jobSystem_.get());
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

    void RJ_End() {
        g_joltRuntime.End();
    }

    JPH::EPhysicsUpdateError RJ_Update(JPH::PhysicsSystem& physics, float timeStep, int collisionSteps) {
        return g_joltRuntime.Update(physics, timeStep, collisionSteps);
    }

}
