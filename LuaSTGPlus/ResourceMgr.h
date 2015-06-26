#pragma once
#include "Global.h"

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

	/// @brief ��Դ��
	class ResourcePool
	{
	public:
		template <typename T>
		using PoolType = std::unordered_map<std::string, typename T>;

		struct SpriteEx
		{
			fcyRefPointer<f2dSprite> sprite;  // �������
			BlendMode blend = BlendMode::AddAlpha;  // ���
			double a = 0., b = 0.;  // ��ײ���᳤
			bool rect = false;  // �Ƿ�Ϊ������ײ��

			SpriteEx() {}
			SpriteEx(const SpriteEx& org)
				: sprite(org.sprite), blend(org.blend), a(org.a), b(org.b), rect(org.rect) {}
		};
	private:
		ResourceMgr* m_pMgr;

		PoolType<fcyRefPointer<f2dTexture2D>> m_TexturePool;
		PoolType<SpriteEx> m_SpritePool;
	public:
		/// @brief ��ն����
		void Clear()LNOEXCEPT;

		/// @brief �����Դ�Ƿ����
		/// @warning ע��t�����ǷǷ�ö����
		bool CheckResourceExists(ResourceType t, const std::string& name)const LNOEXCEPT;

		/// @brief ������Դ��
		/// @note ��L�Ķ�ջ�Ϸ���һ��table���Դ��ResourceType�е���Դ����
		/// @warning ע��t�����ǷǷ�ö����
		void ExportResourceList(lua_State* L, ResourceType t)const LNOEXCEPT;

		/// @brief װ������
		/// @param name ����
		/// @param path ·��
		/// @param mipmaps ������
		bool LoadTexture(const std::string& name, const std::wstring& path, bool mipmaps = true)LNOEXCEPT;

		/// @brief װ������UTF-8��
		LNOINLINE bool LoadTexture(const char* name, const char* path, bool mipmaps = true)LNOEXCEPT;
		
		/// @brief װ��ͼ��
		LNOINLINE bool LoadImage(const char* name, const char* texname,
			double x, double y, double w, double h, double a, double b, bool rect = false)LNOEXCEPT;

		/// @brief ��ȡ����
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

		ResourcePoolType m_ActivedPool = ResourcePoolType::Global;
		ResourcePool m_GlobalResourcePool;
		ResourcePool m_StageResourcePool;
	public:
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

		/// @brief ж��������Դ
		void ClearAllResource()LNOEXCEPT
		{
			m_GlobalResourcePool.Clear();
			m_StageResourcePool.Clear();
			m_ActivedPool = ResourcePoolType::Global;
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
