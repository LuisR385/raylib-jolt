#include "rj.hpp"
#ifdef RJ_WITH_RFBX
#include "rfbx.h"
#endif

#include <cstdlib>
#include <cmath>

static void Check(bool condition)
{
    if (!condition)
        std::abort();
}

static RJ::TriangleMeshData MakeFloor()
{
    return {{{-2, 0, -2}, {2, 0, -2}, {2, 0, 2}, {-2, 0, 2}}, {0, 2, 1, 0, 3, 2}};
}

static void CheckConvexHull(RJ::RayJoltContext& context)
{
    auto& world = context.GetWorld();
    world.SetGravity({0, 0, 0});
    std::vector<Vector3> points;
    for (float x : {-0.5f, 0.5f})
        for (float y : {-0.5f, 0.5f})
            for (float z : {-0.5f, 0.5f})
                points.push_back({x, y, z});
    RJ::ConvexHullDesc desc;
    desc.position = {0, 3, 0};
    desc.mass = 10;
    desc.restitution = 0;
    auto body = world.CreateConvexHull(points, desc);
    Check(body.IsValid());
    Check(std::abs(world.GetMass(body) - 10) < 0.001f);
    world.AddImpulse(body, {20, 0, 0});
    Check(std::abs(world.GetLinearVelocity(body).x - 2) < 0.001f);
    world.SetLinearVelocity(body, {});
    Check(world.CreateStaticMesh(MakeFloor()).IsValid());
    world.SetGravity({0, -9.81f, 0});
    for (int i = 0; i < 240; ++i)
        Check(context.Step(RJ::timeStep) == JPH::EPhysicsUpdateError::None);
    Check(std::abs(world.GetPosition(body).y - 0.5f) < 0.1f);
    auto stale = body;
    Check(world.DestroyBody(body));
    Check(!world.ContainsBody(stale));
    desc.motionType = RJ::MotionType::Kinematic;
    desc.position = {9, 4, 0};
    body = world.CreateConvexHull(points, desc);
    Check(body.IsValid());
    world.MoveKinematic(body, {10, 4, 0}, QuaternionIdentity(), 1.0f);
    for (int i = 0; i < 60; ++i)
        context.Step(RJ::timeStep);
    Check(std::abs(world.GetPosition(body).x - 10) < 0.01f);
    Check(!world.CreateConvexHull({}).IsValid());
    Check(!world.CreateConvexHull({{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}}).IsValid());
    Check(!world.CreateConvexHull({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}}).IsValid());
    desc.mass = 0;
    Check(!world.CreateConvexHull(points, desc).IsValid());
    world.Shutdown();
}

