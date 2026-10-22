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
    RJ::RayJoltContext context;
    if (!context.Init())
        return 1;

    // Destroy all physics objects before unregistering Jolt's types and factory.
    {
        RJ::PhysicsWorld& physics = context.GetWorld();

        RJ::BoxDesc floorSettings;
        floorSettings.size = {20.0f, 1.0f, 20.0f};
        floorSettings.position = {0.0f, -0.5f, 0.0f};
        floorSettings.isStatic = true;
        floorSettings.restitution = 0.8f;
        RJ::BodyHandle floor = physics.CreateBox(floorSettings);
        if (!floor.IsValid())
            return 1;

        constexpr float radius = 0.5f;
        const Vector3 startPosition{0.0f, 6.0f, 0.0f};
        RJ::SphereDesc ballSettings;
        ballSettings.radius = radius;
        ballSettings.position = startPosition;
        ballSettings.restitution = 0.8f;
        RJ::BodyHandle ball = physics.CreateSphere(ballSettings);
        if (!ball.IsValid())
            return 1;
        RJ::CapsuleDesc capsuleSettings;
        capsuleSettings.position = {3.0f, 6.0f, 0.0f};
        RJ::BodyHandle capsule = physics.CreateCapsule(capsuleSettings);
        if (!capsule.IsValid())
            return 1;
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
                physics.SetPosition(ball, startPosition);
                physics.SetRotation(ball, QuaternionIdentity());
                physics.SetLinearVelocity(ball, {0.0f, 0.0f, 0.0f});
                physics.SetAngularVelocity(ball, {0.0f, 0.0f, 0.0f});
                physics.SetPosition(capsule, capsuleSettings.position);
                physics.SetRotation(capsule, capsuleSettings.rotation);
                physics.SetLinearVelocity(capsule, {0.0f, 0.0f, 0.0f});
                physics.SetAngularVelocity(capsule, {0.0f, 0.0f, 0.0f});
                accumulator = 0.0f;
            }
            if (IsKeyPressed(KEY_SPACE))
                physics.AddImpulse(ball, {0.0f, 3000.0f, 0.0f});
            accumulator += std::min(GetFrameTime(), 0.25f);
            while (accumulator >= RJ::timeStep)
            {
                if (IsKeyDown(KEY_F))
                    physics.AddForce(ball, {0.0f, 6000.0f, 0.0f});
                context.Step(RJ::timeStep);
                accumulator -= RJ::timeStep;
            }
            const Vector3 spherePosition = physics.GetPosition(ball);
            const Vector3 capsulePosition = physics.GetPosition(capsule);
            const float halfCylinderHeight = capsuleSettings.height * 0.5f - capsuleSettings.radius;
            const Vector3 capsuleOffset = Vector3RotateByQuaternion(
                {0.0f, halfCylinderHeight, 0.0f}, physics.GetRotation(capsule));
            const Vector3 capsuleTop = Vector3Add(capsulePosition, capsuleOffset);
            const Vector3 capsuleBottom = Vector3Subtract(capsulePosition, capsuleOffset);
            RJ::RayHit mouseHit;
            const bool hitBody = physics.CastRay(GetScreenToWorldRay(GetMousePosition(), camera), 100.0f, mouseHit);
            if (hitBody && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                physics.AddImpulse(mouseHit.body, {0.0f, 3000.0f, 0.0f});

            BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
            DrawCube({0.0f, -0.5f, 0.0f}, 20.0f, 1.0f, 20.0f, LIGHTGRAY);
            DrawCubeWires({0.0f, -0.5f, 0.0f}, 20.0f, 1.0f, 20.0f, GRAY);
            DrawGrid(20, 1.0f);
            DrawSphere(spherePosition, radius, ORANGE);
            DrawSphereWires(spherePosition, radius, 16, 16, MAROON);
            DrawCapsule(capsuleTop, capsuleBottom, capsuleSettings.radius, 8, 16, SKYBLUE);
            DrawCapsuleWires(capsuleTop, capsuleBottom, capsuleSettings.radius, 8, 16, BLUE);
            if (hitBody) {
                DrawSphere(mouseHit.point, 0.08f, RED);
                DrawLine3D(mouseHit.point, Vector3Add(mouseHit.point, mouseHit.normal), RED);
            }
            EndMode3D();
            DrawText("rayjolt | raylib + Jolt Physics", 20, 20, 24, DARKGRAY);
            DrawText("R: reset   SPACE: impulse   F: hold force   ESC: exit", 20, 54, 20, DARKGRAY);
            DrawFPS(20, 84);
            DrawText("Left click: impulse at pointed body", 20, 112, 20, DARKGRAY);
            EndDrawing();
        }
        CloseWindow();
        physics.DestroyBody(ball);
        physics.DestroyBody(capsule);
        physics.DestroyBody(floor);
    }
    context.End();
    return 0;
}
