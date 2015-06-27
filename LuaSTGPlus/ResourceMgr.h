#pragma once
#include "Global.h"
#include "Dictionary.hpp"

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
	
	/// @brief 资源接口
	class Resource :
		public fcyRefObjImpl<fcyRefObj>
	{
	private:
		ResourceType m_Type;
		std::string m_ResName;
	public:
		ResourceType GetType()const LNOEXCEPT { return m_Type; }
		const std::string& GetResName()const LNOEXCEPT { return m_ResName; }
	private:
		Resource& operator=(const Resource&);
		Resource(const Resource&);
	public:
		Resource(ResourceType t, const char* name)
			: m_Type(t), m_ResName(name) {}
	};

	/// @brief 纹理资源
	class ResTexture :
		public Resource
	{
	private:
		fcyRefPointer<f2dTexture2D> m_Texture;
	public:
		f2dTexture2D* GetTexture() { return m_Texture; }
	public:
		ResTexture(const char* name, fcyRefPointer<f2dTexture2D> tex)
			: Resource(ResourceType::Texture, name), m_Texture(tex) {}
	};

	/// @brief 图像资源
	class ResSprite :
		public Resource
	{
	private:
		fcyRefPointer<f2dSprite> m_Sprite;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		double m_HalfSizeX = 0.;
		double m_HalfSizeY = 0.;
		bool m_bRectangle = false;
	public:
		f2dSprite* GetSprite()LNOEXCEPT { return m_Sprite; }
		BlendMode GetBlendMode()const LNOEXCEPT { return m_BlendMode; }
		void SetBlendMode(BlendMode m)LNOEXCEPT { m_BlendMode = m; }
		double GetHalfSizeX()const LNOEXCEPT { return m_HalfSizeX; }
		double GetHalfSizeY()const LNOEXCEPT { return m_HalfSizeY; }
		bool IsRectangle()const LNOEXCEPT { return m_bRectangle; }
	public:
		ResSprite(const char* name, fcyRefPointer<f2dSprite> sprite, double hx, double hy, bool rect)
			: Resource(ResourceType::Sprite, name), m_Sprite(sprite), m_HalfSizeX(hx), m_HalfSizeY(hy), m_bRectangle(rect)
		{
			m_Sprite->SetColor(0xFFFFFFFF);  // 适应乘法
		}
	};

	/// @brief 动画资源
	class ResAnimation :
		public Resource
	{
	private:
		std::vector<fcyRefPointer<f2dSprite>> m_ImageSequences;
		fuInt m_Interval = 1;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		double m_HalfSizeX = 0.;
		double m_HalfSizeY = 0.;
		bool m_bRectangle = false;
	public:
		size_t GetCount()const LNOEXCEPT { return m_ImageSequences.size(); }
		f2dSprite* GetSprite(fuInt index)LNOEXCEPT
		{
			if (index >= GetCount())
				return nullptr;
			return m_ImageSequences[index];
		}
		fuInt GetInterval()const LNOEXCEPT { return m_Interval; }
		BlendMode GetBlendMode()const LNOEXCEPT { return m_BlendMode; }
		void SetBlendMode(BlendMode m)LNOEXCEPT { m_BlendMode = m; }
		double GetHalfSizeX()const LNOEXCEPT { return m_HalfSizeX; }
		double GetHalfSizeY()const LNOEXCEPT { return m_HalfSizeY; }
		bool IsRectangle()const LNOEXCEPT { return m_bRectangle; }
	public:
		ResAnimation(const char* name, fcyRefPointer<ResTexture> tex, float x, float y, float w, float h,
			int n, int m, int intv, double a, double b, bool rect = false);
	};

	/// @brief 资源池
	class ResourcePool
	{
	private:
		ResourceMgr* m_pMgr;

		Dictionary<fcyRefPointer<ResTexture>> m_TexturePool;
		Dictionary<fcyRefPointer<ResSprite>> m_SpritePool;
		Dictionary<fcyRefPointer<ResAnimation>> m_AnimationPool;
	public:
		/// @brief 清空对象池
		void Clear()LNOEXCEPT
		{
			m_TexturePool.clear();
			m_SpritePool.clear();
			m_AnimationPool.clear();
		}

		/// @brief 检查资源是否存在
		/// @warning 注意t可以是非法枚举量
		bool CheckResourceExists(ResourceType t, const std::string& name)const LNOEXCEPT
		{
			switch (t)
			{
			case ResourceType::Texture:
				return m_TexturePool.find(name.c_str()) != m_TexturePool.end();
			case ResourceType::Sprite:
				return m_SpritePool.find(name.c_str()) != m_SpritePool.end();
			case ResourceType::Animation:
				return m_AnimationPool.find(name.c_str()) != m_AnimationPool.end();
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

		/// @brief 导出资源表
		/// @note 在L的堆栈上放置一个table用以存放ResourceType中的资源名称
		/// @warning 注意t可以是非法枚举量
		void ExportResourceList(lua_State* L, ResourceType t)const LNOEXCEPT;

		/// @brief 装载纹理
		/// @param name 名称
		/// @param path 路径
		/// @param mipmaps 纹理链
		bool LoadTexture(const char* name, const std::wstring& path, bool mipmaps = true)LNOEXCEPT;

		/// @brief 装载纹理（UTF-8）
		LNOINLINE bool LoadTexture(const char* name, const char* path, bool mipmaps = true)LNOEXCEPT;
		
		/// @brief 装载图像
		LNOINLINE bool LoadImage(const char* name, const char* texname,
			double x, double y, double w, double h, double a, double b, bool rect = false)LNOEXCEPT;

		LNOINLINE bool LoadAnimation(const char* name, const char* texname,
			double x, double y, double w, double h, int n, int m, int intv, double a, double b, bool rect = false)LNOEXCEPT;

		/// @brief 获取纹理
		fcyRefPointer<ResTexture> GetTexture(const char* name)LNOEXCEPT
		{
			auto i = m_TexturePool.find(name);
			if (i == m_TexturePool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief 获取精灵
		fcyRefPointer<ResSprite> GetSprite(const char* name)LNOEXCEPT
		{
			auto i = m_SpritePool.find(name);
			if (i == m_SpritePool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief 获取精灵
		fcyRefPointer<ResAnimation> GetAnimation(const char* name)LNOEXCEPT
		{
			auto i = m_AnimationPool.find(name);
			if (i == m_AnimationPool.end())
				return nullptr;
			else
				return i->second;
		}
	private:
		ResourcePool& operator=(const ResourcePool&);
		ResourcePool(const ResourcePool&);
	public:
		ResourcePool(ResourceMgr* mgr)
			: m_pMgr(mgr) {}
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

		float m_GlobalImageScaleFactor = 1.0f;
		ResourcePoolType m_ActivedPool = ResourcePoolType::Global;
		ResourcePool m_GlobalResourcePool;
		ResourcePool m_StageResourcePool;
	public:
		float GetGlobalImageScaleFactor()const LNOEXCEPT{ return m_GlobalImageScaleFactor; }
		void SetGlobalImageScaleFactor(float s)LNOEXCEPT{ m_GlobalImageScaleFactor = s; }

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

		/// @brief 卸载所有资源并重置状态
		void ClearAllResource()LNOEXCEPT
		{
			m_GlobalResourcePool.Clear();
			m_StageResourcePool.Clear();
			m_ActivedPool = ResourcePoolType::Global;
			m_GlobalImageScaleFactor = 1.;
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
		fcyRefPointer<ResTexture> FindTexture(const char* texname)LNOEXCEPT
		{
			fcyRefPointer<ResTexture> tRet;
			if (!(tRet = m_StageResourcePool.GetTexture(texname)))
				tRet = m_GlobalResourcePool.GetTexture(texname);
			return tRet;
		}

		/// @brief 获取纹理大小
		bool GetTextureSize(const char* texname, fcyVec2& out)LNOEXCEPT
		{
			fcyRefPointer<ResTexture> tRet = FindTexture(texname);
			if (!tRet)
				return false;
			out.x = static_cast<float>(tRet->GetTexture()->GetWidth());
			out.y = static_cast<float>(tRet->GetTexture()->GetHeight());
			return true;
		}

		/// @brief 寻找精灵
		fcyRefPointer<ResSprite> FindSprite(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResSprite> tRet;
			if (!(tRet = m_StageResourcePool.GetSprite(name)))
				tRet = m_GlobalResourcePool.GetSprite(name);
			return tRet;
		}

		/// @brief 寻找动画
		fcyRefPointer<ResAnimation> FindAnimation(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResAnimation> tRet;
			if (!(tRet = m_StageResourcePool.GetAnimation(name)))
				tRet = m_GlobalResourcePool.GetAnimation(name);
			return tRet;
		}
	public:
		ResourceMgr();
	};
}
