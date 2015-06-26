#include "ResourceMgr.h"
#include "AppFrame.h"

#include <iowin32.h>

using namespace std;
using namespace LuaSTGPlus;

////////////////////////////////////////////////////////////////////////////////
/// ResourcePool
////////////////////////////////////////////////////////////////////////////////
ResourcePool::ResourcePool(ResourceMgr* mgr)
	: m_pMgr(mgr)
{
}

void ResourcePool::Clear()LNOEXCEPT
{
	m_TexturePool.clear();
	m_SpritePool.clear();
}

bool ResourcePool::CheckResourceExists(ResourceType t, const std::string& name)const LNOEXCEPT
{
	switch (t)
	{
	case ResourceType::Texture:
		return m_TexturePool.find(name) != m_TexturePool.end();
	case ResourceType::Sprite:
		return m_SpritePool.find(name) != m_SpritePool.end();
	case ResourceType::Animation:
		break;
	case ResourceType::Music:
		break;
	case ResourceType::SoundEffect:
		break;
	case ResourceType::Particle:
		break;
	case ResourceType::SpriteFont:
		break;
	case ResourceType::TrueType:
		break;
	default:
		break;
	}
	return false;
}

void ResourcePool::ExportResourceList(lua_State* L, ResourceType t)const LNOEXCEPT
{
	int cnt = 1;
	switch (t)
	{
	case ResourceType::Texture:
		lua_newtable(L);  // t
		for (auto i : m_TexturePool)
		{
			lua_pushstring(L, i.first.c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Sprite:
		lua_newtable(L);  // t
		for (auto i : m_SpritePool)
		{
			lua_pushstring(L, i.first.c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Animation:
		lua_newtable(L);  // t
		break;
	case ResourceType::Music:
		lua_newtable(L);  // t
		break;
	case ResourceType::SoundEffect:
		lua_newtable(L);  // t
		break;
	case ResourceType::Particle:
		lua_newtable(L);  // t
		break;
	case ResourceType::SpriteFont:
		lua_newtable(L);  // t
		break;
	case ResourceType::TrueType:
		lua_newtable(L);  // t
		break;
	default:
		lua_pushnil(L);
		break;
	}
}

bool ResourcePool::LoadTexture(const std::string& name, const std::wstring& path, bool mipmaps)LNOEXCEPT
{
	LASSERT(LAPP.GetRenderDev());

	if (m_TexturePool.find(name) != m_TexturePool.end())
	{
		LWARNING("LoadTexture: 纹理'%m'已存在，试图使用'%s'加载的操作已被取消", name.c_str(), path.c_str());
		return true;
	}
	
	vector<char> tDataBuf;
	if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
	{
		LERROR("LoadTexture: 无法装载文件'%s'", path.c_str());
		return false;
	}
	
	fcyRefPointer<f2dTexture2D> tTexture;
	if (FCYFAILED(LAPP.GetRenderDev()->CreateTextureFromMemory((fcData)tDataBuf.data(), tDataBuf.size(), 0, 0, false, mipmaps, &tTexture)))
	{
		LERROR("LoadTexture: 从文件'%s'创建纹理'%m'失败", path.c_str(), name.c_str());
		return false;
	}

	try
	{
		m_TexturePool.emplace(name, tTexture);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadTexture: 内存不足");
		return false;
	}
	LINFO("LoadTexture: 纹理'%s'已装载 -> '%m'", path.c_str(), name.c_str());
	return true;
}

bool ResourcePool::LoadTexture(const char* name, const char* path, bool mipmaps)LNOEXCEPT
{
	try
	{
		return LoadTexture(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), mipmaps);
	}
	catch (const bad_alloc&)
	{
		LERROR("转换编码时无法分配内存");
		return false;
	}
}

bool ResourcePool::LoadImage(const char* name, const char* texname,
	double x, double y, double w, double h, double a, double b, bool rect)LNOEXCEPT
{
	LASSERT(LAPP.GetRenderer());

	if (m_SpritePool.find(name) != m_SpritePool.end())
	{
		LWARNING("LoadImage: 图像'%m'已存在，加载操作已被取消", name);
		return true;
	}

	// 获得纹理
	fcyRefPointer<f2dTexture2D> pTex = m_pMgr->FindTexture(texname);
	if (!pTex)
	{
		LWARNING("LoadImage: 加载图像'%m'失败, 无法找到纹理'%m'", name, texname);
		return false;
	}

	// 创建精灵对象
	SpriteEx tSpriteEx;
	fcyRect tRect((float)x, (float)y, (float)(x + w), (float)(y + h));
	if (FCYFAILED(LAPP.GetRenderer()->CreateSprite2D(pTex, tRect, &tSpriteEx.sprite)))
	{
		LERROR("LoadImage: 无法从纹理'%m'加载图像'%m' (CreateSprite2D failed)", texname, name);
		return false;
	}
	tSpriteEx.a = a;
	tSpriteEx.b = b;
	tSpriteEx.rect = rect;

	try
	{
		m_SpritePool.emplace(name, tSpriteEx);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadImage: 内存不足");
		return false;
	}
	LINFO("LoadImage: 图像'%m'已装载", name);
	return true;
}

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
		LERROR("ResourcePack: 无法打开资源包'%s' (unzOpen失败)", path);
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
		LERROR("ResourcePack: 转换资源目录编码时无法分配内存");
		return false;
	}

	int tStatus = unzGoToFirstFile(m_zipFile);
	while (UNZ_OK == tStatus)
	{
		unz_file_info tFileInfo;
		char tZipName[MAX_PATH];

		if (UNZ_OK == unzGetCurrentFileInfo(m_zipFile, &tFileInfo, tZipName, sizeof(tZipName), nullptr, 0, nullptr, 0))
		{
			// 对路径做统一性转换
			pathUniform(tZipName, tZipName + MAX_PATH);

			// 检查路径是否命中
			if (strcmp(tPathInUtf8.c_str(), tZipName) == 0)
			{
				LINFO("ResourcePack: 资源包'%s'命中文件'%s'", m_Path.c_str(), path);
				if (unzOpenCurrentFilePassword(m_zipFile, m_Password.length() > 0 ? m_Password.c_str() : nullptr) != UNZ_OK)
				{
					LERROR("ResourcePack: 尝试打开资源包'%s'中的文件'%s'失败(密码错误?)", m_Path.c_str(), path);
					return false;
				}

				try
				{
					outBuf.resize(tFileInfo.uncompressed_size);
				}
				catch (const bad_alloc&)
				{
					unzCloseCurrentFile(m_zipFile);
					LERROR("ResourcePack: 无法分配足够内存解压资源包'%s'中的文件'%s'", m_Path.c_str(), path);
					return false;
				}
				
				if (unzReadCurrentFile(m_zipFile, outBuf.data(), tFileInfo.uncompressed_size) < 0)
				{
					unzCloseCurrentFile(m_zipFile);
					LERROR("ResourcePack: 解压资源包'%s'中的文件'%s'失败 (unzReadCurrentFile失败)", m_Path.c_str(), path);
					return false;
				}
				unzCloseCurrentFile(m_zipFile);

				return true;
			}
		}
		else
			LWARNING("ResourcePack: 在资源包'%s'中寻找文件时发生错误 (unzGetCurrentFileInfo失败)", m_Path.c_str());

		tStatus = unzGoToNextFile(m_zipFile);
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
/// ResourceMgr
////////////////////////////////////////////////////////////////////////////////
ResourceMgr::ResourceMgr()
	: m_GlobalResourcePool(this), m_StageResourcePool(this)
{
}

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
				LWARNING("ResourceMgr: 资源包'%s'已加载，不能重复加载", path);
				return true;
			}
		}
		m_ResPackList.emplace_front(path, passwd);
		LINFO("ResourceMgr: 已装载资源包'%s'", path);
		return true;
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: 加载资源包时无法分配足够内存");
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
				LINFO("ResourceMgr: 已卸载资源包'%s'", path);
				return;
			}
		}
		LWARNING("ResourceMgr: 资源包'%s'未加载，无法卸载", path);
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: 卸载资源包时无法分配足够内存");
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
		LERROR("ResourceMgr: 转换字符编码时无法分配内存");
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
		LERROR("ResourceMgr: 转换字符编码时无法分配内存");
	}
}

