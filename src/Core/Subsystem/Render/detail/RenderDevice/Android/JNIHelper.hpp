/**
 * @file
 * @date 2023/8/19
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <string>
#include <memory>
#include <lstg/Core/Result.hpp>

struct AAssetManager;

struct _JNIEnv;
struct _jobject;

namespace lstg::Subsystem::Render::detail::RenderDevice::Android
{
#ifdef LSTG_PLATFORM_ANDROID
    class JNIHelper
    {
    public:
        class GlobalReference
        {
        public:
            GlobalReference() = default;
            GlobalReference(_JNIEnv* env, _jobject* object);
            GlobalReference(const GlobalReference&) = delete;
            GlobalReference(GlobalReference&& rhs) noexcept;
            ~GlobalReference();

            GlobalReference& operator=(const GlobalReference&) = delete;
            GlobalReference& operator=(GlobalReference&& rhs) noexcept;

            operator bool() const noexcept;
            _jobject* operator*() const noexcept;

        public:
            void Reset() noexcept;

        private:
            _JNIEnv* m_pEnv = nullptr;
            _jobject* m_pObject = nullptr;
        };

    public:
        /**
         * 获取对象类名
         * @param env JNI Env
         * @param object 对象
         * @return 类名
         */
        static Result<std::string> GetClassName(_JNIEnv* env, _jobject* object) noexcept;

        /**
         * 从 SDL 获取 AssetManager 指针
         * @param env JNI Env
         * @param sdlContext SDL Context
         * @return AssetManager 对象指针及 Java 类引用
         */
        static Result<std::pair<GlobalReference, AAssetManager*>> GetAndroidAssetManagerFromSDL(_JNIEnv* env,
            _jobject* sdlContext) noexcept;
    };
#endif
}
