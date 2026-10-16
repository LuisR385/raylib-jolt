#include "rj.hpp"

// Include Jolt.h before all other Jolt headers.
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
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

    JPH::Trace = JoltTrace;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = JoltAssert;)
    RJ::RJ_RegisterDefaultAllocator();
    RJ::RayJoltContext context;
    if (!context.runtime.Init())
        return 1;

    // Destroy all physics objects before unregistering Jolt's types and factory.
    {
        RJ::PhysicsWorld& physics = context.world;
        if (!physics.Init())
        {
            return 1;
        }
        JPH::BodyInterface &bodies = physics.GetbodyInterface();
        context.initialize = true;

        RJ::BoxDesc floorSettings;
        floorSettings.size = {20.0f, 1.0f, 20.0f};
        floorSettings.position = {0.0f, -0.5f, 0.0f};
        floorSettings.isStatic = true;
        floorSettings.restitution = 0.8f;
        const RJ::BodyHandle floor = physics.CreateBox(floorSettings);
        if (!floor.IsValid())
            return 1;

        constexpr float radius = 0.5f;
        const JPH::RVec3 startPosition(0.0f, 6.0f, 0.0f);
        JPH::BodyCreationSettings ballSettings(
            new JPH::SphereShape(radius), startPosition, JPH::Quat::sIdentity(),
            JPH::EMotionType::Dynamic, RJ::ObjectLayers::MOVING);
        ballSettings.mRestitution = 0.8f;
        const JPH::BodyID ball = bodies.CreateAndAddBody(
            ballSettings, JPH::EActivation::Activate);
        RJ::RJ_OptimizeBroadPhase(physics);

        SetConfigFlags(FLAG_MSAA_4X_HINT);
        InitWindow(1280, 720, "rayjolt - raylib + Jolt Hello World");
        SetTargetFPS(60);
        Camera3D camera{};
        camera.position = {12.0f, 9.0f, 12.0f};
        camera.target = {0.0f, 2.0f, 0.0f};
        camera.up = {0.0f, 1.0f, 0.0f};
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;

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
            while (accumulator >= RJ::timeStep)
            {
                context.runtime.Update(physics.GetPhysicsSystem(), RJ::timeStep, 1);
                accumulator -= RJ::timeStep;
            }
            const Vector3 spherePosition = RJ::Detail::ToRaylibRVector3(bodies.GetPosition(ball));

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
        bodies.RemoveBody(floor.GetJoltID());
        bodies.DestroyBody(floor.GetJoltID());
    }
    return 0;
}
