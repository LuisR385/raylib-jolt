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


namespace RayJolt{
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


	}


	void RayJoltVersion();

}

//alias
namespace RJ = RayJolt;
