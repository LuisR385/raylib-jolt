#pragma once

//TODO : API時に使用するため今はまだexperimental
//inline constexpr int RAYJOLT_VERSION_MINOR 0;
//inline constexpr int RAYJOLT_VERSION_MAJOR 0;
//inline constexpr int RAYJOLT_VERSION_PATCH 0;
//inline constexpr const char* RAYJOLT_VERSION_STRING = nullptr;

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
#include <Jolt/Physics/Collision/RayCast.h>
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

namespace RayJolt{
	inline static constexpr int  RAYJOLT_VERSION_MAJOR = 0;
	inline static constexpr int  RAYJOLT_VERSION_MINOR = 1;
	inline static constexpr int  RAYJOLT_VERSION_PATCH = 0;
	inline constexpr const char* RAYJOLT_VERSION = "0.1.0";
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

	struct BoxDesc {
	public:
        Vector3 size = { 1.0f, 1.0f, 1.0f };
        Vector3 position = { 0.0f, 0.0f, 0.0f };
        Quaternion rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
        bool isStatic = false; // trueなら静的、falseなら動的
        float restitution = 0.2f;
        float friction = 0.2f;


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
		Vector3 size = { 1.0f, 1.0f, 1.0f };
		Vector3 position = { 0.0f, 0.0f, 0.0f };
		Quaternion rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
		bool isStatic = false; // trueなら静的、falseなら動的
		float restitution = 0.2f;
		float friction = 0.2f;


		SphereDesc() = default;
		//allow copy/move. just data.
		SphereDesc(const SphereDesc&) = default;
		SphereDesc& operator=(const SphereDesc&) = default;
		SphereDesc(SphereDesc&&) noexcept = default;
		SphereDesc& operator=(SphereDesc&&) noexcept = default;

	};


	struct BodyHandle {
	public:
		BodyHandle() = default;
		explicit BodyHandle(JPH::BodyID jphId) : jphId_(jphId) {}

		bool IsValid() const { return !jphId_.IsInvalid(); }

		// 内部処理用（ユーザーには直接触らせない）
		// NOTE : Only internal processing. dont use it now.
		JPH::BodyID GetJoltID() const { return jphId_; }

	private:
		JPH::BodyID jphId_;
	};


	struct PhysicsBody {

	};


	//raw initialize jolt.
	class JoltRuntime {
	public:
		JoltRuntime() = default;
		~JoltRuntime();

		bool Init();
		void End();
		JPH::EPhysicsUpdateError Update(JPH::PhysicsSystem& physics, float timeStep, int collisionSteps);

		//Advanced Api.

		const std::unique_ptr<JPH::JobSystemThreadPool>& GetJobSystem()const {
			return jobSystem_;
		}

		const std::unique_ptr<JPH::TempAllocatorImpl>& GetTempAllocator()const {
			return tempAllocator_;
		}

		JoltRuntime(const JoltRuntime&) = delete;
		JoltRuntime& operator=(const JoltRuntime&) = delete;
	private:
		bool isInitialized = false;
		std::unique_ptr<JPH::JobSystemThreadPool> jobSystem_ = nullptr;
		std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator_ = nullptr;
	};




	class PhysicsWorld {
	public:
		~PhysicsWorld();

		bool Init();
		void Update(float deltaTime,int collisionStep = 1);
		void Shutdown();

		void OptimizeBroadPhase() {
			physicsSystem_.OptimizeBroadPhase();
		}


		BodyHandle CreateBox(const BoxDesc& desc = {});
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