LNOINLINE bool ResourceMgr::LoadFile(const wchar_t* path, std::vector<char>& outBuf)LNOEXCEPT
{
	// 尝试从各个资源包加载
	for (auto& i : m_ResPackList)
	{
		if (i.LoadFile(path, outBuf))
			return true;
	}

	// 尝试从本地加载
	LINFO("ResourceMgr: 尝试从本地加载文件'%s'", path);
	fcyRefPointer<fcyFileStream> pFile;
	try
	{
		pFile.DirectSet(new fcyFileStream(path, false));
		outBuf.resize((size_t)pFile->GetLength());
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: 无法分配足够内存从本地加载文件'%s'", path);
		return false;
	}
	catch (const fcyException& e)
	{
		LERROR("ResourceMgr: 装载本地文件'%s'失败 (异常信息'%m' 源'%m')", path, e.GetDesc(), e.GetSrc());
		return false;
	}

	if (pFile->GetLength() > 0)
	{
		if (FCYFAILED(pFile->ReadBytes((fData)outBuf.data(), outBuf.size(), nullptr)))
		{
			LERROR("ResourceMgr: 读取本地文件'%s'失败 (fcyFileStream::ReadBytes失败)", path);
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
		LERROR("ResourceMgr: 转换字符编码时无法分配内存");
	}
	return false;
}

bool ResourceMgr::ExtractRes(const wchar_t* path, const wchar_t* target)LNOEXCEPT
{
	vector<char> tBuf;

	// 读取文件
	if (LoadFile(path, tBuf))
	{
		// 打开本地文件
		fcyRefPointer<fcyFileStream> pFile;
		try
		{
			pFile.DirectSet(new fcyFileStream(target, true));
			if (FCYFAILED(pFile->SetLength(0)))
			{
				LERROR("ResourceMgr: 无法清空文件'%s' (fcyFileStream::SetLength 失败)", target);
				return false;
			}
			if (tBuf.size() > 0)
			{
				if (FCYFAILED(pFile->WriteBytes((fcData)tBuf.data(), tBuf.size(), nullptr)))
				{
					LERROR("ResourceMgr: 无法向文件'%s'写出数据", target);
					return false;
				}
			}
		}
		catch (const bad_alloc&)
		{
			LERROR("ResourceMgr: 无法分配足够内存来向文件'%s'写出数据", target);
			return false;
		}
		catch (const fcyException& e)
		{
			LERROR("ResourceMgr: 打开本地文件'%s'失败 (异常信息'%m' 源'%m')", target, e.GetDesc(), e.GetSrc());
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
		LERROR("ResourceMgr: 转换字符编码时无法分配内存");
	}
	return false;
}
