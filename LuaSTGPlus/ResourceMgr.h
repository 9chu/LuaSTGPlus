#pragma once
#include "Global.h"
#include "Dictionary.hpp"

#ifdef LoadImage
#undef LoadImage
#endif

namespace LuaSTGPlus
{
	class ResourceMgr;

	/// @brief ��Դ����
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

	/// @brief ��Դ������
	enum class ResourcePoolType
	{
		None,
		Global,
		Stage
	};

	/// @brief ���ģʽ
	enum class BlendMode
	{
		AddAdd = 1,
		AddAlpha,
		MulAdd,
		MulAlpha
	};
	
	/// @brief ��Դ�ӿ�
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

	/// @brief ������Դ
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

	/// @brief ͼ����Դ
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
			m_Sprite->SetColor(0xFFFFFFFF);  // ��Ӧ�˷�
		}
	};

	/// @brief ������Դ
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

	/// @brief ��Դ��
	class ResourcePool
	{
	private:
		ResourceMgr* m_pMgr;

		Dictionary<fcyRefPointer<ResTexture>> m_TexturePool;
		Dictionary<fcyRefPointer<ResSprite>> m_SpritePool;
		Dictionary<fcyRefPointer<ResAnimation>> m_AnimationPool;
	public:
		/// @brief ��ն����
		void Clear()LNOEXCEPT
		{
			m_TexturePool.clear();
			m_SpritePool.clear();
			m_AnimationPool.clear();
		}

		/// @brief �����Դ�Ƿ����
		/// @warning ע��t�����ǷǷ�ö����
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

		/// @brief ������Դ��
		/// @note ��L�Ķ�ջ�Ϸ���һ��table���Դ��ResourceType�е���Դ����
		/// @warning ע��t�����ǷǷ�ö����
		void ExportResourceList(lua_State* L, ResourceType t)const LNOEXCEPT;

		/// @brief װ������
		/// @param name ����
		/// @param path ·��
		/// @param mipmaps ������
		bool LoadTexture(const char* name, const std::wstring& path, bool mipmaps = true)LNOEXCEPT;

		/// @brief װ������UTF-8��
		LNOINLINE bool LoadTexture(const char* name, const char* path, bool mipmaps = true)LNOEXCEPT;
		
		/// @brief װ��ͼ��
		LNOINLINE bool LoadImage(const char* name, const char* texname,
			double x, double y, double w, double h, double a, double b, bool rect = false)LNOEXCEPT;

		LNOINLINE bool LoadAnimation(const char* name, const char* texname,
			double x, double y, double w, double h, int n, int m, int intv, double a, double b, bool rect = false)LNOEXCEPT;

		/// @brief ��ȡ����
		fcyRefPointer<ResTexture> GetTexture(const char* name)LNOEXCEPT
		{
			auto i = m_TexturePool.find(name);
			if (i == m_TexturePool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief ��ȡ����
		fcyRefPointer<ResSprite> GetSprite(const char* name)LNOEXCEPT
		{
			auto i = m_SpritePool.find(name);
			if (i == m_SpritePool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief ��ȡ����
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

		float m_GlobalImageScaleFactor = 1.0f;
		ResourcePoolType m_ActivedPool = ResourcePoolType::Global;
		ResourcePool m_GlobalResourcePool;
		ResourcePool m_StageResourcePool;
	public:
		float GetGlobalImageScaleFactor()const LNOEXCEPT{ return m_GlobalImageScaleFactor; }
		void SetGlobalImageScaleFactor(float s)LNOEXCEPT{ m_GlobalImageScaleFactor = s; }

		/// @brief ��õ�ǰ�������Դ������
		ResourcePoolType GetActivedPoolType()LNOEXCEPT
		{
			return m_ActivedPool;
		}

		/// @brief ���õ�ǰ�������Դ������
		void SetActivedPoolType(ResourcePoolType t)LNOEXCEPT
		{
			m_ActivedPool = t;
		}

		/// @brief ��õ�ǰ�������Դ��
		ResourcePool* GetActivedPool()LNOEXCEPT
		{
			return GetResourcePool(m_ActivedPool);
		}

		/// @brief �����Դ��
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

		/// @brief ������Դ��
		/// @param[in] path ·��
		/// @param[in] passwd ����
		bool LoadPack(const wchar_t* path, const char* passwd)LNOEXCEPT;

		/// @brief ж����Դ��
		/// @param[in] path ·��
		void UnloadPack(const wchar_t* path)LNOEXCEPT;

		/// @brief ж��������Դ��
		void UnloadAllPack()LNOEXCEPT { m_ResPackList.clear(); }

		/// @brief ж��������Դ������״̬
		void ClearAllResource()LNOEXCEPT
		{
			m_GlobalResourcePool.Clear();
			m_StageResourcePool.Clear();
			m_ActivedPool = ResourcePoolType::Global;
			m_GlobalImageScaleFactor = 1.;
		}

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

		/// @brief Ѱ������
		fcyRefPointer<ResTexture> FindTexture(const char* texname)LNOEXCEPT
		{
			fcyRefPointer<ResTexture> tRet;
			if (!(tRet = m_StageResourcePool.GetTexture(texname)))
				tRet = m_GlobalResourcePool.GetTexture(texname);
			return tRet;
		}

		/// @brief ��ȡ�����С
		bool GetTextureSize(const char* texname, fcyVec2& out)LNOEXCEPT
		{
			fcyRefPointer<ResTexture> tRet = FindTexture(texname);
			if (!tRet)
				return false;
			out.x = static_cast<float>(tRet->GetTexture()->GetWidth());
			out.y = static_cast<float>(tRet->GetTexture()->GetHeight());
			return true;
		}

		/// @brief Ѱ�Ҿ���
		fcyRefPointer<ResSprite> FindSprite(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResSprite> tRet;
			if (!(tRet = m_StageResourcePool.GetSprite(name)))
				tRet = m_GlobalResourcePool.GetSprite(name);
			return tRet;
		}

		/// @brief Ѱ�Ҷ���
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
