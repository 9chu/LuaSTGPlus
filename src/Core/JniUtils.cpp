/**
 * @file
 * @date 2023/8/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/JniUtils.hpp>

#ifdef LSTG_PLATFORM_ANDROID

#include <cassert>
#include <system_error>

#include <jni.h>

using namespace std;
using namespace lstg;

// <editor-fold desc="JniUtils::JObjectReference">

JniUtils::JObjectReference::JObjectReference(JNIEnv* env, jobject object)
    : m_pEnv(env), m_pObject(object)
{
    assert(env && object);
}

JniUtils::JObjectReference::JObjectReference(JObjectReference&& rhs) noexcept
    : m_pEnv(rhs.m_pEnv), m_pObject(rhs.m_pObject)
{
    assert((!m_pEnv && !m_pObject) || (m_pEnv && m_pObject));
    rhs.m_pObject = nullptr;
}

JniUtils::JObjectReference::~JObjectReference()
{
    Reset();
}

JniUtils::JObjectReference& JniUtils::JObjectReference::operator=(JObjectReference&& rhs) noexcept
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

JniUtils::JObjectReference::operator bool() const noexcept
{
    return m_pObject != nullptr;
}

jobject JniUtils::JObjectReference::operator*() const noexcept
{
    return m_pObject;
}

void JniUtils::JObjectReference::Reset() noexcept
{
    if (m_pObject)
        m_pEnv->DeleteGlobalRef(m_pObject);

    m_pEnv = nullptr;
    m_pObject = nullptr;
}

// </editor-fold>

#endif
