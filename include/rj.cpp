#include "rj.hpp"

namespace RayJolt {

    JoltRuntime::JoltRuntime()
    {
        // 同じオブジェクトでの二重初期化を防ぐ
        if (factory_)
            return;

        JPH::RegisterDefaultAllocator();

        factory_ = std::make_unique<JPH::Factory>();
        JPH::Factory::sInstance = factory_.get(); //owner is factory_.

        JPH::RegisterTypes();
    }

    JoltRuntime::~JoltRuntime()
    {
        // 初期化していなければ、片付けも不要
        if (!factory_)
            return;

        JPH::UnregisterTypes();
        JPH::Factory::sInstance = nullptr;
    }
}
