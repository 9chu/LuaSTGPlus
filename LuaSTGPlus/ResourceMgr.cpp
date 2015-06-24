#include "ResourceMgr.h"

#include <iowin32.h>

using namespace std;
using namespace LuaSTGPlus;

////////////////////////////////////////////////////////////////////////////////
/// ResourcePack
////////////////////////////////////////////////////////////////////////////////
template <typename T>
void pathUniform(T begin, T end)
{
	while (begin != end)
	{
		int c = *begin;
		if (c == '/')
			*begin = '\\';
		else if (c >= 'A' && c <= 'Z')
			*begin = c - 'A' + 'a';
		else if (c == 0)
			break;
		++begin;
	}
}

ResourcePack::ResourcePack(const wchar_t* path, const char* passwd)
	: m_Path(path), m_PathLowerCase(path), m_Password(passwd ? passwd : "")
{
	pathUniform(m_PathLowerCase.begin(), m_PathLowerCase.end());

	zlib_filefunc_def tZlibFileFunc;
	memset(&tZlibFileFunc, 0, sizeof(tZlibFileFunc));
	fill_win32_filefunc(&tZlibFileFunc);
	m_zipFile = unzOpen2(reinterpret_cast<const char*>(path), &tZlibFileFunc);
	if (!m_zipFile)
	{
		LERROR("ResourcePack: �޷�����Դ��'%s' (unzOpenʧ��)", path);
		throw fcyException("ResourcePack::ResourcePack", "Can't open resource pack.");
	}
}

ResourcePack::~ResourcePack()
{
	unzClose(m_zipFile);
}

