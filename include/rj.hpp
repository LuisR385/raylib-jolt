#pragma once



//Jolt headers.
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Constraints/TwoBodyConstraint.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterTable.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h>
#include <Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h>

//raylib
#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <thread>
#include <memory>
#include <vector>
#include <cstdint>
#include <string>
#include <mutex>
#include <unordered_map>

#ifdef RJ_WITH_RFBX
struct RfbxModel;
struct RfbxInstance;
#endif

namespace RayJolt{
	inline static constexpr int  RAYJOLT_VERSION_MAJOR = 0;
	inline static constexpr int  RAYJOLT_VERSION_MINOR = 4;
	inline static constexpr int  RAYJOLT_VERSION_PATCH = 0;
	inline constexpr const char* RAYJOLT_VERSION = "0.4.0";
	inline constexpr float		 timeStep = 1.0f / 60.0f;

	namespace ObjectLayers
	{
		constexpr JPH::ObjectLayer NON_MOVING = 0;
		constexpr JPH::ObjectLayer MOVING = 1;
	}

	namespace BroadPhaseLayers
	{
		constexpr JPH::BroadPhaseLayer NON_MOVING(0);
		constexpr JPH::BroadPhaseLayer MOVING(1);
	}


	//calculate
	namespace Detail {




		//=====JOLT->RAYLIB================================

		//NOTE : maybe dont use
		inline ::Vector2 ToRaylibVector2(const JPH::RVec3Arg& value) {
			return { value.GetX(),value.GetY() };
		}

		inline Vector3 ToRaylibVector3(const JPH::Vec3& value) {
			return { value.GetX(), value.GetY(), value.GetZ() };
		}

		inline Vector3 ToRaylibRVector3(const JPH::RVec3& value) {
			return { static_cast<float>(value.GetX()), static_cast<float>(value.GetY()), static_cast<float>(value.GetZ()) };
		}

		inline Vector4 ToRaylibVector4(const JPH::Vec4& value) {
			return { value.GetX(), value.GetY(), value.GetZ(), value.GetW() };
		}

		inline Quaternion ToRaylibQuaternion(const JPH::Quat& value) {
			return { value.GetX(), value.GetY(), value.GetZ(), value.GetW() };
		}

		//NOTE : Mat44 is use Transform.

		inline ::Matrix ToRaylibMatrix(const JPH::Mat44& inJoltMat)
		{
			::Matrix outMat{};

			const JPH::Vec4 col0 = inJoltMat.GetColumn4(0);
			const JPH::Vec4 col1 = inJoltMat.GetColumn4(1);
			const JPH::Vec4 col2 = inJoltMat.GetColumn4(2);
			const JPH::Vec4 col3 = inJoltMat.GetColumn4(3);

			outMat.m0 = col0.GetX();
			outMat.m1 = col0.GetY();
			outMat.m2 = col0.GetZ();
			outMat.m3 = col0.GetW();

			outMat.m4 = col1.GetX();
			outMat.m5 = col1.GetY();
			outMat.m6 = col1.GetZ();
			outMat.m7 = col1.GetW();

			outMat.m8 = col2.GetX();
			outMat.m9 = col2.GetY();
			outMat.m10 = col2.GetZ();
			outMat.m11 = col2.GetW();

			outMat.m12 = col3.GetX();
			outMat.m13 = col3.GetY();
			outMat.m14 = col3.GetZ();
			outMat.m15 = col3.GetW();

			return outMat;
		}


		inline ::Matrix ToRaylibRMatrix(const JPH::RMat44& inJoltMat)
		{
			::Matrix outMat{};

			const JPH::Vec4 col0 = inJoltMat.GetColumn4(0);
			const JPH::Vec4 col1 = inJoltMat.GetColumn4(1);
			const JPH::Vec4 col2 = inJoltMat.GetColumn4(2);
			const JPH::RVec3 translation = inJoltMat.GetTranslation();

			outMat.m0 = col0.GetX();
			outMat.m1 = col0.GetY();
			outMat.m2 = col0.GetZ();
			outMat.m3 = col0.GetW();

			outMat.m4 = col1.GetX();
			outMat.m5 = col1.GetY();
			outMat.m6 = col1.GetZ();
			outMat.m7 = col1.GetW();

			outMat.m8 = col2.GetX();
			outMat.m9 = col2.GetY();
			outMat.m10 = col2.GetZ();
			outMat.m11 = col2.GetW();

			outMat.m12 = static_cast<float>(translation.GetX());
			outMat.m13 = static_cast<float>(translation.GetY());
			outMat.m14 = static_cast<float>(translation.GetZ());

			outMat.m15 = 1.0f;

			return outMat;
		}

