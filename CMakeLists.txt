cmake_minimum_required(VERSION 3.20)
project(rayjolt VERSION 0.1.0 LANGUAGES C CXX)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# Keep Jolt and raylib on the same MSVC runtime and retain CMake's compiler flags.
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
set(OVERRIDE_CXX_FLAGS OFF CACHE BOOL "" FORCE)
set(INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
# This rigid-body example does not require GPU compute SDKs.
set(JPH_USE_DX12 OFF CACHE BOOL "" FORCE)
set(JPH_USE_VK OFF CACHE BOOL "" FORCE)
set(JPH_USE_MTL OFF CACHE BOOL "" FORCE)

# Official release sources: raylib 6.0 and Jolt Physics v5.6.0.
add_subdirectory(external/raylib EXCLUDE_FROM_ALL)
add_subdirectory(external/JoltPhysics/Build EXCLUDE_FROM_ALL)

# integration release source.


add_executable(rayjolt src/main.cpp)
target_include_directories(rayjolt PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(rayjolt PRIVATE cxx_std_17)
target_link_libraries(rayjolt PRIVATE raylib Jolt)