static void CheckMeshScene(RJ::RayJoltContext& context)
{
    auto& world = context.GetWorld();
    RJ::StaticMeshDesc surface;
    surface.scale = {3, 1, 3};
    const auto floor = world.CreateStaticMesh(MakeFloor(), surface);
    RJ::SphereDesc sphere;
    sphere.position = {0, 3, 0};
    sphere.restitution = 0;
    auto ball = world.CreateSphere(sphere);
    RJ::CapsuleDesc capsule;
    capsule.position = {-3, 3, 0};
    capsule.restitution = 0;
    auto pill = world.CreateCapsule(capsule);
    RJ::CharacterDesc character;
    character.position = {3, 3, 0};
    auto player = world.CreateCharacter(character);
    for (int i = 0; i < 240; ++i)
        Check(context.Step(RJ::timeStep) == JPH::EPhysicsUpdateError::None);
    Check(std::abs(world.GetPosition(ball).y - sphere.radius) < 0.1f);
    Check(std::abs(world.GetPosition(pill).y - capsule.height * 0.5f) < 0.1f);
    Check(world.IsGrounded(player));
    Check(world.GetCharacterGroundInfo(player).body.GetJoltID() == floor.GetJoltID());

    world.Shutdown();
    surface.rotation = QuaternionFromAxisAngle({1, 0, 0}, 25 * DEG2RAD);
    const auto ramp = world.CreateStaticMesh(MakeFloor(), surface);
    const auto normal = Vector3RotateByQuaternion({0, 1, 0}, surface.rotation);
    RJ::RayHit hit;
    Check(world.CastRay({Vector3Scale(normal, 4), Vector3Negate(normal)}, 8, hit));
    Check(hit.body.GetJoltID() == ramp.GetJoltID());
    Check(Vector3Distance(hit.normal, normal) < 0.002f);
    world.SetGravity(Vector3Scale(normal, -9.81f));
    character.position = Vector3Scale(normal, 3);
    player = world.CreateCharacter(character);
    for (int i = 0; i < 240; ++i)
        Check(context.Step(RJ::timeStep) == JPH::EPhysicsUpdateError::None);
    Check(world.IsGrounded(player));
    Check(world.GetCharacterGroundInfo(player).body.GetJoltID() == ramp.GetJoltID());

    world.Shutdown();
    world.SetGravity({0, 0, 0});
    surface.rotation = QuaternionFromAxisAngle({1, 0, 0}, 90 * DEG2RAD);
    const auto wall = world.CreateStaticMesh(MakeFloor(), surface);
    Check(world.CastRay({{0, 0, 4}, {0, 0, -1}}, 8, hit));
    Check(hit.body.GetJoltID() == wall.GetJoltID());
    Check(std::abs(hit.distance - 4) < 0.002f && hit.normal.z > 0.99f);
    sphere.position = {0, 0, 4};
    ball = world.CreateSphere(sphere);
    world.SetLinearVelocity(ball, {0, 0, -12});
    for (int i = 0; i < 60; ++i)
        Check(context.Step(RJ::timeStep) == JPH::EPhysicsUpdateError::None);
    Check(std::abs(world.GetPosition(ball).z - sphere.radius) < 0.1f);
    Check(std::abs(world.GetLinearVelocity(ball).z) < 0.1f);
}

#ifdef RJ_WITH_RFBX
static void CheckRfbxBridge(RJ::PhysicsWorld& world)
{
    float vertices[] = {0, 0, 0, 0, 0, 2, 2, 0, 0};
    RfbxPrimitive primitive{};
    primitive.mesh.vertices = vertices;
    primitive.mesh.vertexCount = 3;
    primitive.mesh.triangleCount = 1;
    RfbxMesh mesh{};
    mesh.primitives = &primitive;
    mesh.primitiveCount = 1;
    RfbxNode nodes[3]{};
    nodes[0].visible = nodes[1].visible = true;
    nodes[0].geometryToWorld = MatrixTranslate(2, 0, 0);
    nodes[1].geometryToWorld = MatrixMultiply(MatrixScale(-1, 1, 1), MatrixTranslate(6, 0, 0));
    RfbxModel model{};
    model.valid = true;
    model.nodeCount = 3;
    model.nodes = nodes;
    model.meshCount = 1;
    model.meshes = &mesh;

    RJ::TriangleMeshData output;
    Check(RJ::BuildRfbxCollisionMesh(model, output, MatrixTranslate(0, 2, 0)).valid);
    Check(output.vertices.size() == 6);
    const auto body = world.CreateStaticMesh(output);
    Check(body.IsValid());
    for (float x : {2.25f, 5.75f}) {
        RJ::RayHit hit;
        Check(world.CastRay({{x, 5, 0.25f}, {0, -1, 0}}, 4, hit));
        Check(hit.body.GetJoltID() == body.GetJoltID());
        Check(std::abs(hit.distance - 3) < 0.001f && hit.normal.y > 0.99f);
    }
    mesh.hasSkinning = true;
    Check(!RJ::BuildRfbxCollisionMesh(model, output).valid);
    Check(output.vertices.empty() && output.indices.empty());
    world.Shutdown();

    float collisionVertices[] = {0, 0, 0, 0, 0, 1, 1, 0, 0};
    RfbxPrimitive collisionPrimitive{};
    collisionPrimitive.mesh.vertices = collisionVertices;
    collisionPrimitive.mesh.vertexCount = 3;
    collisionPrimitive.mesh.triangleCount = 1;
    RfbxMesh collisionMesh{};
    collisionMesh.primitives = &collisionPrimitive;
    collisionMesh.primitiveCount = 1;
    RfbxNode collisionNode{};
    collisionNode.visible = true;
    collisionNode.geometryToWorld = MatrixTranslate(20, 0, 0);
    RfbxModel collisionModel{};
    collisionModel.valid = true;
    collisionModel.nodeCount = 1;
    collisionModel.nodes = &collisionNode;
    collisionModel.meshCount = 1;
    collisionModel.meshes = &collisionMesh;
    Check(RJ::BuildRfbxCollisionMesh(collisionModel, output, MatrixTranslate(0, 2, 0)).valid);
    const auto separateBody = world.CreateStaticMesh(output);
    Check(separateBody.IsValid());
    collisionVertices[0] = 100;
    output = {};
    RJ::RayHit separateHit;
    Check(world.CastRay({{20.25f, 5, 0.25f}, {0, -1, 0}}, 4, separateHit));
    Check(separateHit.body.GetJoltID() == separateBody.GetJoltID());
    Check(!world.CastRay({{2.25f, 5, 0.25f}, {0, -1, 0}}, 4, separateHit));
    Check(vertices[0] == 0 && nodes[0].geometryToWorld.m12 == 2);
    collisionModel.valid = false;
    Check(!RJ::BuildRfbxCollisionMesh(collisionModel, output).valid);
    Check(output.vertices.empty());
    world.Shutdown();
}
#endif

