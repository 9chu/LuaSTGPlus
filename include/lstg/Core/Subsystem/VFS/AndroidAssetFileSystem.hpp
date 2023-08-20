/**
 * @file
 * @date 2023/8/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "IFileSystem.hpp"

struct AAssetManager;

namespace lstg::Subsystem::VFS
{
   /**
    * 安卓 Asset 文件系统
    */
   class AndroidAssetFileSystem :
       public IFileSystem
   {
   public:
       /**
        * 构造文件系统
        * @param assetManager 安卓 AssetManager 对象，由外部确保指针有效
        * @param prefix Asset 目录前缀，默认为空
        */
       AndroidAssetFileSystem(AAssetManager* assetManager, const char* prefix="") noexcept;

   public:  // IFileSystem
       Result<void> CreateDirectory(Path path) noexcept override;
       Result<void> Remove(Path path) noexcept override;
       Result<void> Rename(Path from, Path to) noexcept override;
       Result<FileAttribute> GetFileAttribute(Path path) noexcept override;
       Result<DirectoryIteratorPtr> VisitDirectory(Path path) noexcept override;
       Result<StreamPtr> OpenFile(Path path, FileAccessMode access, FileOpenFlags flags) noexcept override;
       const std::string& GetUserData() const noexcept override;
       void SetUserData(std::string ud) noexcept override;

   private:
       std::string MakeAssetPath(const Path& path) const;

   private:
       std::string m_stUserData;
       AAssetManager* m_pAssetManager = nullptr;
       std::string m_stPathPrefix;
   };
} // namespace lstg::Subsystem::VFS
