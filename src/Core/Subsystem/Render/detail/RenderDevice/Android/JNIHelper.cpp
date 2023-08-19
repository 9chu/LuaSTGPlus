/**
 * @file
 * @date 2023/8/19
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "JNIHelper.hpp"

#include <cassert>
#include <system_error>
#include <lstg/Core/Logging.hpp>

#ifdef LSTG_PLATFORM_ANDROID

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::RenderDevice::Android;

LSTG_DEF_LOG_CATEGORY(JNIHelper);

// <editor-fold desc="JNIHelper::GlobalReference">

JNIHelper::GlobalReference::GlobalReference(JNIEnv* env, jobject object)
    : m_pEnv(env), m_pObject(object)
{
    assert(env && object);
}

JNIHelper::GlobalReference::GlobalReference(GlobalReference&& rhs) noexcept
    : m_pEnv(rhs.m_pEnv), m_pObject(rhs.m_pObject)
{
    assert((!m_pEnv && !m_pObject) || (m_pEnv && m_pObject));
    rhs.m_pObject = nullptr;
}

JNIHelper::GlobalReference::~GlobalReference()
{
    Reset();
}

JNIHelper::GlobalReference& JNIHelper::GlobalReference::operator=(GlobalReference&& rhs) noexcept
{
    if (this == &rhs)
        return *this;

    Reset();

    m_pEnv = rhs.m_pEnv;
    m_pObject = rhs.m_pObject;
    rhs.m_pEnv = nullptr;
    rhs.m_pObject = nullptr;

    return *this;
}

JNIHelper::GlobalReference::operator bool() const noexcept
{
    return m_pObject != nullptr;
}

jobject JNIHelper::GlobalReference::operator*() const noexcept
{
    return m_pObject;
}

void JNIHelper::GlobalReference::Reset() noexcept
{
    if (m_pObject)
        m_pEnv->DeleteGlobalRef(m_pObject);

    m_pEnv = nullptr;
    m_pObject = nullptr;
}

// </editor-fold>

Result<std::string> JNIHelper::GetClassName(JNIEnv* env, jobject object) noexcept
{
    // https://stackoverflow.com/questions/12719766/can-i-know-the-name-of-the-class-that-calls-a-jni-c-method

    assert(env && object);

    // clsObj = object.getClass()
    auto cls = env->GetObjectClass(object);
    auto mid = env->GetMethodID(cls, "getClass", "()Ljava/lang/Class;");
    auto clsObj = env->CallObjectMethod(object, mid);

    // strObj = clsObj.getName()
    cls = env->GetObjectClass(clsObj);
    mid = env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
    auto strObj = static_cast<jstring>(env->CallObjectMethod(clsObj, mid));

    string ret;
    auto str = env->GetStringUTFChars(strObj, nullptr);
    try
    {
        ret.assign(str);
    }
    catch (...)
    {
        env->ReleaseStringUTFChars(strObj, str);
        return make_error_code(errc::not_enough_memory);
    }

    env->ReleaseStringUTFChars(strObj, str);
    return ret;
}

Result<std::pair<JNIHelper::GlobalReference, AAssetManager*>> JNIHelper::GetAndroidAssetManagerFromSDL(JNIEnv* env,
    jobject sdlContext) noexcept
{
    assert(env && sdlContext);

    // javaAssetManager = context.getAssets()
    auto mid = env->GetMethodID(env->GetObjectClass(sdlContext), "getAssets", "()Landroid/content/res/AssetManager;");
    auto javaAssetManager = env->CallObjectMethod(sdlContext, mid);
    if (!javaAssetManager)
    {
        LSTG_LOG_ERROR_CAT(JNIHelper, "context.getAssets() -> null");
        return make_error_code(errc::operation_not_supported);
    }

    // 需要持有 javaAssetManager 对象防止被 GC
    GlobalReference ref(env, env->NewGlobalRef(javaAssetManager));
    auto native = ::AAssetManager_fromJava(env, *ref);
    if (!native)
    {
        LSTG_LOG_ERROR_CAT(JNIHelper, "AAssetManager_fromJava() -> null");
        return make_error_code(errc::operation_not_supported);
    }

    return make_pair<GlobalReference, AAssetManager*>(std::move(ref), std::move(native));
}

#endif