bool ResourcePack::LoadFile(const wchar_t* path, std::vector<char>& outBuf)LNOEXCEPT
{
	string tPathInUtf8;
	try
	{
		tPathInUtf8 = fcyStringHelper::WideCharToMultiByte(path, CP_UTF8);
		pathUniform(tPathInUtf8.begin(), tPathInUtf8.end());
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourcePack: ת����ԴĿ¼����ʱ�޷������ڴ�");
		return false;
	}

	int tStatus = unzGoToFirstFile(m_zipFile);
	while (UNZ_OK == tStatus)
	{
		unz_file_info tFileInfo;
		char tZipName[MAX_PATH];

		if (UNZ_OK == unzGetCurrentFileInfo(m_zipFile, &tFileInfo, tZipName, sizeof(tZipName), nullptr, 0, nullptr, 0))
		{
			// ��·����ͳһ��ת��
			pathUniform(tZipName, tZipName + MAX_PATH);

			// ���·���Ƿ�����
			if (strcmp(tPathInUtf8.c_str(), tZipName) == 0)
			{
				LINFO("ResourcePack: ��Դ��'%s'�����ļ�'%s'", m_Path.c_str(), path);
				if (unzOpenCurrentFilePassword(m_zipFile, m_Password.length() > 0 ? m_Password.c_str() : nullptr) != UNZ_OK)
				{
					LERROR("ResourcePack: ���Դ���Դ��'%s'�е��ļ�'%s'ʧ��(�������?)", m_Path.c_str(), path);
					return false;
				}

				try
				{
					outBuf.resize(tFileInfo.uncompressed_size);
				}
				catch (const bad_alloc&)
				{
					unzCloseCurrentFile(m_zipFile);
					LERROR("ResourcePack: �޷������㹻�ڴ��ѹ��Դ��'%s'�е��ļ�'%s'", m_Path.c_str(), path);
					return false;
				}
				
				if (unzReadCurrentFile(m_zipFile, outBuf.data(), tFileInfo.uncompressed_size) < 0)
				{
					unzCloseCurrentFile(m_zipFile);
					LERROR("ResourcePack: ��ѹ��Դ��'%s'�е��ļ�'%s'ʧ�� (unzReadCurrentFileʧ��)", m_Path.c_str(), path);
					return false;
				}
				unzCloseCurrentFile(m_zipFile);

				return true;
			}
		}
		else
			LWARNING("ResourcePack: ����Դ��'%s'��Ѱ���ļ�ʱ�������� (unzGetCurrentFileInfoʧ��)", m_Path.c_str());

		tStatus = unzGoToNextFile(m_zipFile);
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
/// ResourceMgr
////////////////////////////////////////////////////////////////////////////////
bool ResourceMgr::LoadPack(const wchar_t* path, const char* passwd)LNOEXCEPT
{
	try
	{
		wstring tPath = path;
		pathUniform(tPath.begin(), tPath.end());
		for (auto& i : m_ResPackList)
		{
			if (i.GetPathLowerCase() == tPath)
			{
				LWARNING("ResourceMgr: ��Դ��'%s'�Ѽ��أ������ظ�����", path);
				return true;
			}
		}
		m_ResPackList.emplace_front(path, passwd);
		LINFO("ResourceMgr: ��װ����Դ��'%s'", path);
		return true;
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: ������Դ��ʱ�޷������㹻�ڴ�");
	}
	catch (const fcyException&)
	{
	}
	return false;
}

void ResourceMgr::UnloadPack(const wchar_t* path)LNOEXCEPT
{
	try
	{
		wstring tPath = path;
		pathUniform(tPath.begin(), tPath.end());
		for (auto i = m_ResPackList.begin(); i != m_ResPackList.end(); ++i)
		{
			if (i->GetPathLowerCase() == tPath)
			{
				m_ResPackList.erase(i);
				LINFO("ResourceMgr: ��ж����Դ��'%s'", path);
				return;
			}
		}
		LWARNING("ResourceMgr: ��Դ��'%s'δ���أ��޷�ж��", path);
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: ж����Դ��ʱ�޷������㹻�ڴ�");
	}
}

LNOINLINE bool ResourceMgr::LoadPack(const char* path, const char* passwd)LNOEXCEPT
{
	try
	{
		wstring tPath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
		return LoadPack(tPath.c_str(), passwd);
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: ת���ַ�����ʱ�޷������ڴ�");
		return false;
	}
}

LNOINLINE void ResourceMgr::UnloadPack(const char* path)LNOEXCEPT
{
	try
	{
		wstring tPath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
		UnloadPack(tPath.c_str());
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: ת���ַ�����ʱ�޷������ڴ�");
	}
}

LNOINLINE bool ResourceMgr::LoadFile(const wchar_t* path, std::vector<char>& outBuf)LNOEXCEPT
{
	// ���ԴӸ�����Դ������
	for (auto& i : m_ResPackList)
	{
		if (i.LoadFile(path, outBuf))
			return true;
	}

	// ���Դӱ��ؼ���
	LINFO("ResourceMgr: ���Դӱ��ؼ����ļ�'%s'", path);
	fcyRefPointer<fcyFileStream> pFile;
	try
	{
		pFile.DirectSet(new fcyFileStream(path, false));
		outBuf.resize((size_t)pFile->GetLength());
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: �޷������㹻�ڴ�ӱ��ؼ����ļ�'%s'", path);
		return false;
	}
	catch (const fcyException& e)
	{
		LERROR("ResourceMgr: װ�ر����ļ�'%s'ʧ�� (�쳣��Ϣ'%m' Դ'%m')", path, e.GetDesc(), e.GetSrc());
		return false;
	}

	if (pFile->GetLength() > 0)
	{
		if (FCYFAILED(pFile->ReadBytes((fData)outBuf.data(), outBuf.size(), nullptr)))
		{
			LERROR("ResourceMgr: ��ȡ�����ļ�'%s'ʧ�� (fcyFileStream::ReadBytesʧ��)", path);
			return false;
		}
	}

	return true;
}

LNOINLINE bool ResourceMgr::LoadFile(const char* path, std::vector<char>& outBuf)LNOEXCEPT
{
	try
	{
		wstring tPath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
		return LoadFile(tPath.c_str(), outBuf);
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: ת���ַ�����ʱ�޷������ڴ�");
	}
	return false;
}

bool ResourceMgr::ExtractRes(const wchar_t* path, const wchar_t* target)LNOEXCEPT
{
	vector<char> tBuf;

	// ��ȡ�ļ�
	if (LoadFile(path, tBuf))
	{
		// �򿪱����ļ�
		fcyRefPointer<fcyFileStream> pFile;
		try
		{
			pFile.DirectSet(new fcyFileStream(target, true));
			if (FCYFAILED(pFile->SetLength(0)))
			{
				LERROR("ResourceMgr: �޷�����ļ�'%s' (fcyFileStream::SetLength ʧ��)", target);
				return false;
			}
			if (tBuf.size() > 0)
			{
				if (FCYFAILED(pFile->WriteBytes((fcData)tBuf.data(), tBuf.size(), nullptr)))
				{
					LERROR("ResourceMgr: �޷����ļ�'%s'д������", target);
					return false;
				}
			}
		}
		catch (const bad_alloc&)
		{
			LERROR("ResourceMgr: �޷������㹻�ڴ������ļ�'%s'д������", target);
			return false;
		}
		catch (const fcyException& e)
		{
			LERROR("ResourceMgr: �򿪱����ļ�'%s'ʧ�� (�쳣��Ϣ'%m' Դ'%m')", target, e.GetDesc(), e.GetSrc());
			return false;
		}
	}
	return true;
}

LNOINLINE bool ResourceMgr::ExtractRes(const char* path, const char* target)LNOEXCEPT
{
	try
	{
		wstring tPath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
		wstring tTarget = fcyStringHelper::MultiByteToWideChar(target, CP_UTF8);
		return ExtractRes(tPath.c_str(), tTarget.c_str());
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: ת���ַ�����ʱ�޷������ڴ�");
	}
	return false;
}
