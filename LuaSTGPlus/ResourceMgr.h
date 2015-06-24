#pragma once
#include "Global.h"

namespace LuaSTGPlus
{
	/// @brief ��Դ��
	class ResourcePack
	{
	private:
		std::wstring m_Path;
		std::wstring m_PathLowerCase;
		std::string m_Password;

		unzFile m_zipFile;
	public:
		/// @brief �����Դ����ʵ��·��
		const std::wstring& GetPath()const LNOEXCEPT { return m_Path; }
		/// @brief �����Դ����ʵ��·��Сд����
		const std::wstring& GetPathLowerCase()const LNOEXCEPT { return m_PathLowerCase; }
		/// @brief ��������Դ���ж�λ�������ļ����ڴ�
		/// @param[in] path ���·��
		/// @param[out] outBuf �������ļ�����
		/// @return ʧ�ܷ���false���ɹ�����true
		bool LoadFile(const wchar_t* path, std::vector<char>& outBuf)LNOEXCEPT;
	protected:
		ResourcePack& operator=(const ResourcePack&);
		ResourcePack(const ResourcePack&);
	public:
		/// @brief ������ָ��·��������Դ��
		/// @exception ʧ���׳��쳣
		ResourcePack(const wchar_t* path, const char* passwd);
		~ResourcePack();
	};

	/// @brief ��Դ������
	class ResourceMgr
	{
	private:
		std::list<ResourcePack> m_ResPackList;
	public:
		/// @brief ������Դ��
		/// @param[in] path ·��
		/// @param[in] passwd ����
		bool LoadPack(const wchar_t* path, const char* passwd)LNOEXCEPT;

		/// @brief ж����Դ��
		/// @param[in] path ·��
		void UnloadPack(const wchar_t* path)LNOEXCEPT;

		/// @brief ж��������Դ��
		void UnloadAllPack()LNOEXCEPT { m_ResPackList.clear(); }

		/// @brief ������Դ����UTF8��
		/// @param[in] path ·��
		/// @param[in] passwd ����
		LNOINLINE bool LoadPack(const char* path, const char* passwd)LNOEXCEPT;

		/// @brief ж����Դ����UTF8��
		/// @param[in] path ·��
		LNOINLINE void UnloadPack(const char* path)LNOEXCEPT;

		/// @brief װ���ļ�
		/// @param[in] path ·��
		/// @param[out] outBuf �������
		LNOINLINE bool LoadFile(const wchar_t* path, std::vector<char>& outBuf)LNOEXCEPT;

		/// @brief װ���ļ���UTF8��
		/// @param[in] path ·��
		/// @param[out] outBuf �������
		LNOINLINE bool LoadFile(const char* path, std::vector<char>& outBuf)LNOEXCEPT;

		/// @brief ��ѹ��Դ�ļ�
		/// @param[in] path ·��
		/// @param[in] target Ŀ�ĵ�
		bool ExtractRes(const wchar_t* path, const wchar_t* target)LNOEXCEPT;

		/// @brief ��ѹ��Դ�ļ���UTF8��
		/// @param[in] path ·��
		/// @param[in] target Ŀ�ĵ�
		LNOINLINE bool ExtractRes(const char* path, const char* target)LNOEXCEPT;
	};
}
