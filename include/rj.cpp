#include "rj.hpp"

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

            jobSystem_ =
                std::make_unique<JPH::JobSystemThreadPool>(
                    JPH::cMaxPhysicsJobs,
                    JPH::cMaxPhysicsBarriers,
                    std::thread::hardware_concurrency() - 1
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
            TraceLog(LOG_INFO, "rayjolt Process finished successfully.");
        }
        
        TraceLog(LOG_INFO, "rayjolt Process failed successfully!");
        
    
    }

}
