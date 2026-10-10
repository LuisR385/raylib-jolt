// Include Jolt.h before all other Jolt headers.
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterTable.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h>
#include <Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h>
#include <raylib.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <thread>

static void JoltTrace(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);
    std::fputc('\n', stderr);
}

#ifdef JPH_ENABLE_ASSERTS
static bool JoltAssert(const char *expression, const char *message,
                       const char *file, JPH::uint line)
{
    std::fprintf(stderr, "%s:%u: %s (%s)\n", file, line, expression,
                 message ? message : "");
    return true;
}
#endif

int main()
{
    JPH::RegisterDefaultAllocator();
    JPH::Trace = JoltTrace;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = JoltAssert;)
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    // Destroy all physics objects before unregistering Jolt's types and factory.
    {
        constexpr JPH::ObjectLayer staticLayer = 0;
        constexpr JPH::ObjectLayer dynamicLayer = 1;
        JPH::ObjectLayerPairFilterTable layerPairs(2);
        layerPairs.EnableCollision(staticLayer, dynamicLayer);
        layerPairs.EnableCollision(dynamicLayer, dynamicLayer);
        JPH::BroadPhaseLayerInterfaceTable broadPhaseLayers(2, 2);
        broadPhaseLayers.MapObjectToBroadPhaseLayer(staticLayer, JPH::BroadPhaseLayer(0));
        broadPhaseLayers.MapObjectToBroadPhaseLayer(dynamicLayer, JPH::BroadPhaseLayer(1));
        JPH::ObjectVsBroadPhaseLayerFilterTable broadPhaseFilter(
            broadPhaseLayers, 2, layerPairs, 2);

        JPH::TempAllocatorImpl allocator(10 * 1024 * 1024);
        const unsigned cores = std::thread::hardware_concurrency();
        JPH::JobSystemThreadPool jobs(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                                     static_cast<int>(cores > 1 ? cores - 1 : 1));
        JPH::PhysicsSystem physics;
        physics.Init(1024, 0, 1024, 1024, broadPhaseLayers, broadPhaseFilter, layerPairs);
        JPH::BodyInterface &bodies = physics.GetBodyInterface();

        // BoxShape takes half extents. The top of this floor is y = 0.
        JPH::BodyCreationSettings floorSettings(
            new JPH::BoxShape(JPH::Vec3(10.0f, 0.5f, 10.0f)),
            JPH::RVec3(0.0f, -0.5f, 0.0f), JPH::Quat::sIdentity(),
            JPH::EMotionType::Static, staticLayer);
        floorSettings.mRestitution = 0.8f;
        const JPH::BodyID floor = bodies.CreateAndAddBody(
            floorSettings, JPH::EActivation::DontActivate);

        constexpr float radius = 0.5f;
        const JPH::RVec3 startPosition(0.0f, 6.0f, 0.0f);
        JPH::BodyCreationSettings ballSettings(
            new JPH::SphereShape(radius), startPosition, JPH::Quat::sIdentity(),
            JPH::EMotionType::Dynamic, dynamicLayer);
        ballSettings.mRestitution = 0.8f;
        const JPH::BodyID ball = bodies.CreateAndAddBody(
            ballSettings, JPH::EActivation::Activate);
        physics.OptimizeBroadPhase();

        SetConfigFlags(FLAG_MSAA_4X_HINT);
        InitWindow(1280, 720, "rayjolt - raylib + Jolt Hello World");
        SetTargetFPS(60);
        Camera3D camera{};
        camera.position = {12.0f, 9.0f, 12.0f};
        camera.target = {0.0f, 2.0f, 0.0f};
        camera.up = {0.0f, 1.0f, 0.0f};
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        constexpr float timeStep = 1.0f / 60.0f;
        float accumulator = 0.0f;
        while (!WindowShouldClose())
        {
            if (IsKeyPressed(KEY_R))
            {
                bodies.SetPositionAndRotation(ball, startPosition,
                    JPH::Quat::sIdentity(), JPH::EActivation::Activate);
                bodies.SetLinearAndAngularVelocity(ball, JPH::Vec3::sZero(), JPH::Vec3::sZero());
                accumulator = 0.0f;
            }
            accumulator += std::min(GetFrameTime(), 0.25f);
            while (accumulator >= timeStep)
            {
                physics.Update(timeStep, 1, &allocator, &jobs);
                accumulator -= timeStep;
            }
            const JPH::RVec3 position = bodies.GetPosition(ball);
            const Vector3 spherePosition{static_cast<float>(position.GetX()),
                static_cast<float>(position.GetY()), static_cast<float>(position.GetZ())};

            BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
            DrawCube({0.0f, -0.5f, 0.0f}, 20.0f, 1.0f, 20.0f, LIGHTGRAY);
            DrawCubeWires({0.0f, -0.5f, 0.0f}, 20.0f, 1.0f, 20.0f, GRAY);
            DrawGrid(20, 1.0f);
            DrawSphere(spherePosition, radius, ORANGE);
            DrawSphereWires(spherePosition, radius, 16, 16, MAROON);
            EndMode3D();
            DrawText("rayjolt | raylib + Jolt Physics", 20, 20, 24, DARKGRAY);
            DrawText("R: reset ball   ESC: exit", 20, 54, 20, DARKGRAY);
            DrawFPS(20, 84);
            EndDrawing();
        }
        CloseWindow();
        bodies.RemoveBody(ball);
        bodies.DestroyBody(ball);
        bodies.RemoveBody(floor);
        bodies.DestroyBody(floor);
    }
    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
    return 0;
}