int main()
{
    RJ::RayJoltContext context;
    Check(context.Init());
    auto& world = context.GetWorld();
#ifdef RJ_WITH_RFBX
    CheckRfbxBridge(world);
#endif
    RJ::BodyHandle floor;
    {
        auto mesh = MakeFloor();
        floor = world.CreateStaticMesh(mesh);
        Check(floor.IsValid());
        mesh.vertices.assign(4, {100, 100, 100});
        mesh.indices.clear();
    }
    RJ::RayHit hit;
    Check(world.CastRay({{0, 5, 0}, {0, -1, 0}}, 10, hit));
    Check(hit.body.GetJoltID() == floor.GetJoltID());
    auto shape = world.GetbodyInterface().GetShape(floor.GetJoltID());
    const auto references = shape->GetRefCount();
    auto copy = floor;
    Check(world.DestroyBody(floor));
    Check(!floor.IsValid() && !world.ContainsBody(copy));
    Check(!world.DestroyBody(copy));
    Check(shape->GetRefCount() + 1 == references);
    shape = nullptr;

    floor = world.CreateStaticMesh(MakeFloor());
    shape = world.GetbodyInterface().GetShape(floor.GetJoltID());
    const auto shutdownReferences = shape->GetRefCount();
    world.Shutdown();
    Check(world.GetPhysicsSystem().GetNumBodies() == 0);
    Check(shape->GetRefCount() + 1 == shutdownReferences);
    shape = nullptr;
    world.Shutdown();
    Check(world.CreateStaticMesh(MakeFloor()).IsValid());
    context.End();
    context.End();
    Check(JPH::Factory::sInstance == nullptr);

    for (int cycle = 0; cycle < 5; ++cycle) {
        Check(context.Init());
        auto& nextWorld = context.GetWorld();
        for (int body = 0; body < 32; ++body)
            Check(nextWorld.CreateStaticMesh(MakeFloor()).IsValid());
        Check(nextWorld.GetPhysicsSystem().GetNumBodies() == 32);
        context.End();
        Check(JPH::Factory::sInstance == nullptr);
    }
    {
        RJ::RayJoltContext automatic;
        Check(automatic.Init());
        Check(automatic.GetWorld().CreateStaticMesh(MakeFloor()).IsValid());
    }
    Check(JPH::Factory::sInstance == nullptr);
    Check(context.Init());
    CheckMeshScene(context);
    context.GetWorld().Shutdown();
    CheckConvexHull(context);
    return 0;
}
