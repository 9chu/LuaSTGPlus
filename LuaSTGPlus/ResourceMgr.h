#pragma once
#include "Global.h"

namespace LuaSTGPlus
{
	/// @brief 资源包
	class ResourcePack
	{
	private:
		std::wstring m_Path;
		std::wstring m_PathLowerCase;
		std::string m_Password;

		unzFile m_zipFile;
	public:
		/// @brief 获得资源包的实际路径
		const std::wstring& GetPath()const LNOEXCEPT { return m_Path; }
		/// @brief 获得资源包的实际路径小写名称
		const std::wstring& GetPathLowerCase()const LNOEXCEPT { return m_PathLowerCase; }
		/// @brief 尝试在资源包中定位并加载文件到内存
		/// @param[in] path 相对路径
		/// @param[out] outBuf 导出的文件数据
		/// @return 失败返回false，成功返回true
		bool LoadFile(const wchar_t* path, std::vector<char>& outBuf)LNOEXCEPT;
	protected:
		ResourcePack& operator=(const ResourcePack&);
		ResourcePack(const ResourcePack&);
	public:
		/// @brief 尝试在指定路径加载资源包
		/// @exception 失败抛出异常
		ResourcePack(const wchar_t* path, const char* passwd);
		~ResourcePack();
	};

	/// @brief 资源管理器
	class ResourceMgr
	{
	private:
		std::list<ResourcePack> m_ResPackList;
	public:
		/// @brief 加载资源包
		/// @param[in] path 路径
		/// @param[in] passwd 密码
		bool LoadPack(const wchar_t* path, const char* passwd)LNOEXCEPT;

		/// @brief 卸载资源包
		/// @param[in] path 路径
		void UnloadPack(const wchar_t* path)LNOEXCEPT;

		/// @brief 卸载所有资源包
		void UnloadAllPack()LNOEXCEPT { m_ResPackList.clear(); }

		/// @brief 加载资源包（UTF8）
		/// @param[in] path 路径
		/// @param[in] passwd 密码
		LNOINLINE bool LoadPack(const char* path, const char* passwd)LNOEXCEPT;

		/// @brief 卸载资源包（UTF8）
		/// @param[in] path 路径
		LNOINLINE void UnloadPack(const char* path)LNOEXCEPT;

		/// @brief 装载文件
		/// @param[in] path 路径
		/// @param[out] outBuf 输出缓冲
		LNOINLINE bool LoadFile(const wchar_t* path, std::vector<char>& outBuf)LNOEXCEPT;

		/// @brief 装载文件（UTF8）
		/// @param[in] path 路径
		/// @param[out] outBuf 输出缓冲
		LNOINLINE bool LoadFile(const char* path, std::vector<char>& outBuf)LNOEXCEPT;

		/// @brief 解压资源文件
		/// @param[in] path 路径
		/// @param[in] target 目的地
		bool ExtractRes(const wchar_t* path, const wchar_t* target)LNOEXCEPT;

		/// @brief 解压资源文件（UTF8）
		/// @param[in] path 路径
		/// @param[in] target 目的地
		LNOINLINE bool ExtractRes(const char* path, const char* target)LNOEXCEPT;
	};
}