		inline BoundingBox ToRaylibBoundingBox(const JPH::AABox box) {
			return {
				ToRaylibVector3(box.mMin),
				ToRaylibVector3(box.mMax)
			};
		}


		//==================================================

		//===========RAYLIB->JOLT============================

		inline JPH::Vec4 ToJoltVec4(const Vector4& value) {
			return { value.x,value.y,value.z,value.w };
		}

		inline JPH::Vec3 ToJoltVec3(const Vector3& value) {
			return { value.x, value.y, value.z };
		}

		inline JPH::RVec3 ToJoltRVec3(const Vector3& value) {
			return { value.x, value.y, value.z };
		}

		inline JPH::Quat ToJoltQuaternion(const Quaternion& value) {
			return { value.x, value.y, value.z, value.w };
		}

		inline JPH::RMat44 ToJoltRMat44(const ::Matrix& inRaylibMat)
		{
			const JPH::Vec4 col0(
				inRaylibMat.m0,
				inRaylibMat.m1,
				inRaylibMat.m2,
				inRaylibMat.m3
			);

			const JPH::Vec4 col1(
				inRaylibMat.m4,
				inRaylibMat.m5,
				inRaylibMat.m6,
				inRaylibMat.m7
			);

			const JPH::Vec4 col2(
				inRaylibMat.m8,
				inRaylibMat.m9,
				inRaylibMat.m10,
				inRaylibMat.m11
			);

			const JPH::RVec3 translation(
				static_cast<JPH::Real>(inRaylibMat.m12),
				static_cast<JPH::Real>(inRaylibMat.m13),
				static_cast<JPH::Real>(inRaylibMat.m14)
			);

			return JPH::RMat44(
				col0,
				col1,
				col2,
				translation
			);
		}

		inline JPH::Mat44 ToJoltMat44(const ::Matrix& inRaylibMat)
		{
			const JPH::Vec4 col0(
				inRaylibMat.m0,
				inRaylibMat.m1,
				inRaylibMat.m2,
				inRaylibMat.m3
			);

			const JPH::Vec4 col1(
				inRaylibMat.m4,
				inRaylibMat.m5,
				inRaylibMat.m6,
				inRaylibMat.m7
			);

			const JPH::Vec4 col2(
				inRaylibMat.m8,
				inRaylibMat.m9,
				inRaylibMat.m10,
				inRaylibMat.m11
			);

			const JPH::Vec3 translation(
				static_cast<JPH::Real>(inRaylibMat.m12),
				static_cast<JPH::Real>(inRaylibMat.m13),
				static_cast<JPH::Real>(inRaylibMat.m14)
			);

			return JPH::Mat44(
				col0,
				col1,
				col2,
				translation
			);
		}

		// NOTE : Jolt ray direction is "direction * scaler(distance)" = Jolt RayCast.
		inline JPH::RayCast ToJolt(const Ray& ray, float maxDistance = 1000.0f) {
			JPH::Vec3 origin(ray.position.x, ray.position.y, ray.position.z);
			JPH::Vec3 direction(ray.direction.x * maxDistance, ray.direction.y * maxDistance, ray.direction.z * maxDistance);
			return JPH::RayCast(origin, direction);
		}

		//===================================================


	} //!Detail



	//====================Start declare Struct/Class====================

	enum class MotionType {
		Static,
		Dynamic,
		Kinematic
	};

	struct TriangleMeshData {
		std::vector<Vector3> vertices;
		std::vector<std::uint32_t> indices;
	};

