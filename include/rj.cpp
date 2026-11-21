#include "rj.hpp"
#ifdef RJ_WITH_RFBX
#include "rfbx.h"
#endif
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <atomic>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
//NOTE : ONLY ONE JOLT RUNTIME.
static RayJolt::JoltRuntime g_joltRuntime;

namespace RayJolt {

    namespace {
        std::uint64_t NextWorldId() {
            static std::atomic<std::uint64_t> next{1};
            auto id = next.load(std::memory_order_relaxed);
            do {
                if (id == std::numeric_limits<std::uint64_t>::max())
                    throw std::overflow_error("World identifiers exhausted");
            } while (!next.compare_exchange_weak(id, id + 1, std::memory_order_relaxed));
            return id;
        }
    }

    PhysicsWorld::PhysicsWorld() : runtimeState_([] {
        auto state = JoltRuntime::activeState_.lock();
        if (!state)
            throw std::logic_error("JoltRuntime::Init() must succeed before creating a PhysicsWorld");
        return state;
    }()), worldId_(NextWorldId()) {}

    BodyHandle PhysicsWorld::RegisterBody(JPH::BodyID id) {
        if (id.IsInvalid())
            return {};
        if (nextBodyGeneration_ == std::numeric_limits<std::uint64_t>::max())
            throw std::overflow_error("Body generations exhausted");
        bodyGenerations_[id.GetIndexAndSequenceNumber()] = nextBodyGeneration_++;
        return FindBodyHandle(id);
    }

    BodyHandle PhysicsWorld::FindBodyHandle(JPH::BodyID id) const {
        const auto found = bodyGenerations_.find(id.GetIndexAndSequenceNumber());
        return found == bodyGenerations_.end() ? BodyHandle{} : BodyHandle(id, worldId_, found->second);
    }

    MeshValidationResult ValidateTriangleMesh(const TriangleMeshData& mesh) {
        //triangle size small size guard.
        if (mesh.vertices.size() < 3)
            return {false, "At least three vertices are required"};
        if (mesh.indices.empty() || mesh.indices.size() % 3 != 0)
            return {false, "Indices must contain complete triangles"};

        for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
            const auto& vertex = mesh.vertices[i];
            if (!std::isfinite(vertex.x) || !std::isfinite(vertex.y) || !std::isfinite(vertex.z))
                return {false, "Vertex " + std::to_string(i) + " must have finite coordinates"};
        }

        for (std::size_t i = 0; i < mesh.indices.size(); i += 3) {
            const auto a = mesh.indices[i];
            const auto b = mesh.indices[i + 1];
            const auto c = mesh.indices[i + 2];
            const auto triangle = std::to_string(i / 3);
            if (a >= mesh.vertices.size() || b >= mesh.vertices.size() || c >= mesh.vertices.size())
                return {false, "Triangle " + triangle + " contains an out-of-range vertex index"};

            const auto& p = mesh.vertices[a];
            const auto& q = mesh.vertices[b];
            const auto& r = mesh.vertices[c];
            const double ux = double(q.x) - p.x;
            const double uy = double(q.y) - p.y;
            const double uz = double(q.z) - p.z;
            const double vx = double(r.x) - p.x;
            const double vy = double(r.y) - p.y;
            const double vz = double(r.z) - p.z;
            if (uy * vz - uz * vy == 0.0 && uz * vx - ux * vz == 0.0 && ux * vy - uy * vx == 0.0)
                return {false, "Triangle " + triangle + " has zero area"};
        }
        return {true, {}};
    }

#ifdef RJ_WITH_RFBX
    MeshValidationResult BuildRfbxCollisionMesh(const RfbxModel& model, TriangleMeshData& output, Matrix transform) {
        output = {};
        if (!model.valid || !model.nodes || !model.meshes)
            return {false, "Invalid rfbx model"};

        TriangleMeshData result;
        for (int i = 0; i < model.nodeCount; ++i) {
            const auto& node = model.nodes[i];
            if (!node.visible || node.meshIndex < 0)
                continue;
            if (node.meshIndex >= model.meshCount)
                return {false, "Invalid rfbx node mesh index"};
            const auto& mesh = model.meshes[node.meshIndex];
            if (mesh.hasSkinning || mesh.primitiveCount == 0)
                continue;
            if (!mesh.primitives)
                return {false, "Missing rfbx mesh primitives"};

            const auto matrix = MatrixMultiply(node.geometryToWorld, transform);
            const float determinant = MatrixDeterminant(matrix);
            if (!std::isfinite(determinant) || determinant == 0.0f)
                return {false, "Invalid rfbx geometry transform"};
            for (int j = 0; j < mesh.primitiveCount; ++j) {
                const auto& primitive = mesh.primitives[j].mesh;
                if (!primitive.vertices || primitive.vertexCount <= 0 ||
                    primitive.vertexCount % 3 != 0 || primitive.indices ||
                    primitive.triangleCount != primitive.vertexCount / 3)
                    return {false, "Invalid rfbx triangle stream"};
                const auto base = result.vertices.size();
                if (base > std::numeric_limits<std::uint32_t>::max() - std::uint32_t(primitive.vertexCount))
                    return {false, "Too many rfbx collision vertices"};
                for (int k = 0; k < primitive.vertexCount; ++k) {
                    const auto* p = primitive.vertices + std::size_t(k) * 3;
                    result.vertices.push_back(Vector3Transform({p[0], p[1], p[2]}, matrix));
                }
                for (int k = 0; k < primitive.vertexCount; k += 3) {
                    const auto a = static_cast<std::uint32_t>(base + k);
                    result.indices.push_back(a);
                    result.indices.push_back(a + (determinant < 0.0f ? 2 : 1));
                    result.indices.push_back(a + (determinant < 0.0f ? 1 : 2));
                }
            }
        }
        const auto validation = ValidateTriangleMesh(result);
        if (validation.valid)
            output = std::move(result);
        return validation;
    }
