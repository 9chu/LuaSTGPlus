#pragma once
#include "Global.h"

#ifdef LoadImage
#undef LoadImage
#endif

namespace LuaSTGPlus
{
	class ResourceMgr;

	/// @brief 资源类型
	enum class ResourceType
	{
		Texture = 1,
		Sprite,
		Animation,
		Music,
		SoundEffect,
		Particle,
		SpriteFont,
		TrueType
	};

	/// @brief 资源池类型
	enum class ResourcePoolType
	{
		None,
		Global,
		Stage
	};

	/// @brief 混合模式
	enum class BlendMode
	{
		AddAdd = 1,
		AddAlpha,
		MulAdd,
		MulAlpha
	};

	/// @brief 资源池
	class ResourcePool
	{
	public:
		template <typename T>
		using PoolType = std::unordered_map<std::string, typename T>;

		struct SpriteEx
		{
			fcyRefPointer<f2dSprite> sprite;  // 精灵对象
			BlendMode blend = BlendMode::AddAlpha;  // 混合
			double a = 0., b = 0.;  // 碰撞半轴长
			bool rect = false;  // 是否为矩形碰撞盒

			SpriteEx() {}
			SpriteEx(const SpriteEx& org)
				: sprite(org.sprite), blend(org.blend), a(org.a), b(org.b), rect(org.rect) {}
		};
	private:
		ResourceMgr* m_pMgr;

		PoolType<fcyRefPointer<f2dTexture2D>> m_TexturePool;
		PoolType<SpriteEx> m_SpritePool;
	public:
		/// @brief 清空对象池
		void Clear()LNOEXCEPT;

		/// @brief 检查资源是否存在
		/// @warning 注意t可以是非法枚举量
		bool CheckResourceExists(ResourceType t, const std::string& name)const LNOEXCEPT;

		/// @brief 导出资源表
		/// @note 在L的堆栈上放置一个table用以存放ResourceType中的资源名称
		/// @warning 注意t可以是非法枚举量
		void ExportResourceList(lua_State* L, ResourceType t)const LNOEXCEPT;

		/// @brief 装载纹理
		/// @param name 名称
		/// @param path 路径
		/// @param mipmaps 纹理链
		bool LoadTexture(const std::string& name, const std::wstring& path, bool mipmaps = true)LNOEXCEPT;

		/// @brief 装载纹理（UTF-8）
		LNOINLINE bool LoadTexture(const char* name, const char* path, bool mipmaps = true)LNOEXCEPT;
		
		/// @brief 装载图像
		LNOINLINE bool LoadImage(const char* name, const char* texname,
			double x, double y, double w, double h, double a, double b, bool rect = false)LNOEXCEPT;

		/// @brief 获取纹理
		fcyRefPointer<f2dTexture2D> GetTexture(const char* name)LNOEXCEPT
		{
			if (m_TexturePool.find(name) == m_TexturePool.end())
				return nullptr;
			else
				return m_TexturePool[name];
		}
	private:
		ResourcePool& operator=(const ResourcePool&);
		ResourcePool(const ResourcePool&);
	public:
		ResourcePool(ResourceMgr* mgr);
	};

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

		ResourcePoolType m_ActivedPool = ResourcePoolType::Global;
		ResourcePool m_GlobalResourcePool;
		ResourcePool m_StageResourcePool;
	public:
		/// @brief 获得当前激活的资源池类型
		ResourcePoolType GetActivedPoolType()LNOEXCEPT
		{
			return m_ActivedPool;
		}

		/// @brief 设置当前激活的资源池类型
		void SetActivedPoolType(ResourcePoolType t)LNOEXCEPT
		{
			m_ActivedPool = t;
		}

		/// @brief 获得当前激活的资源池
		ResourcePool* GetActivedPool()LNOEXCEPT
		{
			return GetResourcePool(m_ActivedPool);
		}

		/// @brief 获得资源池
		ResourcePool* GetResourcePool(ResourcePoolType t)LNOEXCEPT
		{
			switch (t)
			{
			case ResourcePoolType::Global:
				return &m_GlobalResourcePool;
			case ResourcePoolType::Stage:
				return &m_StageResourcePool;
			default:
				return nullptr;
			}
		}

		/// @brief 加载资源包
		/// @param[in] path 路径
		/// @param[in] passwd 密码
		bool LoadPack(const wchar_t* path, const char* passwd)LNOEXCEPT;

		/// @brief 卸载资源包
		/// @param[in] path 路径
		void UnloadPack(const wchar_t* path)LNOEXCEPT;

		/// @brief 卸载所有资源包
		void UnloadAllPack()LNOEXCEPT { m_ResPackList.clear(); }

		/// @brief 卸载所有资源
		void ClearAllResource()LNOEXCEPT
		{
			m_GlobalResourcePool.Clear();
			m_StageResourcePool.Clear();
			m_ActivedPool = ResourcePoolType::Global;
		}

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

		/// @brief 寻找纹理
		fcyRefPointer<f2dTexture2D> FindTexture(const char* texname)LNOEXCEPT
		{
			fcyRefPointer<f2dTexture2D> tRet;
			if (!(tRet = m_StageResourcePool.GetTexture(texname)))
				tRet = m_GlobalResourcePool.GetTexture(texname);
			return tRet;
		}

		LNOINLINE bool GetTextureSize(const char* texname, fcyVec2& out)LNOEXCEPT
		{
			fcyRefPointer<f2dTexture2D> tRet = FindTexture(texname);
			if (!tRet)
				return false;
			out.x = static_cast<float>(tRet->GetWidth());
			out.y = static_cast<float>(tRet->GetHeight());
			return true;
		}
	public:
		ResourceMgr();
	};
}