	struct StaticMeshDesc {
		Vector3 position = {0.0f, 0.0f, 0.0f};
		Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f};
		Vector3 scale = {1.0f, 1.0f, 1.0f};
		bool flipWinding = false;
		bool isSensor = false;
	};

	struct MeshValidationResult {
		bool valid = false;
		std::string message;
	};

	struct ConvexHullDesc {
		Vector3 position = {0.0f, 0.0f, 0.0f};
		Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f};
		MotionType motionType = MotionType::Dynamic;
		float mass = 1.0f;
		float friction = 0.2f;
		float restitution = 0.2f;
		bool isSensor = false;
	};

	MeshValidationResult ValidateTriangleMesh(const TriangleMeshData& mesh);
#ifdef RJ_WITH_RFBX
	MeshValidationResult BuildRfbxCollisionMesh(const RfbxModel& model, TriangleMeshData& output,
		Matrix transform = MatrixIdentity());
#endif

	struct BoxDesc {
	public:
        Vector3 size = { 1.0f, 1.0f, 1.0f };
        Vector3 position = { 0.0f, 0.0f, 0.0f };
        Quaternion rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
        MotionType motionType = MotionType::Dynamic;
        float restitution = 0.2f;
        float friction = 0.2f;
		bool isSensor = false;


		BoxDesc() = default;
		//NOTE : BoxDesc is just Data so allow copy/move.
		//allow to copy.
		BoxDesc(const BoxDesc&) = default;
		BoxDesc& operator=(const BoxDesc&) = default;
		//allow to move.
		BoxDesc(BoxDesc&&) noexcept = default;
		BoxDesc& operator=(BoxDesc&&) noexcept = default;

	};

	struct SphereDesc {
	public:
		float radius = 0.5f;
		Vector3 position = { 0.0f, 0.0f, 0.0f };
		Quaternion rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
		MotionType motionType = MotionType::Dynamic;
		float restitution = 0.2f;
		float friction = 0.2f;
		bool isSensor = false;


		SphereDesc() = default;
		//allow copy/move. just data.
		SphereDesc(const SphereDesc&) = default;
		SphereDesc& operator=(const SphereDesc&) = default;
		SphereDesc(SphereDesc&&) noexcept = default;
		SphereDesc& operator=(SphereDesc&&) noexcept = default;

	};


	struct CapsuleDesc {
		float radius = 0.3f;
		float height = 1.8f;
		Vector3 position = { 0.0f, 0.0f, 0.0f };
		Quaternion rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
		MotionType motionType = MotionType::Dynamic;
		float restitution = 0.2f;
		float friction = 0.2f;
		bool isSensor = false;
	};

	enum class CompoundPartType {
		Box,
		Sphere,
		Capsule,
		ConvexHull
	};

	struct CompoundPart {
		CompoundPartType type = CompoundPartType::Box;
		Vector3 localPosition = {};
		Quaternion localRotation = {0, 0, 0, 1};
		Vector3 size = {1, 1, 1};
		float radius = 0.3f;
		float height = 1.8f;
		std::vector<Vector3> points;
	};

	struct CompoundDesc {
		Vector3 position = {};
		Quaternion rotation = {0, 0, 0, 1};
		MotionType motionType = MotionType::Dynamic;
		float mass = 1.0f;
		float friction = 0.2f;
		float restitution = 0.2f;
		bool isSensor = false;
	};

	struct CharacterDesc {
		float radius = 0.3f;
		float height = 1.8f;
		float mass = 80.0f;
		float maxSlopeAngleDegrees = 50.0f;
		Vector3 position = { 0.0f, 0.0f, 0.0f };
		float maxStepHeight = 0.0f;
	};

	struct BodyHandle {
	public:
		BodyHandle() = default;

		bool IsValid() const { return worldId_ != 0 && generation_ != 0 && !jphId_.IsInvalid(); }
		bool operator==(const BodyHandle& other) const {
			return worldId_ == other.worldId_ && generation_ == other.generation_ && jphId_ == other.jphId_;
		}
		bool operator!=(const BodyHandle& other) const { return !(*this == other); }

		// 内部処理用（ユーザーには直接触らせない）
		// NOTE : Only internal processing. dont use it now.
		JPH::BodyID GetJoltID() const { return jphId_; }

	private:
		friend class PhysicsWorld;
		BodyHandle(JPH::BodyID id, std::uint64_t worldId, std::uint64_t generation)
			: jphId_(id), worldId_(worldId), generation_(generation) {}
		JPH::BodyID jphId_;
		std::uint64_t worldId_ = 0;
		std::uint64_t generation_ = 0;
	};


	struct RagdollPartDesc {
		CompoundPart shape;
		int parent = -1;
		float mass = 1.0f;
		Vector3 jointPosition = {};
		Quaternion jointRotation = {0, 0, 0, 1};
		float swingAngleDegrees = 45.0f;
		float twistMinDegrees = -30.0f;
		float twistMaxDegrees = 30.0f;
	};

	struct RagdollDesc {
		Vector3 position = {};
		Quaternion rotation = {0, 0, 0, 1};
		float friction = 0.5f;
		float restitution = 0.0f;
		bool selfCollision = false;
	};

	struct RagdollHandle {
		bool IsValid() const { return root_.IsValid(); }
		bool operator==(const RagdollHandle& other) const { return root_ == other.root_; }
		bool operator!=(const RagdollHandle& other) const { return !(*this == other); }
	private:
		friend class PhysicsWorld;
		BodyHandle root_;
	};

