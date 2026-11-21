#include "rj.hpp"
#ifdef RJ_WITH_RFBX
#include "rfbx.h"
#endif

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
#include <cmath>


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

int main(int argc, char** argv)
{

    JPH::Trace = JoltTrace;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = JoltAssert;)
    RJ::RayJoltContext context;
    if (!context.Init())
        return 1;

    // Destroy all physics objects before unregistering Jolt's types and factory.
    {
        RJ::PhysicsWorld& physics = context.GetWorld();
        physics.SetGravity({0.0f, -9.81f, 0.0f});

        RJ::TriangleMeshData floorMesh;
        floorMesh.vertices = {{-1, 0, -1}, {1, 0, -1}, {1, 0, 1}, {-1, 0, 1}};
        floorMesh.indices = {0, 2, 1, 0, 3, 2};
        RJ::StaticMeshDesc floorSettings;
        floorSettings.scale = {-10.0f, 1.0f, 10.0f};
        RJ::BodyHandle floor = physics.CreateStaticMesh(floorMesh, floorSettings);
        if (!floor.IsValid())
            return 1;

        RJ::StaticMeshDesc rampSettings;
        rampSettings.position = {-6.0f, 3.0f * sinf(25.0f * DEG2RAD), 1.5f};
        rampSettings.rotation = QuaternionFromAxisAngle({1.0f, 0.0f, 0.0f}, 25.0f * DEG2RAD);
        rampSettings.scale = {2.0f, 1.0f, 3.0f};
        const auto ramp = physics.CreateStaticMesh(floorMesh, rampSettings);
        if (!ramp.IsValid())
            return 1;

        RJ::StaticMeshDesc wallSettings;
        wallSettings.position = {4.0f, 1.5f, -3.0f};
        wallSettings.rotation = QuaternionFromAxisAngle({1.0f, 0.0f, 0.0f}, 90.0f * DEG2RAD);
        wallSettings.scale = {2.0f, 1.0f, 1.5f};
        const auto wall = physics.CreateStaticMesh(floorMesh, wallSettings);
        if (!wall.IsValid())
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
        const float initialBallMass = physics.GetMass(ball);
        RJ::CapsuleDesc capsuleSettings;
        capsuleSettings.position = {3.0f, 6.0f, 0.0f};
        RJ::BodyHandle capsule = physics.CreateCapsule(capsuleSettings);
        if (!capsule.IsValid())
            return 1;
        RJ::CharacterDesc characterSettings;
        characterSettings.maxSlopeAngleDegrees = 45.0f;
        characterSettings.position = {-3.0f, 3.0f, 0.0f};
        RJ::BodyHandle player = physics.CreateCharacter(characterSettings);
        if (!player.IsValid())
            return 1;
        RJ::BoxDesc platformSettings;
        platformSettings.size = {3.0f, 0.4f, 3.0f};
        platformSettings.position = {0.0f, 1.0f, -5.0f};
        platformSettings.motionType = RJ::MotionType::Kinematic;
        RJ::BodyHandle platform = physics.CreateBox(platformSettings);
        if (!platform.IsValid())
            return 1;

        std::vector<RJ::BoxDesc> obstacles;
        for (int step = 0; step < 4; ++step) {
            RJ::BoxDesc desc;
            const float height = 0.2f * (step + 1);
            desc.position = {6.0f, height * 0.5f, 2.0f + step * 1.2f};
            desc.size = {2.0f, height, 1.2f};
            desc.motionType = RJ::MotionType::Static;
            obstacles.push_back(desc);
        }
        RJ::BoxDesc ceiling;
        ceiling.position = {-3.0f, 1.45f, 5.0f};
        ceiling.size = {3.0f, 0.2f, 4.0f};
        ceiling.motionType = RJ::MotionType::Static;
        obstacles.push_back(ceiling);
        for (const auto& desc : obstacles)
            if (!physics.CreateBox(desc).IsValid())
                return 1;

        RJ::BoxDesc sensorSettings;
        sensorSettings.position = {0.0f, 1.0f, 3.0f};
        sensorSettings.size = {3.0f, 2.0f, 3.0f};
        sensorSettings.motionType = RJ::MotionType::Static;
        sensorSettings.isSensor = true;
        const auto sensor = physics.CreateBox(sensorSettings);
        if (!sensor.IsValid())
            return 1;

        const auto resetPlayer = [&](Vector3 position) {
            physics.SetPosition(player, {0.0f, 20.0f, 0.0f});
            physics.SetCrouching(player, false);
            physics.SetPosition(player, position);
            physics.SetRotation(player, QuaternionIdentity());
            physics.SetLinearVelocity(player, {0.0f, 0.0f, 0.0f});
            physics.SetAngularVelocity(player, {0.0f, 0.0f, 0.0f});
        };
        if (argc > 1 && TextIsEqual(argv[1], "--smoke-test")) {
            const auto check = [](bool condition, const char* message) {
                if (!condition)
                    TraceLog(LOG_ERROR, "Stage check failed: %s", message);
                return condition;
            };
            for (int step = 0; step < 240; ++step)
                if (!check(context.Step(RJ::timeStep) == JPH::EPhysicsUpdateError::None, "physics update"))
                    return 1;
            if (!check(physics.IsGrounded(player), "flat ground"))
                return 1;
            physics.SetCrouching(player, true);
            physics.SetPosition(player, {-3.0f, 0.62f, 5.0f});
            physics.SetLinearVelocity(player, {});
            for (int step = 0; step < 60; ++step)
                context.Step(RJ::timeStep);
            if (!check(physics.IsGrounded(player), "crouching ground") ||
                !check(!physics.SetCrouching(player, false), "ceiling blocks standing"))
                return 1;
            resetPlayer(characterSettings.position);
            for (int step = 0; step < 240; ++step)
                context.Step(RJ::timeStep);
            if (!check(!physics.IsCrouching(player) && physics.IsGrounded(player), "player reset"))
                return 1;
            RJ::RayHit hit;
            for (int step = 0; step < 4; ++step) {
                const float z = 2.0f + step * 1.2f;
                if (!check(physics.CastRay({{6, 3, z}, {0, -1, 0}}, 4, hit) &&
                    std::abs(hit.point.y - 0.2f * (step + 1)) < 0.01f, "stair height"))
                    return 1;
            }
            const Vector3 rampNormal = Vector3RotateByQuaternion({0, 1, 0}, rampSettings.rotation);
            if (!check(physics.CastRay({Vector3Add(rampSettings.position, Vector3Scale(rampNormal, 2)),
                Vector3Negate(rampNormal)}, 4, hit) && hit.body.GetJoltID() == ramp.GetJoltID(), "ramp ray") ||
                !check(physics.CastRay({{4, 1, -1}, {0, 0, -1}}, 4, hit) &&
                    hit.body.GetJoltID() == wall.GetJoltID(), "wall ray"))
                return 1;
            resetPlayer({platformSettings.position.x, platformSettings.position.y + 1.12f, platformSettings.position.z});
            for (int step = 0; step < 60; ++step)
                context.Step(RJ::timeStep);
            for (int step = 0; step < 120; ++step) {
                Vector3 target = platformSettings.position;
                target.x += sinf((step + 1) * RJ::timeStep) * 3.0f;
                physics.MoveKinematic(platform, target, QuaternionIdentity(), RJ::timeStep);
                physics.MoveCharacter(player, {});
                context.Step(RJ::timeStep);
            }
            if (!check(physics.GetCharacterGroundInfo(player).body.GetJoltID() == platform.GetJoltID() &&
                std::abs(physics.GetPosition(player).x - physics.GetPosition(platform).x) < 0.15f, "platform follow"))
                return 1;
            TraceLog(LOG_INFO, "Stage smoke test passed");
            return 0;
        }
        SetConfigFlags(FLAG_MSAA_4X_HINT);
        InitWindow(1280, 720, "rayjolt - physics test stage");
#ifdef RJ_WITH_RFBX
        RfbxModel importedModel{};
        RJ::BodyHandle importedBody;
        const bool importedDynamic = argc > 3 && TextIsEqual(argv[3], "--dynamic");
        const Matrix importedTransform = MatrixTranslate(0.0f, importedDynamic ? 3.0f : 0.0f, 5.0f);
        if (argc > 1) {
            importedModel = RfbxLoadModel(argv[1]);
            if (importedModel.valid) {
                RfbxModel collisionModel{};
                const RfbxModel* collisionSource = &importedModel;
                if (argc > 2) {
                    collisionModel = RfbxLoadModel(argv[2]);
                    collisionSource = &collisionModel;
                }
                RJ::TriangleMeshData collisionMesh;
                const auto result = RJ::BuildRfbxCollisionMesh(*collisionSource, collisionMesh,
                    importedDynamic ? MatrixIdentity() : importedTransform);
                if (result.valid) {
                    if (importedDynamic) {
                        RJ::ConvexHullDesc desc;
                        desc.position = {0.0f, 3.0f, 5.0f};
                        desc.mass = 10.0f;
                        importedBody = physics.CreateConvexHull(collisionMesh.vertices, desc);
                    } else {
                        importedBody = physics.CreateStaticMesh(collisionMesh);
                    }
                    if (!importedBody.IsValid())
                        TraceLog(LOG_WARNING, "Could not create FBX collision body");
                } else {
                    TraceLog(LOG_WARNING, "FBX collision mesh: %s", result.message.c_str());
                }
                RfbxUnloadModel(&collisionModel);
            } else {
                TraceLog(LOG_WARNING, "Could not load FBX render model: %s", argv[1]);
            }
        }
#endif
        RJ::RJ_OptimizeBroadPhase(physics);
        Model platformModel = LoadModelFromMesh(GenMeshCube(
            platformSettings.size.x, platformSettings.size.y, platformSettings.size.z));
        Model surfaceModel = LoadModelFromMesh(GenMeshPlane(2.0f, 2.0f, 1, 1));
        const auto drawSurface = [&](RJ::BodyHandle body, const RJ::StaticMeshDesc& desc, Color color) {
            surfaceModel.transform = MatrixMultiply(
                MatrixScale(desc.scale.x, desc.scale.y, desc.scale.z), physics.GetTransform(body));
            DrawModel(surfaceModel, {0.0f, 0.0f, 0.0f}, 1.0f, color);
        };
        SetTargetFPS(60);
        Camera3D camera{};
        camera.position = {14.0f, 14.0f, 14.0f};
        camera.target = {0.0f, 0.5f, 0.0f};
        camera.up = {0.0f, 1.0f, 0.0f};
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        float accumulator = 0.0f;
        float platformTime = 0.0f;
        bool jumpRequested = false;
        bool showDebugControls = false;
        bool followCamera = false;
        int contactBegins = 0;
        int contactStays = 0;
        int contactEnds = 0;
        int sensorEvents = 0;
        const char* currentArea = "Flat ground";
        while (!WindowShouldClose())
        {
            if (IsKeyPressed(KEY_F1))
                showDebugControls = !showDebugControls;
            if (IsKeyPressed(KEY_TAB))
                followCamera = !followCamera;
            const Vector3 checkpoints[] = {
                characterSettings.position, {-6.0f, 1.0f, 4.2f}, {6.0f, 1.0f, 0.5f},
                {-3.0f, 1.0f, 2.2f}, {4.0f, 1.0f, -1.0f}};
            const char* areaNames[] = {"Flat ground", "25 degree ramp", "Stairs (step assist not implemented)",
                "Low ceiling (hold C)", "Wall"};
            for (int i = 0; i < 5; ++i) {
                if (IsKeyPressed(KEY_ONE + i)) {
                    resetPlayer(checkpoints[i]);
                    currentArea = areaNames[i];
                    jumpRequested = false;
                }
            }
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
                resetPlayer(characterSettings.position);
                currentArea = "Flat ground";
                physics.SetGravity({0.0f, -9.81f, 0.0f});
                physics.SetGravityFactor(ball, 1.0f);
                physics.SetRestitution(ball, ballSettings.restitution);
                physics.SetFriction(ball, ballSettings.friction);
                physics.SetMass(ball, initialBallMass);
                physics.SetLinearDamping(ball, 0.05f);
                physics.SetAngularDamping(ball, 0.05f);
                physics.SetContinuousCollision(ball, false);
                accumulator = 0.0f;
                jumpRequested = false;
                platformTime = 0.0f;
                physics.SetPosition(platform, platformSettings.position);
                physics.SetLinearVelocity(platform, {0.0f, 0.0f, 0.0f});
#ifdef RJ_WITH_RFBX
                if (importedDynamic && importedBody.IsValid()) {
                    physics.SetPosition(importedBody, {0.0f, 3.0f, 5.0f});
                    physics.SetRotation(importedBody, QuaternionIdentity());
                    physics.SetLinearVelocity(importedBody, {});
                    physics.SetAngularVelocity(importedBody, {});
                }
#endif
            }
            if (IsKeyPressed(KEY_SPACE))
                physics.AddImpulse(ball, {0.0f, 3000.0f, 0.0f});
            if (IsKeyPressed(KEY_G)) {
                const bool gravityEnabled = Vector3LengthSqr(physics.GetGravity()) > 0.0f;
                physics.SetGravity(gravityEnabled ? Vector3{0.0f, 0.0f, 0.0f} : Vector3{0.0f, -9.81f, 0.0f});
            }
            if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_SIX)) {
                Vector3 position = physics.GetPosition(platform);
                position.y += platformSettings.size.y * 0.5f + characterSettings.height * 0.5f + 0.02f;
                resetPlayer(position);
                currentArea = "Moving platform";
                jumpRequested = false;
            }
            if (IsKeyPressed(KEY_D)) {
                const float damping = physics.GetLinearDamping(ball) < 1.0f ? 2.0f : 0.0f;
                physics.SetLinearDamping(ball, damping);
                physics.SetAngularDamping(ball, damping);
            }
            if (IsKeyPressed(KEY_J))
                jumpRequested = true;
            if (IsKeyPressed(KEY_H))
                physics.SetGravityFactor(ball, physics.GetGravityFactor(ball) == 0.0f ? 1.0f : 0.0f);
            if (IsKeyPressed(KEY_B))
                physics.SetRestitution(ball, physics.GetRestitution(ball) == 0.0f ? 0.8f : 0.0f);
            if (IsKeyPressed(KEY_M))
                physics.SetMass(ball, physics.GetMass(ball) < initialBallMass * 2.0f ? initialBallMass * 5.0f : initialBallMass);
            if (IsKeyPressed(KEY_I))
                physics.AddAngularImpulse(capsule, {0.0f, 0.0f, 200.0f});
            if (IsKeyPressed(KEY_N)) {
                if (physics.IsActive(ball))
                    physics.SleepBody(ball);
                else
                    physics.WakeBody(ball);
            }
            if (IsKeyPressed(KEY_V))
                physics.SetContinuousCollision(ball, !physics.IsContinuousCollisionEnabled(ball));
            if (IsKeyPressed(KEY_L)) {
                physics.SetPosition(ball, {0.0f, 10.0f, 0.0f});
                physics.SetLinearDamping(ball, 0.0f);
                physics.SetLinearVelocity(ball, {0.0f, -400.0f, 0.0f});
                physics.SetAngularVelocity(ball, {0.0f, 0.0f, 0.0f});
            }
            if (physics.GetPosition(player).y < -10.0f) {
                resetPlayer(characterSettings.position);
                currentArea = "Flat ground (recovered from fall)";
                jumpRequested = false;
            }
            accumulator += std::min(GetFrameTime(), 0.25f);
            while (accumulator >= RJ::timeStep)
            {
                platformTime += RJ::timeStep;
                Vector3 platformTarget = platformSettings.position;
                platformTarget.x += sinf(platformTime) * 3.0f;
                physics.MoveKinematic(platform, platformTarget, QuaternionIdentity(), RJ::timeStep);
                physics.SetCrouching(player, IsKeyDown(KEY_C));
                Vector2 movement{
                    float(IsKeyDown(KEY_RIGHT)) - float(IsKeyDown(KEY_LEFT)),
                    float(IsKeyDown(KEY_DOWN)) - float(IsKeyDown(KEY_UP))};
                physics.MoveCharacter(player, movement);
                if (jumpRequested) {
                    physics.Jump(player);
                    jumpRequested = false;
                }
                if (IsKeyDown(KEY_F))
                    physics.AddForce(ball, {0.0f, 6000.0f, 0.0f});
                if (IsKeyDown(KEY_T))
                    physics.AddTorque(capsule, {0.0f, 0.0f, 400.0f});
                context.Step(RJ::timeStep);
                contactBegins = contactStays = contactEnds = 0;
                sensorEvents = 0;
                for (const auto& event : physics.GetContactEvents()) {
                    if (event.bodyA.GetJoltID() == sensor.GetJoltID() || event.bodyB.GetJoltID() == sensor.GetJoltID())
                        ++sensorEvents;
                    switch (event.type) {
                    case RJ::ContactEventType::Begin: ++contactBegins; break;
                    case RJ::ContactEventType::Stay: ++contactStays; break;
                    case RJ::ContactEventType::End: ++contactEnds; break;
                    }
                }
                accumulator -= RJ::timeStep;
            }
            const Vector3 spherePosition = physics.GetPosition(ball);
            const Vector3 capsulePosition = physics.GetPosition(capsule);
            const float halfCylinderHeight = capsuleSettings.height * 0.5f - capsuleSettings.radius;
            const Vector3 capsuleOffset = Vector3RotateByQuaternion(
                {0.0f, halfCylinderHeight, 0.0f}, physics.GetRotation(capsule));
            const Vector3 capsuleTop = Vector3Add(capsulePosition, capsuleOffset);
            const Vector3 capsuleBottom = Vector3Subtract(capsulePosition, capsuleOffset);
            const auto groundInfo = physics.GetCharacterGroundInfo(player);
            const char* groundText = "In air";
            switch (groundInfo.state) {
            case RJ::GroundState::OnGround: groundText = "On ground"; break;
            case RJ::GroundState::OnSteepGround: groundText = "On steep ground"; break;
            case RJ::GroundState::NotSupported: groundText = "Touching, not supported"; break;
            case RJ::GroundState::InAir: break;
            }
            const Vector3 playerPosition = physics.GetPosition(player);
            camera.target = followCamera ? playerPosition : Vector3{0.0f, 0.5f, 0.0f};
            camera.position = followCamera ? Vector3Add(playerPosition, {6.0f, 4.0f, 6.0f}) : Vector3{14.0f, 14.0f, 14.0f};
            RJ::RayHit mouseHit;
            const bool hitBody = physics.CastRay(GetScreenToWorldRay(GetMousePosition(), camera), 100.0f, mouseHit);
            if (hitBody && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                physics.AddImpulse(mouseHit.body, {0.0f, 3000.0f, 0.0f}, mouseHit.point);
            const Ray footRay{playerPosition, {0.0f, -1.0f, 0.0f}};
            RJ::RayHit footHit;
            const bool hitBelowPlayer = physics.CastRay(footRay, 5.0f, footHit, player);

            BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
            DrawPlane({0.0f, 0.0f, 0.0f}, {20.0f, 20.0f}, LIGHTGRAY);
            drawSurface(ramp, rampSettings, ORANGE);
            drawSurface(wall, wallSettings, GRAY);
            DrawGrid(20, 1.0f);
            for (std::size_t i = 0; i < obstacles.size(); ++i) {
                const auto& obstacle = obstacles[i];
                if (i < 4)
                    DrawCubeV(obstacle.position, obstacle.size, i % 2 == 0 ? SKYBLUE : BLUE);
                DrawCubeWiresV(obstacle.position, obstacle.size, i < 4 ? DARKBLUE : PURPLE);
            }
#ifdef RJ_WITH_RFBX
            if (importedModel.valid)
                RfbxDrawModelMatrix(&importedModel,
                    importedDynamic && importedBody.IsValid() ? physics.GetTransform(importedBody) : importedTransform, WHITE);
#endif
            platformModel.transform = physics.GetTransform(platform);
            DrawModel(platformModel, {0.0f, 0.0f, 0.0f}, 1.0f, BEIGE);
            DrawModelWires(platformModel, {0.0f, 0.0f, 0.0f}, 1.0f, BROWN);
            DrawSphere(spherePosition, radius, ORANGE);
            DrawSphereWires(spherePosition, radius, 16, 16, MAROON);
            DrawCapsule(capsuleTop, capsuleBottom, capsuleSettings.radius, 8, 16, SKYBLUE);
            DrawCapsuleWires(capsuleTop, capsuleBottom, capsuleSettings.radius, 8, 16, BLUE);
            const float playerHalfCylinder = physics.GetCharacterHeight(player) * 0.5f - characterSettings.radius;
            DrawCapsule(Vector3Add(playerPosition, {0, playerHalfCylinder, 0}),
                Vector3Subtract(playerPosition, {0, playerHalfCylinder, 0}),
                characterSettings.radius, 8, 16, groundInfo.state == RJ::GroundState::OnGround ? GREEN : DARKGREEN);
            if (groundInfo.body.IsValid())
                DrawLine3D(groundInfo.position, Vector3Add(groundInfo.position, groundInfo.normal), PURPLE);
            if (hitBelowPlayer) {
                DrawLine3D(playerPosition, footHit.point, GOLD);
                DrawSphere(footHit.point, 0.06f, GOLD);
            }
            if (hitBody) {
                DrawBoundingBox(physics.GetBoundingBox(mouseHit.body), RED);
                DrawSphere(mouseHit.point, 0.08f, RED);
                DrawLine3D(mouseHit.point, Vector3Add(mouseHit.point, mouseHit.normal), RED);
            }
            DrawCubeWiresV(sensorSettings.position, sensorSettings.size, ORANGE);
            EndMode3D();
            DrawRectangle(12, 12, 750, 196, Fade(RAYWHITE, 0.9f));
            DrawText("rayjolt | physics test stage", 20, 20, 24, DARKGRAY);
            DrawText("Arrows: move   J: jump   Hold C: crouch   Tab: camera", 20, 54, 20, DARKGRAY);
            DrawText("1: flat   2: ramp   3: stairs   4: low ceiling   5: wall   6: platform", 20, 82, 18, DARKGRAY);
            DrawText(TextFormat("Area: %s | %s", currentArea, groundText), 20, 108, 18, DARKGRAY);
            DrawText(TextFormat("Position: %.2f, %.2f, %.2f | %s", playerPosition.x, playerPosition.y, playerPosition.z,
                physics.IsCrouching(player) ? "Crouching" : "Standing"), 20, 132, 18, DARKGRAY);
            DrawText("R: full reset   F1: physics controls   Esc: exit", 20, 156, 18, DARKGRAY);
            DrawText(TextFormat("Contacts: Begin %d   Stay %d   End %d | Orange sensor: %d",
                contactBegins, contactStays, contactEnds, sensorEvents), 20, 180, 18, DARKGRAY);
            DrawFPS(GetScreenWidth() - 100, 20);
            if (showDebugControls) {
                DrawRectangle(12, 214, 750, 300, Fade(RAYWHITE, 0.9f));
                DrawText("SPACE: ball impulse   Hold F: ball force", 20, 226, 18, DARKGRAY);
                DrawText("Left click: impulse at hit point", 20, 250, 18, DARKGRAY);
                DrawText(TextFormat("Character: %s   Max slope: %.0f degrees",
                    groundText, characterSettings.maxSlopeAngleDegrees), 20, 274, 18, DARKGRAY);
                DrawText(TextFormat("G: toggle gravity   Y: %.2f m/s^2", physics.GetGravity().y), 20, 298, 18, DARKGRAY);
                DrawText(TextFormat("H: toggle ball gravity   Factor: %.1f", physics.GetGravityFactor(ball)), 20, 322, 18, DARKGRAY);
                DrawText(TextFormat("B: toggle ball bounce   Restitution: %.1f", physics.GetRestitution(ball)), 20, 346, 18, DARKGRAY);
                DrawText(TextFormat("M: toggle ball mass   Mass: %.1f kg", physics.GetMass(ball)), 20, 370, 18, DARKGRAY);
                DrawText(TextFormat("D: toggle ball damping   Linear: %.1f   Angular: %.1f",
                    physics.GetLinearDamping(ball), physics.GetAngularDamping(ball)), 20, 394, 18, DARKGRAY);
                DrawText("Capsule: hold T for torque   I for angular impulse", 20, 418, 18, DARKGRAY);
                DrawText(TextFormat("N: sleep/wake ball   State: %s",
                    physics.IsActive(ball) ? "Active" : "Sleeping"), 20, 442, 18, DARKGRAY);
                DrawText(TextFormat("V: toggle CCD (%s)   L: fast drop",
                    physics.IsContinuousCollisionEnabled(ball) ? "On" : "Off"), 20, 466, 18, DARKGRAY);
            }
            EndDrawing();
        }
        UnloadModel(surfaceModel);
        UnloadModel(platformModel);
#ifdef RJ_WITH_RFBX
        RfbxUnloadModel(&importedModel);
#endif
        CloseWindow();
        physics.DestroyBody(ball);
        physics.DestroyBody(capsule);
        physics.DestroyBody(player);
        physics.DestroyBody(platform);
        physics.DestroyBody(floor);
    }
    context.End();
    return 0;
}