#endif

    static JPH::EMotionType ToJoltMotionType(MotionType type) {
        switch (type) {
        case MotionType::Static: return JPH::EMotionType::Static;
        case MotionType::Dynamic: return JPH::EMotionType::Dynamic;
        case MotionType::Kinematic: return JPH::EMotionType::Kinematic;
        }
        throw std::invalid_argument("Unknown motion type");
    }

    struct JoltRuntime::State {
        std::unique_ptr<JPH::Factory> factory;
        std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;
        std::mutex updateMutex;

        ~State() {
            jobSystem.reset();
            tempAllocator.reset();
            if (factory) {
                JPH::UnregisterTypes();
                JPH::Factory::sInstance = nullptr;
                factory.reset();
                TraceLog(LOG_INFO, "rayjolt Runtime resources released.");
            }
        }

        JPH::EPhysicsUpdateError Update(JPH::PhysicsSystem& physics, float deltaTime, int collisionSteps) {
            return physics.Update(deltaTime, collisionSteps, tempAllocator.get(), jobSystem.get());
        }
    };

    std::weak_ptr<JoltRuntime::State> JoltRuntime::activeState_;

    JoltRuntime::~JoltRuntime() = default;

    bool JoltRuntime::Init() {
        if (state_)
            return true;
        if (JPH::Factory::sInstance != nullptr)
            return false;
        auto state = std::make_shared<State>();
        JPH::RegisterDefaultAllocator();
        state->factory = std::make_unique<JPH::Factory>();
        JPH::Factory::sInstance = state->factory.get();
        JPH::RegisterTypes();
        state->tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        const unsigned cores = std::thread::hardware_concurrency();
        state->jobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs,
            JPH::cMaxPhysicsBarriers, static_cast<int>(cores > 1 ? cores - 1 : 1));
        state_ = std::move(state);
        activeState_ = state_;
        TraceLog(LOG_INFO, "rayjolt Runtime initialized.");
        return true;
    }

    void JoltRuntime::End() {
        if (!state_)
            return;
        activeState_.reset();
        state_.reset();
    }

    const std::unique_ptr<JPH::JobSystemThreadPool>& JoltRuntime::GetJobSystem() const {
        static const std::unique_ptr<JPH::JobSystemThreadPool> empty;
        return state_ ? state_->jobSystem : empty;
    }

    const std::unique_ptr<JPH::TempAllocatorImpl>& JoltRuntime::GetTempAllocator() const {
        static const std::unique_ptr<JPH::TempAllocatorImpl> empty;
        return state_ ? state_->tempAllocator : empty;
    }

    std::unique_ptr<PhysicsWorld> JoltRuntime::CreateWorld() {
        if (!state_)
            throw std::logic_error("JoltRuntime::Init() must succeed before CreateWorld()");
        auto world = std::make_unique<PhysicsWorld>();
        if (!world->Init())
            return nullptr;
        return world;
    }

    void PhysicsWorld::ContactEventListener::Record(ContactEventType type, const JPH::Body& a,
        const JPH::Body& b, const JPH::ContactManifold& manifold) {
        ContactEvent event;
        event.type = type;
        event.bodyA = world_.FindBodyHandle(a.GetID());
        event.bodyB = world_.FindBodyHandle(b.GetID());
        if (!event.bodyA.IsValid() || !event.bodyB.IsValid())
            return;
        event.subShapeA = manifold.mSubShapeID1.GetValue();
        event.subShapeB = manifold.mSubShapeID2.GetValue();
        event.normal = Detail::ToRaylibVector3(manifold.mWorldSpaceNormal);
        if (!manifold.mRelativeContactPointsOn1.empty()) {
            event.point = Detail::ToRaylibRVector3(manifold.GetWorldSpaceContactPointOn1(0));
            event.hasContactPoint = true;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        contacts_[JPH::SubShapeIDPair(a.GetID(), manifold.mSubShapeID1, b.GetID(), manifold.mSubShapeID2)] =
            {event.bodyA, event.bodyB};
        events_.push_back(event);
    }

    void PhysicsWorld::ContactEventListener::OnContactAdded(const JPH::Body& a, const JPH::Body& b,
        const JPH::ContactManifold& manifold, JPH::ContactSettings&) {
        Record(ContactEventType::Begin, a, b, manifold);
    }

    void PhysicsWorld::ContactEventListener::OnContactPersisted(const JPH::Body& a, const JPH::Body& b,
        const JPH::ContactManifold& manifold, JPH::ContactSettings&) {
        Record(ContactEventType::Stay, a, b, manifold);
    }

    void PhysicsWorld::ContactEventListener::OnContactRemoved(const JPH::SubShapeIDPair& pair) {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto found = contacts_.find(pair);
        if (found == contacts_.end())
            return;
        ContactEvent event;
        event.type = ContactEventType::End;
        event.bodyA = found->second.first;
        event.bodyB = found->second.second;
        event.subShapeA = pair.GetSubShapeID1().GetValue();
        event.subShapeB = pair.GetSubShapeID2().GetValue();
        contacts_.erase(found);
        events_.push_back(event);
    }

    void PhysicsWorld::ContactEventListener::Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        events_.clear();
    }

    void PhysicsWorld::ContactEventListener::Reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        events_.clear();
        contacts_.clear();
    }

    std::vector<ContactEvent> PhysicsWorld::ContactEventListener::GetEvents() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return events_;
    }

    std::vector<ContactEvent> PhysicsWorld::GetContactEvents() const {
        return contactListener_.GetEvents();
    }

    bool PhysicsWorld::IsSensor(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        return physicsSystem_.GetBodyInterface().IsSensor(body.GetJoltID());
    }

    PhysicsWorld::~PhysicsWorld() {
        physicsSystem_.SetContactListener(nullptr);
        Shutdown();
    }

    bool PhysicsWorld::Init()
    {
        if (initialized_)
            return true;
        if (runtimeState_ != JoltRuntime::activeState_.lock())
            return false;
        layerPairs_.EnableCollision(ObjectLayers::NON_MOVING, ObjectLayers::MOVING);
        layerPairs_.EnableCollision(ObjectLayers::MOVING, ObjectLayers::MOVING);
        broadPhaseLayers_.MapObjectToBroadPhaseLayer(ObjectLayers::NON_MOVING, BroadPhaseLayers::NON_MOVING);
        broadPhaseLayers_.MapObjectToBroadPhaseLayer(ObjectLayers::MOVING, BroadPhaseLayers::MOVING);
        broadPhaseFilter_ = std::make_unique<JPH::ObjectVsBroadPhaseLayerFilterTable>(
            broadPhaseLayers_, 2, layerPairs_, 2);

        physicsSystem_.Init(1024, 0, 1024, 1024, broadPhaseLayers_, *broadPhaseFilter_, layerPairs_);
        physicsSystem_.SetContactListener(&contactListener_);
        initialized_ = true;

        return true;
    }

    void PhysicsWorld::Update(float deltaTime, int collisionStep) {
        RJ_Update(*this, deltaTime, collisionStep);
    }

    void PhysicsWorld::Shutdown() {
        if (!initialized_)
            return;
        for (auto& state : characters_)
            state.character->RemoveFromPhysicsSystem();
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
        bodyGenerations_.clear();
        contactListener_.Reset();
    }

    void PhysicsWorld::SetGravity(Vector3 gravity) {
        if (!std::isfinite(gravity.x) || !std::isfinite(gravity.y) || !std::isfinite(gravity.z))
            throw std::invalid_argument("Gravity must be finite");
        const auto value = Detail::ToJoltVec3(gravity);
        if (physicsSystem_.GetGravity() == value)
            return;
        physicsSystem_.SetGravity(value);
        JPH::BodyIDVector bodyIDs;
        physicsSystem_.GetBodies(bodyIDs);
        auto& bodies = physicsSystem_.GetBodyInterface();
        for (const auto& id : bodyIDs)
            if (bodies.GetMotionType(id) == JPH::EMotionType::Dynamic)
                bodies.ActivateBody(id);
    }

    Vector3 PhysicsWorld::GetGravity() const {
        return Detail::ToRaylibVector3(physicsSystem_.GetGravity());
    }

    void PhysicsWorld::SetGravityFactor(BodyHandle body, float factor) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        if (!std::isfinite(factor) || factor < 0.0f)
            throw std::invalid_argument("Gravity factor must be finite and non-negative");
        auto& bodies = physicsSystem_.GetBodyInterface();
        if (bodies.GetMotionType(body.GetJoltID()) != JPH::EMotionType::Dynamic)
            throw std::invalid_argument("Gravity factor requires a dynamic body");
        if (bodies.GetGravityFactor(body.GetJoltID()) == factor)
            return;
        bodies.SetGravityFactor(body.GetJoltID(), factor);
        bodies.ActivateBody(body.GetJoltID());
    }

    float PhysicsWorld::GetGravityFactor(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        const auto& bodies = physicsSystem_.GetBodyInterface();
        if (bodies.GetMotionType(body.GetJoltID()) != JPH::EMotionType::Dynamic)
            throw std::invalid_argument("Gravity factor requires a dynamic body");
        return bodies.GetGravityFactor(body.GetJoltID());
    }

    void PhysicsWorld::SetFriction(BodyHandle body, float value) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        if (!std::isfinite(value) || value < 0.0f)
            throw std::invalid_argument("Friction must be finite and non-negative");
        auto& bodies = physicsSystem_.GetBodyInterface();
        if (bodies.GetFriction(body.GetJoltID()) == value)
            return;
        bodies.SetFriction(body.GetJoltID(), value);
        bodies.InvalidateContactCache(body.GetJoltID());
        if (bodies.GetMotionType(body.GetJoltID()) == JPH::EMotionType::Dynamic)
            bodies.ActivateBody(body.GetJoltID());
    }

    float PhysicsWorld::GetFriction(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        return physicsSystem_.GetBodyInterface().GetFriction(body.GetJoltID());
    }

    void PhysicsWorld::SetRestitution(BodyHandle body, float value) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        if (!std::isfinite(value) || value < 0.0f || value > 1.0f)
            throw std::invalid_argument("Restitution must be between zero and one");
        auto& bodies = physicsSystem_.GetBodyInterface();
        if (bodies.GetRestitution(body.GetJoltID()) == value)
            return;
        bodies.SetRestitution(body.GetJoltID(), value);
        bodies.InvalidateContactCache(body.GetJoltID());
        if (bodies.GetMotionType(body.GetJoltID()) == JPH::EMotionType::Dynamic)
            bodies.ActivateBody(body.GetJoltID());
    }

    float PhysicsWorld::GetRestitution(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        return physicsSystem_.GetBodyInterface().GetRestitution(body.GetJoltID());
    }

    void PhysicsWorld::SetMass(BodyHandle body, float mass) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        if (!std::isfinite(mass) || mass < std::numeric_limits<float>::min())
            throw std::invalid_argument("Mass must be a positive finite normal float");
        {
            JPH::BodyLockWrite lock(physicsSystem_.GetBodyLockInterface(), body.GetJoltID());
            if (!lock.Succeeded())
                throw std::invalid_argument("Body does not exist in this world");
            if (!lock.GetBody().IsDynamic())
                throw std::invalid_argument("Mass requires a dynamic body");
            auto* motion = lock.GetBody().GetMotionProperties();
            if (motion->GetInverseMass() <= 0.0f)
                throw std::invalid_argument("Body must have finite mass to change mass");
            motion->ScaleToMass(mass);
        }
        auto& bodies = physicsSystem_.GetBodyInterface();
        bodies.InvalidateContactCache(body.GetJoltID());
        bodies.ActivateBody(body.GetJoltID());
    }

    float PhysicsWorld::GetMass(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        JPH::BodyLockRead lock(physicsSystem_.GetBodyLockInterface(), body.GetJoltID());
        if (!lock.Succeeded())
            throw std::invalid_argument("Body does not exist in this world");
        if (!lock.GetBody().IsDynamic())
            throw std::invalid_argument("Mass requires a dynamic body");
        const float inverseMass = lock.GetBody().GetMotionProperties()->GetInverseMass();
        if (inverseMass <= 0.0f)
            throw std::invalid_argument("Body does not have finite mass");
        return 1.0f / inverseMass;
    }

    void PhysicsWorld::SetLinearDamping(BodyHandle body, float damping) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        if (!std::isfinite(damping) || damping < 0.0f)
            throw std::invalid_argument("Damping must be finite and non-negative");
        {
            JPH::BodyLockWrite lock(physicsSystem_.GetBodyLockInterface(), body.GetJoltID());
            if (!lock.Succeeded())
                throw std::invalid_argument("Body does not exist in this world");
            if (!lock.GetBody().IsDynamic())
                throw std::invalid_argument("Damping requires a dynamic body");
            auto* motion = lock.GetBody().GetMotionProperties();
            if (motion->GetLinearDamping() == damping)
                return;
            motion->SetLinearDamping(damping);
        }
        physicsSystem_.GetBodyInterface().ActivateBody(body.GetJoltID());
    }

    float PhysicsWorld::GetLinearDamping(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        JPH::BodyLockRead lock(physicsSystem_.GetBodyLockInterface(), body.GetJoltID());
        if (!lock.Succeeded())
            throw std::invalid_argument("Body does not exist in this world");
        if (!lock.GetBody().IsDynamic())
            throw std::invalid_argument("Damping requires a dynamic body");
        return lock.GetBody().GetMotionProperties()->GetLinearDamping();
    }

    void PhysicsWorld::SetAngularDamping(BodyHandle body, float damping) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        if (!std::isfinite(damping) || damping < 0.0f)
            throw std::invalid_argument("Damping must be finite and non-negative");
        {
            JPH::BodyLockWrite lock(physicsSystem_.GetBodyLockInterface(), body.GetJoltID());
            if (!lock.Succeeded())
                throw std::invalid_argument("Body does not exist in this world");
            if (!lock.GetBody().IsDynamic())
                throw std::invalid_argument("Damping requires a dynamic body");
            auto* motion = lock.GetBody().GetMotionProperties();
            if (motion->GetAngularDamping() == damping)
                return;
            motion->SetAngularDamping(damping);
        }
        physicsSystem_.GetBodyInterface().ActivateBody(body.GetJoltID());
    }

    float PhysicsWorld::GetAngularDamping(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        JPH::BodyLockRead lock(physicsSystem_.GetBodyLockInterface(), body.GetJoltID());
        if (!lock.Succeeded())
            throw std::invalid_argument("Body does not exist in this world");
        if (!lock.GetBody().IsDynamic())
            throw std::invalid_argument("Damping requires a dynamic body");
        return lock.GetBody().GetMotionProperties()->GetAngularDamping();
    }

    void PhysicsWorld::WakeBody(BodyHandle body) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        auto& bodies = physicsSystem_.GetBodyInterface();
        if (bodies.GetMotionType(body.GetJoltID()) != JPH::EMotionType::Dynamic || !bodies.IsAdded(body.GetJoltID()))
            throw std::invalid_argument("WakeBody requires a dynamic body added to the world");
        bodies.ActivateBody(body.GetJoltID());
    }

    void PhysicsWorld::SleepBody(BodyHandle body) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        auto& bodies = physicsSystem_.GetBodyInterface();
        if (bodies.GetMotionType(body.GetJoltID()) != JPH::EMotionType::Dynamic || !bodies.IsAdded(body.GetJoltID()))
            throw std::invalid_argument("SleepBody requires a dynamic body added to the world");
        bodies.DeactivateBody(body.GetJoltID());
    }

    bool PhysicsWorld::IsActive(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        return physicsSystem_.GetBodyInterface().IsActive(body.GetJoltID());
    }

    void PhysicsWorld::SetContinuousCollision(BodyHandle body, bool enabled) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        auto& bodies = physicsSystem_.GetBodyInterface();
        if (bodies.GetMotionType(body.GetJoltID()) != JPH::EMotionType::Dynamic)
            throw std::invalid_argument("Continuous collision requires a dynamic body");
        const auto quality = enabled ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete;
        if (bodies.GetMotionQuality(body.GetJoltID()) == quality)
            return;
        bodies.SetMotionQuality(body.GetJoltID(), quality);
        if (bodies.IsAdded(body.GetJoltID()))
            bodies.ActivateBody(body.GetJoltID());
    }

    bool PhysicsWorld::IsContinuousCollisionEnabled(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        const auto& bodies = physicsSystem_.GetBodyInterface();
        if (bodies.GetMotionType(body.GetJoltID()) != JPH::EMotionType::Dynamic)
            throw std::invalid_argument("Continuous collision requires a dynamic body");
        return bodies.GetMotionQuality(body.GetJoltID()) == JPH::EMotionQuality::LinearCast;
    }

    bool PhysicsWorld::ContainsBody(BodyHandle body) const {
        if (!body.IsValid() || body.worldId_ != worldId_)
            return false;
        const auto found = bodyGenerations_.find(body.GetJoltID().GetIndexAndSequenceNumber());
        if (found == bodyGenerations_.end() || found->second != body.generation_)
            return false;
        JPH::BodyLockRead lock(physicsSystem_.GetBodyLockInterface(), body.GetJoltID());
        return lock.Succeeded();
    }

    bool PhysicsWorld::DestroyBody(BodyHandle& body) {
        if (!ContainsBody(body)) {
            body = BodyHandle{};
            return false;
        }
        auto& bodies = physicsSystem_.GetBodyInterface();
        const auto id = body.GetJoltID();
        for (auto it = characters_.begin(); it != characters_.end(); ++it) {
            if (it->character->GetBodyID() == id) {
                it->character->RemoveFromPhysicsSystem();
                characters_.erase(it);
                jumpingCharacters_.erase(std::remove(jumpingCharacters_.begin(), jumpingCharacters_.end(), id), jumpingCharacters_.end());
                bodyGenerations_.erase(id.GetIndexAndSequenceNumber());
                body = BodyHandle{};
                return true;
            }
        }
        if (bodies.IsAdded(id))
            bodies.RemoveBody(id);
        bodies.DestroyBody(id);
        bodyGenerations_.erase(id.GetIndexAndSequenceNumber());
        body = BodyHandle{};
        return true;
    }

    BodyHandle PhysicsWorld::CreateSphere(const SphereDesc& desc) {
        if (!std::isfinite(desc.radius) || desc.radius <= 0.0f)
            return BodyHandle{};
        const auto motionType = ToJoltMotionType(desc.motionType);
        JPH::BodyCreationSettings settings(
            new JPH::SphereShape(desc.radius),
            Detail::ToJoltRVec3(desc.position),
            Detail::ToJoltQuaternion(desc.rotation),
            motionType,
            desc.motionType == MotionType::Static ? ObjectLayers::NON_MOVING : ObjectLayers::MOVING);
        settings.mIsSensor = desc.isSensor;
        settings.mRestitution = desc.restitution;
        settings.mFriction = desc.friction;
        return RegisterBody(physicsSystem_.GetBodyInterface().CreateAndAddBody(settings,
            desc.motionType == MotionType::Static ? JPH::EActivation::DontActivate : JPH::EActivation::Activate));
    }

    BodyHandle PhysicsWorld::CreateCapsule(const CapsuleDesc& desc) {
        if (!std::isfinite(desc.radius) || desc.radius <= 0.0f ||
            !std::isfinite(desc.height) || desc.height < 2.0f * desc.radius)
            return BodyHandle{};

        const auto motionType = ToJoltMotionType(desc.motionType);
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
            motionType,
            desc.motionType == MotionType::Static ? ObjectLayers::NON_MOVING : ObjectLayers::MOVING);
        settings.mIsSensor = desc.isSensor;
        settings.mRestitution = desc.restitution;
        settings.mFriction = desc.friction;
        return RegisterBody(physicsSystem_.GetBodyInterface().CreateAndAddBody(settings,
            desc.motionType == MotionType::Static ? JPH::EActivation::DontActivate : JPH::EActivation::Activate));
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
        for (auto& state : characters_) {
            if (state.character->GetBodyID() == body.GetJoltID()) {
                state.groundVelocity = JPH::Vec3::sZero();
                state.character->PostSimulation(0.05f);
                break;
            }
        }
    }

    void PhysicsWorld::MoveKinematic(BodyHandle body, Vector3 position, Quaternion rotation, float deltaTime) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        auto& bodies = physicsSystem_.GetBodyInterface();
        if (bodies.GetMotionType(body.GetJoltID()) != JPH::EMotionType::Kinematic)
            throw std::invalid_argument("MoveKinematic requires a kinematic body");
        if (!std::isfinite(deltaTime) || deltaTime <= 0.0f ||
            !std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z) ||
            !std::isfinite(rotation.x) || !std::isfinite(rotation.y) ||
            !std::isfinite(rotation.z) || !std::isfinite(rotation.w))
            throw std::invalid_argument("Target transform must be finite and deltaTime must be positive");
        const double length = std::sqrt(double(rotation.x) * rotation.x + double(rotation.y) * rotation.y +
            double(rotation.z) * rotation.z + double(rotation.w) * rotation.w);
        if (length == 0.0)
            throw std::invalid_argument("Rotation must not be a zero quaternion");
        const JPH::Quat normalized(float(rotation.x / length), float(rotation.y / length),
            float(rotation.z / length), float(rotation.w / length));
        bodies.MoveKinematic(body.GetJoltID(), Detail::ToJoltRVec3(position), normalized, deltaTime);
    }

    Quaternion PhysicsWorld::GetRotation(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        return Detail::ToRaylibQuaternion(physicsSystem_.GetBodyInterface().GetRotation(body.GetJoltID()));
    }

    Matrix PhysicsWorld::GetTransform(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        return Detail::ToRaylibRMatrix(physicsSystem_.GetBodyInterface().GetWorldTransform(body.GetJoltID()));
    }

    BoundingBox PhysicsWorld::GetBoundingBox(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        JPH::BodyLockRead lock(physicsSystem_.GetBodyLockInterface(), body.GetJoltID());
        if (!lock.Succeeded())
            throw std::invalid_argument("Body does not exist in this world");
        return Detail::ToRaylibBoundingBox(lock.GetBody().GetWorldSpaceBounds());
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

    void PhysicsWorld::AddForce(BodyHandle body, Vector3 force, Vector3 worldPosition) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        if (!std::isfinite(force.x) || !std::isfinite(force.y) || !std::isfinite(force.z) ||
            !std::isfinite(worldPosition.x) || !std::isfinite(worldPosition.y) || !std::isfinite(worldPosition.z))
            throw std::invalid_argument("Force or impulse and world position must be finite");
        physicsSystem_.GetBodyInterface().AddForce(body.GetJoltID(),
            Detail::ToJoltVec3(force), Detail::ToJoltRVec3(worldPosition));
    }

    void PhysicsWorld::AddImpulse(BodyHandle body, Vector3 impulse, Vector3 worldPosition) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        if (!std::isfinite(impulse.x) || !std::isfinite(impulse.y) || !std::isfinite(impulse.z) ||
            !std::isfinite(worldPosition.x) || !std::isfinite(worldPosition.y) || !std::isfinite(worldPosition.z))
            throw std::invalid_argument("Force or impulse and world position must be finite");
        physicsSystem_.GetBodyInterface().AddImpulse(body.GetJoltID(),
            Detail::ToJoltVec3(impulse), Detail::ToJoltRVec3(worldPosition));
    }

    void PhysicsWorld::AddTorque(BodyHandle body, Vector3 torque) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        if (!std::isfinite(torque.x) || !std::isfinite(torque.y) || !std::isfinite(torque.z))
            throw std::invalid_argument("Torque must be finite");
        physicsSystem_.GetBodyInterface().AddTorque(body.GetJoltID(), Detail::ToJoltVec3(torque));
    }

    void PhysicsWorld::AddAngularImpulse(BodyHandle body, Vector3 impulse) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        if (!std::isfinite(impulse.x) || !std::isfinite(impulse.y) || !std::isfinite(impulse.z))
            throw std::invalid_argument("Angular impulse must be finite");
        physicsSystem_.GetBodyInterface().AddAngularImpulse(body.GetJoltID(), Detail::ToJoltVec3(impulse));
    }

    BodyHandle PhysicsWorld::CreateCharacter(const CharacterDesc& desc) {
        if (!std::isfinite(desc.radius) || desc.radius <= 0.0f ||
            !std::isfinite(desc.height) || desc.height <= 2.0f * desc.radius ||
            !std::isfinite(desc.mass) || desc.mass <= 0.0f ||
            !std::isfinite(desc.maxSlopeAngleDegrees) || desc.maxSlopeAngleDegrees < 1.0f ||
            desc.maxSlopeAngleDegrees > 90.0f ||
            !std::isfinite(desc.maxStepHeight) || desc.maxStepHeight < 0.0f ||
            !std::isfinite(desc.position.x) || !std::isfinite(desc.position.y) ||
            !std::isfinite(desc.position.z) ||
            physicsSystem_.GetNumBodies() >= physicsSystem_.GetMaxBodies())
            return BodyHandle{};

        const float halfCylinderHeight = desc.height * 0.5f - desc.radius;
        JPH::CharacterSettings settings;
        settings.mShape = new JPH::CapsuleShape(halfCylinderHeight, desc.radius);
        settings.mMass = desc.mass;
        settings.mMaxSlopeAngle = desc.maxSlopeAngleDegrees * DEG2RAD;
        settings.mLayer = ObjectLayers::MOVING;
        settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), halfCylinderHeight);
        JPH::Ref<JPH::Character> character = new JPH::Character(
            &settings, Detail::ToJoltRVec3(desc.position), JPH::Quat::sIdentity(), 0, &physicsSystem_);
        const float crouchingHeight = (desc.height + 2.0f * desc.radius) * 0.5f;
        characters_.push_back({character, settings.mShape,
            new JPH::CapsuleShape(halfCylinderHeight * 0.5f, desc.radius),
            desc.height, crouchingHeight, desc.radius});
        characters_.back().maxStepHeight = desc.maxStepHeight;
        character->AddToPhysicsSystem();
        return RegisterBody(character->GetBodyID());
    }

    bool PhysicsWorld::IsGrounded(BodyHandle body) const {
        return GetCharacterGroundInfo(body).state == GroundState::OnGround;
    }

    CharacterGroundInfo PhysicsWorld::GetCharacterGroundInfo(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        for (const auto& state : characters_) {
            const auto& character = state.character;
            if (character->GetBodyID() != body.GetJoltID())
                continue;
            CharacterGroundInfo info;
            switch (character->GetGroundState()) {
            case JPH::CharacterBase::EGroundState::OnGround:
                info.state = GroundState::OnGround;
                break;
            case JPH::CharacterBase::EGroundState::OnSteepGround:
                info.state = GroundState::OnSteepGround;
                break;
            case JPH::CharacterBase::EGroundState::NotSupported:
                info.state = GroundState::NotSupported;
                break;
            case JPH::CharacterBase::EGroundState::InAir:
                return info;
            }
            info.body = FindBodyHandle(character->GetGroundBodyID());
            if (!ContainsBody(info.body))
                return CharacterGroundInfo{};
            info.position = Detail::ToRaylibRVector3(character->GetGroundPosition());
            info.normal = Detail::ToRaylibVector3(character->GetGroundNormal());
            info.velocity = Detail::ToRaylibVector3(character->GetGroundVelocity());
            return info;
        }
        throw std::invalid_argument("Body is not a character in this world");
    }

    void PhysicsWorld::MoveCharacter(BodyHandle body, Vector2 direction, float speed) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        for (auto& state : characters_) {
            auto& character = state.character;
            if (character->GetBodyID() != body.GetJoltID())
                continue;
            if (!std::isfinite(direction.x) || !std::isfinite(direction.y) ||
                !std::isfinite(speed) || speed < 0.0f)
                throw std::invalid_argument("Movement direction must be finite and speed must be finite and non-negative");

            const double length = std::hypot(double(direction.x), double(direction.y));
            const double scale = speed / std::max(1.0, length);
            state.movement = JPH::Vec3(float(direction.x * scale), 0, float(direction.y * scale));
            auto velocity = character->GetLinearVelocity();
            velocity.SetX(float(direction.x * scale));
            velocity.SetZ(float(direction.y * scale));
            const auto ground = GetCharacterGroundInfo(body);
            if (ground.state == GroundState::OnSteepGround || ground.state == GroundState::NotSupported) {
                const JPH::Vec3 normal(ground.normal.x, 0.0f, ground.normal.z);
                JPH::Vec3 horizontal(velocity.GetX(), 0.0f, velocity.GetZ());
                const float intoSurface = normal.Dot(horizontal);
                if (intoSurface < 0.0f)
                    horizontal -= normal * (intoSurface / normal.LengthSq());
                velocity.SetX(horizontal.GetX());
                velocity.SetZ(horizontal.GetZ());
            }
            const bool jumping = std::find(jumpingCharacters_.begin(), jumpingCharacters_.end(), body.GetJoltID()) != jumpingCharacters_.end();
            if (ground.state == GroundState::OnGround && !jumping) {
                const auto groundVelocity = physicsSystem_.GetBodyInterface().GetPointVelocity(
                    ground.body.GetJoltID(), Detail::ToJoltRVec3(ground.position));
                if (velocity.GetY() - state.groundVelocity.GetY() <= 0.1f)
                    velocity.SetY(groundVelocity.GetY());
                state.groundVelocity = groundVelocity;
            }
            velocity.SetX(velocity.GetX() + state.groundVelocity.GetX());
            velocity.SetZ(velocity.GetZ() + state.groundVelocity.GetZ());
            character->SetLinearVelocity(velocity);
            return;
        }
        throw std::invalid_argument("Body is not a character in this world");
    }

    bool PhysicsWorld::SetCrouching(BodyHandle body, bool crouching) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        for (auto& state : characters_) {
            if (state.character->GetBodyID() != body.GetJoltID())
                continue;
            if (state.crouching == crouching)
                return true;

            const float oldHeight = state.crouching ? state.crouchingHeight : state.standingHeight;
            const float newHeight = crouching ? state.crouchingHeight : state.standingHeight;
            const auto oldPosition = state.character->GetPosition();
            state.character->SetPosition(oldPosition + JPH::RVec3(0, (newHeight - oldHeight) * 0.5f, 0));
            const auto& shape = crouching ? state.crouchingShape : state.standingShape;
            const float allowedPenetration = physicsSystem_.GetPhysicsSettings().mPenetrationSlop * 1.5f;
            if (!state.character->SetShape(shape, allowedPenetration)) {
                state.character->SetPosition(oldPosition);
                return false;
            }
            state.crouching = crouching;
            state.character->SetSupportingVolume(JPH::Plane(JPH::Vec3::sAxisY(), newHeight * 0.5f - state.radius));
            state.character->PostSimulation(0.05f);
            return true;
        }
        throw std::invalid_argument("Body is not a character in this world");
    }

    bool PhysicsWorld::IsCrouching(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        for (const auto& state : characters_)
            if (state.character->GetBodyID() == body.GetJoltID())
                return state.crouching;
        throw std::invalid_argument("Body is not a character in this world");
    }

    float PhysicsWorld::GetCharacterHeight(BodyHandle body) const {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        for (const auto& state : characters_)
            if (state.character->GetBodyID() == body.GetJoltID())
                return state.crouching ? state.crouchingHeight : state.standingHeight;
        throw std::invalid_argument("Body is not a character in this world");
    }

    bool PhysicsWorld::Jump(BodyHandle body, float speed) {
        if (!ContainsBody(body))
            throw std::invalid_argument("Body does not exist in this world");
        for (auto& state : characters_) {
            auto& character = state.character;
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
            const auto groundBody = character->GetGroundBodyID();
            const auto groundVelocity = physicsSystem_.GetBodyInterface().GetPointVelocity(
                groundBody, character->GetGroundPosition());
            velocity.SetX(velocity.GetX() + groundVelocity.GetX() - state.groundVelocity.GetX());
            velocity.SetZ(velocity.GetZ() + groundVelocity.GetZ() - state.groundVelocity.GetZ());
            velocity.SetY(groundVelocity.GetY() + speed);
            state.groundVelocity = groundVelocity;
            jumpingCharacters_.push_back(id);
            character->SetLinearVelocity(velocity);
            character->Activate();
            return true;
        }
        throw std::invalid_argument("Body is not a character in this world");
    }

    void PhysicsWorld::UpdateCharacterSteps(float deltaTime) {
        if (!std::isfinite(deltaTime) || deltaTime <= 0.0f)
            return;
        class StepFilter final : public JPH::IgnoreSingleBodyFilter {
        public:
            using JPH::IgnoreSingleBodyFilter::IgnoreSingleBodyFilter;
            bool ShouldCollideLocked(const JPH::Body& body) const override { return !body.IsSensor(); }
        };
        class BlockingHit final : public JPH::ClosestHitCollisionCollector<JPH::CastShapeCollector> {
        public:
            explicit BlockingHit(JPH::Vec3 direction) : direction_(direction) {}
            void AddHit(const JPH::ShapeCastResult& hit) override {
                if (hit.mPenetrationAxis.Dot(direction_) > 1.0e-5f * direction_.Length() * hit.mPenetrationAxis.Length())
                    JPH::ClosestHitCollisionCollector<JPH::CastShapeCollector>::AddHit(hit);
            }
        private:
            JPH::Vec3 direction_;
        };
        for (auto& state : characters_) {
            const auto movement = state.movement * deltaTime;
            state.movement = JPH::Vec3::sZero();
            auto& character = state.character;
            if (state.maxStepHeight == 0.0f || movement.LengthSq() < 1.0e-8f ||
                character->GetGroundState() != JPH::CharacterBase::EGroundState::OnGround ||
                character->GetLinearVelocity().GetY() - character->GetGroundVelocity().GetY() > 0.1f ||
                std::find(jumpingCharacters_.begin(), jumpingCharacters_.end(), character->GetBodyID()) != jumpingCharacters_.end())
                continue;
            const StepFilter filter(character->GetBodyID());
            const auto position = character->GetPosition();
            const auto cast = [&](JPH::RVec3 start, JPH::Vec3 direction, BlockingHit& hit) {
                const auto transform = JPH::RMat44::sRotationTranslation(character->GetRotation(), start);
                const auto shapeCast = JPH::RShapeCast::sFromWorldTransform(character->GetShape(), JPH::Vec3::sReplicate(1), transform, direction);
                physicsSystem_.GetNarrowPhaseQuery().CastShape(shapeCast, {}, start, hit, {}, {}, filter);
                return hit.HadHit();
            };
            BlockingHit obstacle(movement);
            if (!cast(position + JPH::RVec3(0, 0.02f, 0), movement, obstacle) ||
                !character->IsSlopeTooSteep(-obstacle.mHit.mPenetrationAxis.Normalized()))
                continue;
            const JPH::Vec3 up(0, state.maxStepHeight + 0.02f, 0);
            BlockingHit ceiling(up);
            if (cast(position, up, ceiling))
                continue;
            const auto raised = position + up;
            BlockingHit ahead(movement);
            if (cast(raised, movement, ahead))
                continue;
            const auto down = -up;
            BlockingHit landing(down);
            if (!cast(raised + movement, down, landing) ||
                character->IsSlopeTooSteep(-landing.mHit.mPenetrationAxis.Normalized()))
                continue;
            {
                JPH::BodyLockRead lock(physicsSystem_.GetBodyLockInterface(), landing.mHit.mBodyID2);
                if (!lock.Succeeded() || lock.GetBody().IsDynamic())
                    continue;
            }
            const float rise = up.GetY() * (1.0f - landing.mHit.mFraction);
            if (rise > 0.02f && rise <= state.maxStepHeight)
                character->SetPosition(position + JPH::RVec3(0, rise + 0.001f, 0));
        }
    }

    void PhysicsWorld::UpdateCharacterContacts() {
        for (auto& state : characters_) {
            auto& character = state.character;
            character->PostSimulation(0.05f);
            if (character->GetGroundState() != JPH::CharacterBase::EGroundState::OnGround ||
                character->GetLinearVelocity().GetY() <= character->GetGroundVelocity().GetY()) {
                const auto id = character->GetBodyID();
                jumpingCharacters_.erase(std::remove(jumpingCharacters_.begin(), jumpingCharacters_.end(), id), jumpingCharacters_.end());
            }
        }
    }

    bool PhysicsWorld::CastRay(const Ray& ray, float maxDistance, RayHit& hit, BodyHandle ignoreBody) const {
        hit = {};
        if (ignoreBody != BodyHandle{} && !ContainsBody(ignoreBody))
            return false;
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
        const JPH::IgnoreSingleBodyFilter bodyFilter(ignoreBody.GetJoltID());
        if (!physicsSystem_.GetNarrowPhaseQuery().CastRay(joltRay, result, {}, {}, bodyFilter))
            return false;

        JPH::BodyLockRead lock(physicsSystem_.GetBodyLockInterface(), result.mBodyID);
        if (!lock.Succeeded())
            return false;
        const auto point = joltRay.GetPointOnRay(result.mFraction);
        hit.body = FindBodyHandle(result.mBodyID);
        if (!hit.body.IsValid())
            return false;
        hit.point = Detail::ToRaylibRVector3(point);
        hit.normal = Detail::ToRaylibVector3(
            lock.GetBody().GetWorldSpaceSurfaceNormal(result.mSubShapeID2, point));
        hit.distance = result.mFraction * maxDistance;
        return true;
    }

    BodyHandle PhysicsWorld::CreateConvexHull(const std::vector<Vector3>& points, const ConvexHullDesc& desc) {
        if (points.size() < 4 || points.size() > std::size_t(std::numeric_limits<int>::max()) ||
            !std::isfinite(desc.mass) || desc.mass <= 0.0f ||
            !std::isfinite(desc.friction) || desc.friction < 0.0f ||
            !std::isfinite(desc.restitution) || desc.restitution < 0.0f || desc.restitution > 1.0f ||
            !std::isfinite(desc.position.x) || !std::isfinite(desc.position.y) || !std::isfinite(desc.position.z) ||
            !std::isfinite(desc.rotation.x) || !std::isfinite(desc.rotation.y) ||
            !std::isfinite(desc.rotation.z) || !std::isfinite(desc.rotation.w)) {
            TraceLog(LOG_WARNING, "Invalid convex hull points or settings");
            return BodyHandle{};
        }
        const auto motionType = ToJoltMotionType(desc.motionType);
        const double length = std::sqrt(double(desc.rotation.x) * desc.rotation.x + double(desc.rotation.y) * desc.rotation.y +
            double(desc.rotation.z) * desc.rotation.z + double(desc.rotation.w) * desc.rotation.w);
        if (length == 0.0)
            return BodyHandle{};
        JPH::Array<JPH::Vec3> vertices;
        vertices.reserve(points.size());
        for (const auto& point : points) {
            if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
                return BodyHandle{};
            vertices.push_back(Detail::ToJoltVec3(point));
        }
        JPH::ConvexHullShapeSettings shapeSettings(vertices);
        const auto shape = shapeSettings.Create();
        if (shape.HasError()) {
            TraceLog(LOG_WARNING, "Could not create convex hull: %s", shape.GetError().c_str());
            return BodyHandle{};
        }
        const float shapeMass = shape.Get()->GetMassProperties().mMass;
        if (!std::isfinite(shapeMass) || shapeMass <= 0.0f) {
            TraceLog(LOG_WARNING, "Convex hull must have non-zero volume");
            return BodyHandle{};
        }
        const JPH::Quat rotation(float(desc.rotation.x / length), float(desc.rotation.y / length),
            float(desc.rotation.z / length), float(desc.rotation.w / length));
        JPH::BodyCreationSettings settings(shape.Get().GetPtr(), Detail::ToJoltRVec3(desc.position), rotation,
            motionType, desc.motionType == MotionType::Static ? ObjectLayers::NON_MOVING : ObjectLayers::MOVING);
        settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        settings.mMassPropertiesOverride.mMass = desc.mass;
        settings.mFriction = desc.friction;
        settings.mIsSensor = desc.isSensor;
        settings.mRestitution = desc.restitution;
        return RegisterBody(physicsSystem_.GetBodyInterface().CreateAndAddBody(settings,
            desc.motionType == MotionType::Static ? JPH::EActivation::DontActivate : JPH::EActivation::Activate));
    }

    BodyHandle PhysicsWorld::CreateCompound(const std::vector<CompoundPart>& parts, const CompoundDesc& desc) {
        const auto finite = [](Vector3 value) {
            return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
        };
        const auto normalize = [](Quaternion value, JPH::Quat& result) {
            if (!std::isfinite(value.x) || !std::isfinite(value.y) ||
                !std::isfinite(value.z) || !std::isfinite(value.w))
                return false;
            const double length = std::sqrt(double(value.x) * value.x + double(value.y) * value.y +
                double(value.z) * value.z + double(value.w) * value.w);
            if (length == 0.0)
                return false;
            result = JPH::Quat(float(value.x / length), float(value.y / length),
                float(value.z / length), float(value.w / length));
            return true;
        };
        JPH::Quat rotation;
        if (parts.empty() || parts.size() > std::size_t(std::numeric_limits<int>::max()) ||
            !finite(desc.position) || !normalize(desc.rotation, rotation) ||
            !std::isfinite(desc.mass) || desc.mass <= 0.0f ||
            !std::isfinite(desc.friction) || desc.friction < 0.0f ||
            !std::isfinite(desc.restitution) || desc.restitution < 0.0f || desc.restitution > 1.0f ||
            (desc.motionType != MotionType::Static && desc.motionType != MotionType::Dynamic && desc.motionType != MotionType::Kinematic))
            return BodyHandle{};
        JPH::StaticCompoundShapeSettings compound;
        for (const auto& part : parts) {
            JPH::Quat localRotation;
            if (!finite(part.localPosition) || !normalize(part.localRotation, localRotation))
                return BodyHandle{};
            JPH::ShapeSettings::ShapeResult shape;
            switch (part.type) {
            case CompoundPartType::Box: {
                if (!finite(part.size) || part.size.x <= 0 || part.size.y <= 0 || part.size.z <= 0)
                    return BodyHandle{};
                const auto halfSize = Detail::ToJoltVec3(part.size) * 0.5f;
                shape = JPH::BoxShapeSettings(halfSize, std::min(JPH::cDefaultConvexRadius, halfSize.ReduceMin())).Create();
                break;
            }
            case CompoundPartType::Sphere:
                if (!std::isfinite(part.radius) || part.radius <= 0)
                    return BodyHandle{};
                shape = JPH::SphereShapeSettings(part.radius).Create();
                break;
            case CompoundPartType::Capsule:
                if (!std::isfinite(part.radius) || part.radius <= 0 || !std::isfinite(part.height) ||
                    part.height * 0.5f < part.radius)
                    return BodyHandle{};
                shape = JPH::CapsuleShapeSettings(part.height * 0.5f - part.radius, part.radius).Create();
                break;
            case CompoundPartType::ConvexHull: {
                if (part.points.size() < 4 || part.points.size() > std::size_t(std::numeric_limits<int>::max()))
                    return BodyHandle{};
                JPH::Array<JPH::Vec3> points;
                points.reserve(part.points.size());
                for (const auto& point : part.points) {
                    if (!finite(point))
                        return BodyHandle{};
                    points.push_back(Detail::ToJoltVec3(point));
                }
                shape = JPH::ConvexHullShapeSettings(points).Create();
                break;
            }
            default:
                return BodyHandle{};
            }
            if (shape.HasError()) {
                TraceLog(LOG_WARNING, "Could not create compound part: %s", shape.GetError().c_str());
                return BodyHandle{};
            }
            const float mass = shape.Get()->GetMassProperties().mMass;
            if (!std::isfinite(mass) || mass <= 0)
                return BodyHandle{};
            compound.AddShape(Detail::ToJoltVec3(part.localPosition), localRotation, shape.Get().GetPtr());
        }
        const auto shape = compound.Create();
        if (shape.HasError()) {
            TraceLog(LOG_WARNING, "Could not create compound: %s", shape.GetError().c_str());
            return BodyHandle{};
        }
        const float mass = shape.Get()->GetMassProperties().mMass;
        if (!std::isfinite(mass) || mass <= 0)
            return BodyHandle{};
        JPH::BodyCreationSettings settings(shape.Get().GetPtr(), Detail::ToJoltRVec3(desc.position), rotation,
            ToJoltMotionType(desc.motionType), desc.motionType == MotionType::Static ? ObjectLayers::NON_MOVING : ObjectLayers::MOVING);
        settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        settings.mMassPropertiesOverride.mMass = desc.mass;
        settings.mFriction = desc.friction;
        settings.mRestitution = desc.restitution;
        settings.mIsSensor = desc.isSensor;
        return RegisterBody(physicsSystem_.GetBodyInterface().CreateAndAddBody(settings,
            desc.motionType == MotionType::Static ? JPH::EActivation::DontActivate : JPH::EActivation::Activate));
    }

    BodyHandle PhysicsWorld::CreateStaticMesh(const TriangleMeshData& mesh, const StaticMeshDesc& desc) {
        const auto validation = ValidateTriangleMesh(mesh);
        if (!validation.valid) {
            TraceLog(LOG_WARNING, "Invalid triangle mesh: %s", validation.message.c_str());
            return BodyHandle{};
        }
        
        if (!std::isfinite(desc.position.x) || !std::isfinite(desc.position.y) || !std::isfinite(desc.position.z) ||
            !std::isfinite(desc.scale.x) || !std::isfinite(desc.scale.y) || !std::isfinite(desc.scale.z) ||
            desc.scale.x == 0.0f || desc.scale.y == 0.0f || desc.scale.z == 0.0f ||
            !std::isfinite(desc.rotation.x) || !std::isfinite(desc.rotation.y) ||
            !std::isfinite(desc.rotation.z) || !std::isfinite(desc.rotation.w)) {
            TraceLog(LOG_WARNING, "Invalid mesh transform: values must be finite and scale must be non-zero");
            return BodyHandle{};
        }
        const double rotationLength = std::sqrt(double(desc.rotation.x) * desc.rotation.x +
            double(desc.rotation.y) * desc.rotation.y + double(desc.rotation.z) * desc.rotation.z +
            double(desc.rotation.w) * desc.rotation.w);
        if (rotationLength == 0.0) {
            TraceLog(LOG_WARNING, "Invalid mesh transform: rotation cannot be a zero quaternion");
            return BodyHandle{};
        }
        TriangleMeshData scaledMesh = mesh;
        for (auto& vertex : scaledMesh.vertices) {
            const double x = double(vertex.x) * desc.scale.x;
            const double y = double(vertex.y) * desc.scale.y;
            const double z = double(vertex.z) * desc.scale.z;
            const double limit = std::numeric_limits<float>::max();
            if (std::abs(x) > limit || std::abs(y) > limit || std::abs(z) > limit) {
                TraceLog(LOG_WARNING, "Invalid mesh transform: scaled vertex exceeds float range");
                return BodyHandle{};
            }
            vertex = {float(x), float(y), float(z)};
        }
        const auto scaledValidation = ValidateTriangleMesh(scaledMesh);
        if (!scaledValidation.valid) {
            TraceLog(LOG_WARNING, "Invalid scaled triangle mesh: %s", scaledValidation.message.c_str());
            return BodyHandle{};
        }

        JPH::VertexList vertices;
        vertices.reserve(mesh.vertices.size()); 
        for (const auto& vertex : scaledMesh.vertices)
            vertices.emplace_back(vertex.x, vertex.y, vertex.z);

        JPH::IndexedTriangleList triangles;
        triangles.reserve(mesh.indices.size() / 3);
        const bool mirrored = (desc.scale.x < 0.0f) ^ (desc.scale.y < 0.0f) ^ (desc.scale.z < 0.0f);
        const bool reverseWinding = mirrored != desc.flipWinding;
        for (std::size_t i = 0; i < mesh.indices.size(); i += 3) {
            if (reverseWinding)
                triangles.emplace_back(mesh.indices[i], mesh.indices[i + 2], mesh.indices[i + 1], 0);
            else
                triangles.emplace_back(mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2], 0);
        }

        JPH::MeshShapeSettings shapeSettings(std::move(vertices), std::move(triangles));
        const auto shape = shapeSettings.Create();
        if (shape.HasError()) {
            TraceLog(LOG_WARNING, "Could not create triangle mesh: %s", shape.GetError().c_str());
            return BodyHandle{};
        }
        const JPH::Quat rotation(float(desc.rotation.x / rotationLength), float(desc.rotation.y / rotationLength),
            float(desc.rotation.z / rotationLength), float(desc.rotation.w / rotationLength));
        JPH::BodyCreationSettings settings(shape.Get().GetPtr(), Detail::ToJoltRVec3(desc.position),
            rotation, JPH::EMotionType::Static, ObjectLayers::NON_MOVING);
        settings.mIsSensor = desc.isSensor;
        return RegisterBody(physicsSystem_.GetBodyInterface().CreateAndAddBody(settings, JPH::EActivation::DontActivate));
    }

    BodyHandle PhysicsWorld::CreateBox(const BoxDesc& desc) {
        //put int halfExtents
        JPH::Vec3 halfExtents(desc.size.x * 0.5f, desc.size.y * 0.5f, desc.size.z * 0.5f);

        //Set motionType / Object of layer
        JPH::EMotionType motionType = ToJoltMotionType(desc.motionType);
        JPH::ObjectLayer layer = desc.motionType == MotionType::Static ? ObjectLayers::NON_MOVING : ObjectLayers::MOVING;

        //setting
        JPH::BodyCreationSettings settings(
            new JPH::BoxShape(halfExtents),
            Detail::ToJoltRVec3(desc.position),
            Detail::ToJoltQuaternion(desc.rotation),
            motionType,
            layer
        );

        settings.mIsSensor = desc.isSensor;
        settings.mRestitution = desc.restitution;
        settings.mFriction = desc.friction;

        JPH::EActivation activation = desc.motionType == MotionType::Static ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;

        JPH::BodyID jphId = physicsSystem_.GetBodyInterface().CreateAndAddBody(settings, activation);
        return RegisterBody(jphId);
    }

    //====================End declare Struct/Class INFO====================

    JPH::EPhysicsUpdateError JoltRuntime::Update(JPH::PhysicsSystem& physics, float timeStep, int collisionSteps)
    {
        if (!state_)
            throw std::logic_error("JoltRuntime::Init() must succeed before Update()");
        if (!std::isfinite(timeStep) || timeStep <= 0 || collisionSteps <= 0)
            throw std::invalid_argument("Time step and collision steps must be positive");
        std::lock_guard<std::mutex> lock(state_->updateMutex);
        return state_->Update(physics, timeStep, collisionSteps);
    }

    JPH::EPhysicsUpdateError JoltRuntime::Step(PhysicsWorld& world, float deltaTime, int collisionSteps) {
        if (!state_)
            throw std::logic_error("JoltRuntime::Init() must succeed before Step()");
        if (!world.initialized_ || world.runtimeState_ != state_)
            throw std::invalid_argument("World does not belong to this Runtime");
        if (!std::isfinite(deltaTime) || deltaTime <= 0 || collisionSteps <= 0)
            throw std::invalid_argument("Time step and collision steps must be positive");
        std::lock_guard<std::mutex> lock(state_->updateMutex);
        world.contactListener_.Clear();
        world.UpdateCharacterSteps(deltaTime);
        const auto result = state_->Update(world.GetPhysicsSystem(), deltaTime, collisionSteps);
        world.UpdateCharacterContacts();
        return result;
    }

    bool RayJoltContext::Init() {
        if (world_)
            return true;
        if (!runtime_.Init())
            return false;

        try {
            auto world = runtime_.CreateWorld();
            if (!world) {
                world.reset();
                runtime_.End();
                return false;
            }
            world_ = std::move(world);
        } catch (...) {
            runtime_.End();
            throw;
        }
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
        return runtime_.Step(*world_, deltaTime, collisionSteps);
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
        return g_joltRuntime.Step(physics, timeStep, collisionSteps);
    }

    void RJ_OptimizeBroadPhase(PhysicsWorld& physics) {
        physics.OptimizeBroadPhase();
    }

}