#ifdef RJ_WITH_RFBX
	enum class BoneColliderType { Box, Capsule };

	struct RfbxBoneColliderDesc {
		std::string boneName;
		BoneColliderType type = BoneColliderType::Capsule;
		Vector3 localPosition = {};
		Quaternion localRotation = {0, 0, 0, 1};
		Vector3 size = {0.2f, 0.4f, 0.2f};
		float radius = 0.1f;
		float height = 0.4f;
		bool isSensor = true;
	};

	struct RfbxBoneCollider {
		BodyHandle body;
		std::string boneName;
		Matrix localTransform = MatrixIdentity();
		float scale = 1.0f;
	};
#endif

	enum class ContactEventType {
		Begin,
		Stay,
		End
	};

	struct ContactEvent {
		ContactEventType type = ContactEventType::Begin;
		BodyHandle bodyA;
		BodyHandle bodyB;
		std::uint32_t subShapeA = 0;
		std::uint32_t subShapeB = 0;
		Vector3 point = {};
		Vector3 normal = {};
		bool hasContactPoint = false;
	};

	enum class GroundState {
		OnGround,
		OnSteepGround,
		NotSupported,
		InAir
	};

	struct CharacterGroundInfo {
		GroundState state = GroundState::InAir;
		BodyHandle body;
		Vector3 position = {};
		Vector3 normal = {};
		Vector3 velocity = {};
	};

	struct RayHit {
		BodyHandle body;
		Vector3 point = {};
		Vector3 normal = {};
		float distance = 0.0f;
	};

	struct PhysicsBody {

	};


	//raw initialize jolt.
	class PhysicsWorld;

	class JoltRuntime {
	public:
		JoltRuntime() = default;
		~JoltRuntime();

		bool Init();
		void End();
		std::unique_ptr<PhysicsWorld> CreateWorld();
		JPH::EPhysicsUpdateError Step(PhysicsWorld& world, float deltaTime, int collisionSteps = 1);
		JPH::EPhysicsUpdateError Update(JPH::PhysicsSystem& physics, float timeStep, int collisionSteps);

		//Advanced Api.

		const std::unique_ptr<JPH::JobSystemThreadPool>& GetJobSystem() const;
		const std::unique_ptr<JPH::TempAllocatorImpl>& GetTempAllocator() const;

		JoltRuntime(const JoltRuntime&) = delete;
		JoltRuntime& operator=(const JoltRuntime&) = delete;
	private:
		friend class PhysicsWorld;
		struct State;
		std::shared_ptr<State> state_;
		static std::weak_ptr<State> activeState_;
	};




	class PhysicsWorld {
	public:
		PhysicsWorld();
		~PhysicsWorld();
		PhysicsWorld(const PhysicsWorld&) = delete;
		PhysicsWorld& operator=(const PhysicsWorld&) = delete;

		bool Init();
		void Update(float deltaTime,int collisionStep = 1);
		void Shutdown();
		std::vector<ContactEvent> GetContactEvents() const;
		bool IsSensor(BodyHandle body) const;
		void SetGravity(Vector3 gravity);
		Vector3 GetGravity() const;
		void SetGravityFactor(BodyHandle body, float factor);
		float GetGravityFactor(BodyHandle body) const;
		void SetFriction(BodyHandle body, float friction);
		float GetFriction(BodyHandle body) const;
		void SetRestitution(BodyHandle body, float restitution);
		float GetRestitution(BodyHandle body) const;
		void SetMass(BodyHandle body, float mass);
		float GetMass(BodyHandle body) const;
		void SetLinearDamping(BodyHandle body, float damping);
		float GetLinearDamping(BodyHandle body) const;
		void SetAngularDamping(BodyHandle body, float damping);
		float GetAngularDamping(BodyHandle body) const;
		void WakeBody(BodyHandle body);
		void SleepBody(BodyHandle body);
		bool IsActive(BodyHandle body) const;
		void SetContinuousCollision(BodyHandle body, bool enabled);
		bool IsContinuousCollisionEnabled(BodyHandle body) const;

		void OptimizeBroadPhase() {
			physicsSystem_.OptimizeBroadPhase();
		}


		BodyHandle CreateBox(const BoxDesc& desc = {});
		BodyHandle CreateStaticMesh(const TriangleMeshData& mesh, const StaticMeshDesc& desc = {});
		BodyHandle CreateConvexHull(const std::vector<Vector3>& points, const ConvexHullDesc& desc = {});
		BodyHandle CreateCompound(const std::vector<CompoundPart>& parts, const CompoundDesc& desc = {});
		RagdollHandle CreateRagdoll(const std::vector<RagdollPartDesc>& parts, const RagdollDesc& desc = {});
		bool ContainsRagdoll(RagdollHandle ragdoll) const;
		std::vector<BodyHandle> GetRagdollBodies(RagdollHandle ragdoll) const;
		bool DestroyRagdoll(RagdollHandle& ragdoll);
#ifdef RJ_WITH_RFBX
		RfbxBoneCollider CreateRfbxBoneCollider(const RfbxInstance& instance, const RfbxBoneColliderDesc& desc,
			Matrix modelTransform = MatrixIdentity());
		bool UpdateRfbxBoneCollider(const RfbxBoneCollider& collider, const RfbxInstance& instance,
			float deltaTime, Matrix modelTransform = MatrixIdentity());
#endif
		BodyHandle CreateSphere(const SphereDesc& desc = {});
		BodyHandle CreateCapsule(const CapsuleDesc& desc = {});
		BodyHandle CreateCharacter(const CharacterDesc& desc = {});
		bool IsGrounded(BodyHandle character) const;
		CharacterGroundInfo GetCharacterGroundInfo(BodyHandle character) const;
		bool Jump(BodyHandle character, float speed = 5.0f);
		void MoveCharacter(BodyHandle character, Vector2 direction, float speed = 3.0f);
		bool SetCrouching(BodyHandle character, bool crouching);
		bool IsCrouching(BodyHandle character) const;
		float GetCharacterHeight(BodyHandle character) const;
		Vector3 GetPosition(BodyHandle body) const;
		void SetPosition(BodyHandle body, Vector3 value);
		void MoveKinematic(BodyHandle body, Vector3 position, Quaternion rotation, float deltaTime);
		Quaternion GetRotation(BodyHandle body) const;
		Matrix GetTransform(BodyHandle body) const;
		BoundingBox GetBoundingBox(BodyHandle body) const;
		void SetRotation(BodyHandle body, Quaternion value);
		Vector3 GetLinearVelocity(BodyHandle body) const;
		void SetLinearVelocity(BodyHandle body, Vector3 value);
		Vector3 GetAngularVelocity(BodyHandle body) const;
		void SetAngularVelocity(BodyHandle body, Vector3 value);
		void AddForce(BodyHandle body, Vector3 force);
		void AddForce(BodyHandle body, Vector3 force, Vector3 worldPosition);
		void AddImpulse(BodyHandle body, Vector3 impulse);
		void AddImpulse(BodyHandle body, Vector3 impulse, Vector3 worldPosition);
		void AddTorque(BodyHandle body, Vector3 torque);
		void AddAngularImpulse(BodyHandle body, Vector3 impulse);
		bool CastRay(const Ray& ray, float maxDistance, RayHit& hit, BodyHandle ignoreBody = {}) const;
		bool ContainsBody(BodyHandle body) const;
		bool DestroyBody(BodyHandle& body);


		JPH::BodyInterface& GetbodyInterface() {
			return physicsSystem_.GetBodyInterface();
		}

		const JPH::BodyInterface& GetbodyInterface() const {
			return physicsSystem_.GetBodyInterface();
		}

		//Advanced API.
		JPH::PhysicsSystem& GetPhysicsSystem() {
			return physicsSystem_;
		}

		const JPH::PhysicsSystem& GetPhysicsSystem()const {
			return physicsSystem_;
		}




	private:
		friend class JoltRuntime;
		friend class RayJoltContext;
		friend JPH::EPhysicsUpdateError RJ_Update(PhysicsWorld&, float, int);
		BodyHandle RegisterBody(JPH::BodyID id);
		BodyHandle FindBodyHandle(JPH::BodyID id) const;
		std::shared_ptr<JoltRuntime::State> runtimeState_;
		const std::uint64_t worldId_;
		std::uint64_t nextBodyGeneration_ = 1;
		std::unordered_map<std::uint32_t, std::uint64_t> bodyGenerations_;
		bool initialized_ = false;
		struct RagdollState {
			RagdollHandle handle;
			std::vector<BodyHandle> bodies;
			std::vector<JPH::Ref<JPH::TwoBodyConstraint>> constraints;
		};
		std::vector<RagdollState> ragdolls_;
		std::uint32_t nextRagdollGroup_ = 0;
		void UpdateCharacterContacts();
		void UpdateCharacterSteps(float deltaTime);
		struct CharacterState {
			JPH::Ref<JPH::Character> character;
			JPH::RefConst<JPH::Shape> standingShape;
			JPH::RefConst<JPH::Shape> crouchingShape;
			float standingHeight;
			float crouchingHeight;
			float radius;
			bool crouching = false;
			JPH::Vec3 groundVelocity = JPH::Vec3::sZero();
			float maxStepHeight = 0.0f;
			JPH::Vec3 movement = JPH::Vec3::sZero();
		};
		std::vector<CharacterState> characters_;
		std::vector<JPH::BodyID> jumpingCharacters_;
		class ContactEventListener final : public JPH::ContactListener {
		public:
			explicit ContactEventListener(PhysicsWorld& world) : world_(world) {}
			void OnContactAdded(const JPH::Body& a, const JPH::Body& b,
				const JPH::ContactManifold& manifold, JPH::ContactSettings&) override;
			void OnContactPersisted(const JPH::Body& a, const JPH::Body& b,
				const JPH::ContactManifold& manifold, JPH::ContactSettings&) override;
			void OnContactRemoved(const JPH::SubShapeIDPair& pair) override;
			void Clear();
			void Reset();
			std::vector<ContactEvent> GetEvents() const;
		private:
			void Record(ContactEventType type, const JPH::Body& a, const JPH::Body& b,
				const JPH::ContactManifold& manifold);
			mutable std::mutex mutex_;
			std::vector<ContactEvent> events_;
			PhysicsWorld& world_;
			std::unordered_map<JPH::SubShapeIDPair, std::pair<BodyHandle, BodyHandle>> contacts_;
		};
		ContactEventListener contactListener_{*this};
		JPH::ObjectLayerPairFilterTable layerPairs_{2};
		JPH::BroadPhaseLayerInterfaceTable broadPhaseLayers_{2, 2};
		std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilterTable> broadPhaseFilter_;
		JPH::PhysicsSystem physicsSystem_;
		std::vector<PhysicsBody> physicsbodies_;
	};

	class RayJoltContext {
	public:
		bool Init();
		void End();
		JPH::EPhysicsUpdateError Step(float deltaTime, int collisionSteps = 1);
		PhysicsWorld& GetWorld();

	private:
		JoltRuntime runtime_;
		std::unique_ptr<PhysicsWorld> world_;
	};

	//====================End declare Struct/Class====================
	const char* RJ_Version();

	bool RJ_Initialize();
	void RJ_RegisterDefaultAllocator();
	void RJ_End();
	JPH::EPhysicsUpdateError RJ_Update(PhysicsWorld& physics, float timeStep, int collisionSteps = 1);
	void RJ_OptimizeBroadPhase(PhysicsWorld& physics);

}

//alias
namespace RJ = RayJolt;
