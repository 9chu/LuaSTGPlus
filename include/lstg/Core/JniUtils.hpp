/**
 * @file
 * @date 2023/8/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <string>
#include <memory>
#include "Result.hpp"

struct AAssetManager;

struct _JNIEnv;
struct _jobject;

namespace lstg
{
    namespace JniUtils
    {
        /**
         * JObject 全局引用
         */
        class JObjectReference
        {
        public:
            /**
             * 构造空的 JObject 引用
             */
            JObjectReference() = default;

            /**
             * 从 JNIEnv 和 JObject 构造引用
             * @param env JNIEnv
             * @param object JObject，必须是全局引用
             */
            JObjectReference(_JNIEnv* env, _jobject* object);

            JObjectReference(const JObjectReference&) = delete;
            JObjectReference(JObjectReference&& rhs) noexcept;
            ~JObjectReference();

            JObjectReference& operator=(const JObjectReference&) = delete;
            JObjectReference& operator=(JObjectReference&& rhs) noexcept;

            /**
             * 用于检查是否为非空
             */
            operator bool() const noexcept;

            /**
             * 解引用
             */
            _jobject* operator*() const noexcept;

        public:
            /**
             * 对象置空
             */
            void Reset() noexcept;

        private:
            _JNIEnv* m_pEnv = nullptr;
            _jobject* m_pObject = nullptr;
        };
    }  // namespace JniUtils
}  // namespace lstg
