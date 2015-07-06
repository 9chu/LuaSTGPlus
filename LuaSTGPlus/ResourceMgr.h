#pragma once
#include "Global.h"
#include "ObjectPool.hpp"
#include "Dictionary.hpp"

#ifdef LoadImage
#undef LoadImage
#endif

namespace LuaSTGPlus
{
	class ResourceMgr;

	inline float VolumeFix(float v)
	{
		return -exp(-v * 6.f) + 1;  // ����������С������
	}

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
		TrueTypeFont,
		FX
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
		AddSub,
		AddRev,
		MulAdd,
		MulAlpha,
		MulSub,
		MulRev
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

	/// @brief ����ϵͳ
	/// @note HGE����ϵͳ��f2dʵ��
	class ResParticle :
		public Resource
	{
	public:
		/// @brief ������Ϣ
		struct ParticleInfo
		{
			fuInt iBlendInfo;

			int nEmission;  // ÿ�뷢�����
			float fLifetime;  // ������
			float fParticleLifeMin;  // ������С������
			float fParticleLifeMax;  // �������������
			float fDirection;  // ���䷽��
			float fSpread;  // ƫ�ƽǶ�
			bool bRelative;  // ʹ�����ֵ���Ǿ���ֵ

			float fSpeedMin;  // �ٶ���Сֵ
			float fSpeedMax;  // �ٶ����ֵ

			float fGravityMin;  // ������Сֵ
			float fGravityMax;  // �������ֵ

			float fRadialAccelMin;  // ����߼��ٶ�
			float fRadialAccelMax;  // ����߼��ٶ�

			float fTangentialAccelMin;  // ��ͽǼ��ٶ�
			float fTangentialAccelMax;  // ��߽Ǽ��ٶ�

			float fSizeStart;  // ��ʼ��С
			float fSizeEnd;  // ���մ�С
			float fSizeVar;  // ��С����ֵ

			float fSpinStart;  // ��ʼ����
			float fSpinEnd;  // ��������
			float fSpinVar;  // ��������ֵ

			float colColorStart[4];  // ��ʼ��ɫ(rgba)
			float colColorEnd[4];  // ������ɫ
			float fColorVar;  // ��ɫ����ֵ
			float fAlphaVar;  // alpha����ֵ
		};
		/// @brief ����ʵ��
		struct ParticleInstance
		{
			fcyVec2 vecLocation;  // λ��
			fcyVec2 vecVelocity;  // �ٶ�

			float fGravity;  // ����
			float fRadialAccel;  // �߼��ٶ�
			float fTangentialAccel;  // �Ǽ��ٶ�

			float fSpin;  // ����
			float fSpinDelta;  // ��������

			float fSize;  // ��С
			float fSizeDelta;  // ��С����

			float colColor[4];  // ��ɫ
			float colColorDelta[4];  // ��ɫ����

			float fAge;  // ��ǰ���ʱ��
			float fTerminalAge;  // ��ֹʱ��
		};
		/// @brief ���ӳ�
		class ParticlePool
		{
			friend class ResParticle;
		public:
			enum class Status
			{
				Alive,
				Sleep
			};
		private:
			size_t m_iId;  // ����id
			fcyRefPointer<ResParticle> m_pInstance;  // ��Ϣ

			BlendMode m_BlendMode = BlendMode::MulAlpha;
			Status m_iStatus = Status::Alive;  // ״̬
			fcyVec2 m_vCenter;  // ����
			fcyVec2 m_vPrevCenter;  // ��һ������
			float m_fRotation = 0.f;  // ����
			size_t m_iAlive = 0;  // �����
			float m_fAge = 0.f;  // �Ѵ��ʱ��
			float m_fEmission = 0.f;  // ÿ�뷢����
			float m_fEmissionResidue = 0.f;  // �����������
			std::array<ParticleInstance, LPARTICLE_MAXCNT> m_ParticlePool;
		public:
			size_t GetAliveCount()const LNOEXCEPT { return m_iAlive; }
			BlendMode GetBlendMode()const LNOEXCEPT { return m_BlendMode; }
			void SetBlendMode(BlendMode m)LNOEXCEPT { m_BlendMode = m; }
			float GetEmission()const LNOEXCEPT { return m_fEmission; }
			void SetEmission(float e)LNOEXCEPT { m_fEmission = e; }
			void SetActive()LNOEXCEPT
			{
				m_iStatus = Status::Alive;
				m_fAge = 0.f;
				m_fEmissionResidue = 0.f;
			}
			void SetInactive()LNOEXCEPT
			{
				m_iStatus = Status::Sleep;
			}
			void SetCenter(fcyVec2 pos)LNOEXCEPT
			{
				if (m_iStatus == Status::Alive)
					m_vPrevCenter = m_vCenter;
				else
					m_vPrevCenter = pos;
				m_vCenter = pos;
			}
			void SetRotation(float r)LNOEXCEPT { m_fRotation = r; }
			void Update(float delta);
			void Render(f2dGraphics2D* graph, float scaleX, float scaleY);
		public:
			ParticlePool(size_t id, fcyRefPointer<ResParticle> ref);
		};
	private:
		struct PARTICLE_POD { char buf[sizeof(ParticlePool)]; };
		static FixedObjectPool<PARTICLE_POD, LPARTICLESYS_MAX> s_MemoryPool;

		fcyRefPointer<f2dSprite> m_BindedSprite;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		ParticleInfo m_ParticleInfo;
		double m_HalfSizeX = 0.;
		double m_HalfSizeY = 0.;
		bool m_bRectangle = false;
	public:
		ParticlePool* AllocInstance()LNOEXCEPT;
		void FreeInstance(ParticlePool* p)LNOEXCEPT;

		f2dSprite* GetBindedSprite()LNOEXCEPT { return m_BindedSprite; }
		const ParticleInfo& GetParticleInfo()const LNOEXCEPT { return m_ParticleInfo; }
		double GetHalfSizeX()const LNOEXCEPT { return m_HalfSizeX; }
		double GetHalfSizeY()const LNOEXCEPT { return m_HalfSizeY; }
		bool IsRectangle()const LNOEXCEPT { return m_bRectangle; }
	public:
		ResParticle(const char* name, const ParticleInfo& pinfo, fcyRefPointer<f2dSprite> sprite, BlendMode bld, double a, double b, bool rect = false);
	};

	/// @brief ��������
	class ResFont :
		public Resource
	{
	public:
		enum class FontAlignHorizontal  // ˮƽ����
		{
			Left,
			Center,
			Right
		};
		enum class FontAlignVertical  // ��ֱ����
		{
			Top,
			Middle,
			Bottom
		};

		class HGEFont :
			public fcyRefObjImpl<f2dFontProvider>
		{
		public:
			static void ReadDefine(const std::wstring& data, std::unordered_map<wchar_t, f2dGlyphInfo>& out, std::wstring& tex);
		private:
			fcyRefPointer<f2dTexture2D> m_pTex;
			std::unordered_map<wchar_t, f2dGlyphInfo> m_Charset;
			float m_fLineHeight;
		public:
			fFloat GetLineHeight();
			fFloat GetAscender();
			fFloat GetDescender();
			f2dTexture2D* GetCacheTexture();
			fResult CacheString(fcStrW String);
			fResult QueryGlyph(f2dGraphics* pGraph, fCharW Character, f2dGlyphInfo* InfoOut);
		public:
			HGEFont(std::unordered_map<wchar_t, f2dGlyphInfo>&& org, fcyRefPointer<f2dTexture2D> pTex);
		};
	private:
		fcyRefPointer<f2dFontProvider> m_pFontProvider;
		BlendMode m_BlendMode = BlendMode::MulAlpha;
		fcyColor m_BlendColor = fcyColor(0xFFFFFFFF);
	public:
		f2dFontProvider* GetFontProvider()LNOEXCEPT { return m_pFontProvider; }
		BlendMode GetBlendMode()const LNOEXCEPT { return m_BlendMode; }
		void SetBlendMode(BlendMode m)LNOEXCEPT { m_BlendMode = m; }
		fcyColor GetBlendColor()const LNOEXCEPT { return m_BlendColor; }
		void SetBlendColor(fcyColor c)LNOEXCEPT { m_BlendColor = c; }
	public:
		ResFont(const char* name, fcyRefPointer<f2dFontProvider> pFont);
	};

	/// @brief ��Ч
	class ResSound :
		public Resource
	{
	private:
		fcyRefPointer<f2dSoundBuffer> m_pBuffer;
	public:
		void Play(float vol, float pan)
		{
			m_pBuffer->Stop();

			float nv = VolumeFix(vol);
			if (m_pBuffer->GetVolume() != nv)
				m_pBuffer->SetVolume(nv);
			if (m_pBuffer->GetPan() != pan)
				m_pBuffer->SetPan(pan);

			m_pBuffer->Play();
		}
	public:
		ResSound(const char* name, fcyRefPointer<f2dSoundBuffer> buffer)
			: Resource(ResourceType::SoundEffect, name), m_pBuffer(buffer) {}
	};

	/// @brief ��������
	class ResMusic :
		public Resource
	{
	public:
		// ��SoundDecoder��һ����װ������BGMѭ��
		// ʹ�ø�Wrapper֮��SoundBuffer�Ĳ��Ų�����λ�ã���û������
		// ! ��fancystg���ѿӣ��г�ȡ���Ϲ�ʱ�ڴ���
		class BGMWrapper :
			public fcyRefObjImpl<f2dSoundDecoder>
		{
		protected:
			fcyRefPointer<f2dSoundDecoder> m_pDecoder;

			// ת������Ϊ��λ
			fuInt m_TotalSample;
			fuInt m_pLoopStartSample;
			fuInt m_pLoopEndSample; // �����ڣ�=EndSample+1
		public:
			// ֱ�ӷ���ԭʼ����
			fuInt GetBufferSize() { return m_pDecoder->GetBufferSize(); }
			fuInt GetAvgBytesPerSec() { return m_pDecoder->GetAvgBytesPerSec(); }
			fuShort GetBlockAlign() { return m_pDecoder->GetBlockAlign(); }
			fuShort GetChannelCount() { return m_pDecoder->GetChannelCount(); }
			fuInt GetSamplesPerSec() { return m_pDecoder->GetSamplesPerSec(); }
			fuShort GetFormatTag() { return m_pDecoder->GetFormatTag(); }
			fuShort GetBitsPerSample() { return m_pDecoder->GetBitsPerSample(); }

			// �����κδ���
			fLen GetPosition() { return m_pDecoder->GetPosition(); }
			fResult SetPosition(F2DSEEKORIGIN Origin, fInt Offset) { return m_pDecoder->SetPosition(Origin, Offset); }

			// ��Read������
			fResult Read(fData pBuffer, fuInt SizeToRead, fuInt* pSizeRead);
		public:
			BGMWrapper(fcyRefPointer<f2dSoundDecoder> pOrg, fDouble LoopStart, fDouble LoopEnd);
		};
	private:
		fcyRefPointer<f2dSoundBuffer> m_pBuffer;
	public:
		void Play(float vol, double position)
		{
			m_pBuffer->Stop();
			m_pBuffer->SetTime(position);

			float nv = VolumeFix(vol);
			if (m_pBuffer->GetVolume() != nv)
				m_pBuffer->SetVolume(nv);

			m_pBuffer->Play();
		}
		
		void Stop()
		{
			m_pBuffer->Stop();
		}
		
		void Pause()
		{
			m_pBuffer->Pause();
		}
		
		void Resume()
		{
			m_pBuffer->Play();
		}

		bool IsPlaying()
		{
			return m_pBuffer->IsPlaying();
		}

		bool IsStopped()
		{
			return !IsPlaying() && m_pBuffer->GetTime() == 0.;
		}

		void SetVolume(float v)
		{
			float nv = VolumeFix(v);
			if (m_pBuffer->GetVolume() != nv)
				m_pBuffer->SetVolume(nv);
		}
	public:
		ResMusic(const char* name, fcyRefPointer<f2dSoundBuffer> buffer)
			: Resource(ResourceType::Music, name), m_pBuffer(buffer) {}
	};

	/// @brief shader��װ
	class ResFX :
		public Resource
	{
	private:
		fcyRefPointer<f2dEffect> m_pShader;

		// ��������
		std::vector<f2dEffectParamValue*> m_pBindingPostEffectTexture;  // POSTEFFECTTEXTURE
		std::vector<f2dEffectParamValue*> m_pBindingViewport;  // VIEWPORT
		std::vector<f2dEffectParamValue*> m_pBindingScreenSize;  // SCREENSIZE

		// ������
		Dictionary<std::vector<f2dEffectParamValue*>> m_pBindingVar;
	public:
		f2dEffect* GetEffect()LNOEXCEPT { return m_pShader; }
		
		void SetPostEffectTexture(f2dTexture2D* val)LNOEXCEPT;
		void SetViewport(fcyRect rect)LNOEXCEPT;
		void SetScreenSize(fcyVec2 size)LNOEXCEPT;

		void SetValue(const char* key, float val)LNOEXCEPT;
		void SetValue(const char* key, fcyColor val)LNOEXCEPT;  // ��float4���а�
		void SetValue(const char* key, f2dTexture2D* val)LNOEXCEPT;
	public:
		ResFX(const char* name, fcyRefPointer<f2dEffect> shader);
	};

	/// @brief ��Դ��
	class ResourcePool
	{
	private:
		ResourceMgr* m_pMgr;
		ResourcePoolType m_iType;

		Dictionary<fcyRefPointer<ResTexture>> m_TexturePool;
		Dictionary<fcyRefPointer<ResSprite>> m_SpritePool;
		Dictionary<fcyRefPointer<ResAnimation>> m_AnimationPool;
		Dictionary<fcyRefPointer<ResSound>> m_MusicPool;
		Dictionary<fcyRefPointer<ResMusic>> m_SoundSpritePool;
		Dictionary<fcyRefPointer<ResParticle>> m_ParticlePool;
		Dictionary<fcyRefPointer<ResFont>> m_SpriteFontPool;
		Dictionary<fcyRefPointer<ResFont>> m_TTFFontPool;
		Dictionary<fcyRefPointer<ResFX>> m_FXPool;
	private:
		const wchar_t* getResourcePoolTypeName()
		{
			switch (m_iType)
			{
			case ResourcePoolType::Global:
				return L"ȫ����Դ��";
			case ResourcePoolType::Stage:
				return L"�ؿ���Դ��";
			default:
				return nullptr;
			}
		}
		template <typename T>
		void removeResource(Dictionary<fcyRefPointer<T>> pool, const char* name)
		{
			auto i = pool.find(name);
			if (i == pool.end())
				LWARNING("RemoveResource: ��ͼ�Ƴ�һ�������ڵ���Դ'%m'", name);
			pool.erase(i);
#ifdef LSHOWRESLOADINFO
			LINFO("RemoveResource: ��Դ'%m'��ж��", name);
#endif
		}
	public:
		/// @brief ��ն����
		void Clear()LNOEXCEPT
		{
			m_TexturePool.clear();
			m_SpritePool.clear();
			m_AnimationPool.clear();
			m_MusicPool.clear();
			m_SoundSpritePool.clear();
			m_ParticlePool.clear();
			m_SpriteFontPool.clear();
			m_TTFFontPool.clear();
			m_FXPool.clear();
		}

		/// @brief �Ƴ�ĳ����Դ���͵���Դ
		void RemoveResource(ResourceType t, const char* name)LNOEXCEPT
		{
			switch (t)
			{
			case ResourceType::Texture:
				removeResource(m_TexturePool, name);
				break;
			case ResourceType::Sprite:
				removeResource(m_SpritePool, name);
				break;
			case ResourceType::Animation:
				removeResource(m_AnimationPool, name);
				break;
			case ResourceType::Music:
				removeResource(m_MusicPool, name);
				break;
			case ResourceType::SoundEffect:
				removeResource(m_SoundSpritePool, name);
				break;
			case ResourceType::Particle:
				removeResource(m_ParticlePool, name);
				break;
			case ResourceType::SpriteFont:
				removeResource(m_SpriteFontPool, name);
				break;
			case ResourceType::TrueTypeFont:
				removeResource(m_TTFFontPool, name);
				break;
			case ResourceType::FX:
				removeResource(m_FXPool, name);
				break;
			default:
				break;
			}
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
				return m_MusicPool.find(name.c_str()) != m_MusicPool.end();
			case ResourceType::SoundEffect:
				return m_SoundSpritePool.find(name.c_str()) != m_SoundSpritePool.end();
			case ResourceType::Particle:
				return m_ParticlePool.find(name.c_str()) != m_ParticlePool.end();
			case ResourceType::SpriteFont:
				return m_SpriteFontPool.find(name.c_str()) != m_SpriteFontPool.end();
			case ResourceType::TrueTypeFont:
				return m_TTFFontPool.find(name.c_str()) != m_TTFFontPool.end();
			case ResourceType::FX:
				return m_FXPool.find(name.c_str()) != m_FXPool.end();
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

		/// @brief װ�ر�������
		bool LoadMusic(const char* name, const std::wstring& path, double start, double end)LNOEXCEPT;

		LNOINLINE bool LoadMusic(const char* name, const char* path, double start, double end)LNOEXCEPT;

		/// @brief װ����Ч
		bool LoadSound(const char* name, const std::wstring& path)LNOEXCEPT;

		LNOINLINE bool LoadSound(const char* name, const char* path)LNOEXCEPT;

		/// @brief װ������
		bool LoadParticle(const char* name, const std::wstring& path, const char* img_name, double a, double b, bool rect = false)LNOEXCEPT;

		LNOINLINE bool LoadParticle(const char* name, const char* path, const char* img_name, double a, double b, bool rect = false)LNOEXCEPT;

		/// @brief װ����������(HGE)
		bool LoadSpriteFont(const char* name, const std::wstring& path, bool mipmaps = true)LNOEXCEPT;

		/// @brief װ����������(fancy2d)
		bool LoadSpriteFont(const char* name, const std::wstring& path, const std::wstring& tex_path, bool mipmaps = true)LNOEXCEPT;

		LNOINLINE bool LoadSpriteFont(const char* name, const char* path, bool mipmaps = true)LNOEXCEPT;

		LNOINLINE bool LoadSpriteFont(const char* name, const char* path, const char* tex_path, bool mipmaps = true)LNOEXCEPT;

		/// @brief װ��TTF����
		bool LoadTTFFont(const char* name, const std::wstring& path, float width, float height)LNOEXCEPT;

		LNOINLINE bool LoadTTFFont(const char* name, const char* path, float width, float height)LNOEXCEPT;

		/// @brief װ��FX
		bool LoadFX(const char* name, const std::wstring& path)LNOEXCEPT;

		LNOINLINE bool LoadFX(const char* name, const char* path)LNOEXCEPT;

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

		/// @brief ��ȡ������
		fcyRefPointer<ResMusic> GetMusic(const char* name)LNOEXCEPT
		{
			auto i = m_MusicPool.find(name);
			if (i == m_MusicPool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief ��ȡ��Ч
		fcyRefPointer<ResSound> GetSound(const char* name)LNOEXCEPT
		{
			auto i = m_SoundSpritePool.find(name);
			if (i == m_SoundSpritePool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief ��ȡ����ϵͳ
		fcyRefPointer<ResParticle> GetParticle(const char* name)LNOEXCEPT
		{
			auto i = m_ParticlePool.find(name);
			if (i == m_ParticlePool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief ��ȡ��������
		fcyRefPointer<ResFont> GetSpriteFont(const char* name)LNOEXCEPT
		{
			auto i = m_SpriteFontPool.find(name);
			if (i == m_SpriteFontPool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief ��ȡTTF����
		fcyRefPointer<ResFont> GetTTFFont(const char* name)LNOEXCEPT
		{
			auto i = m_TTFFontPool.find(name);
			if (i == m_TTFFontPool.end())
				return nullptr;
			else
				return i->second;
		}

		/// @brief ��ȡFX
		fcyRefPointer<ResFX> GetFX(const char* name)LNOEXCEPT
		{
			auto i = m_FXPool.find(name);
			if (i == m_FXPool.end())
				return nullptr;
			else
				return i->second;
		}
	private:
		ResourcePool& operator=(const ResourcePool&);
		ResourcePool(const ResourcePool&);
	public:
		ResourcePool(ResourceMgr* mgr, ResourcePoolType t)
			: m_pMgr(mgr), m_iType(t) {}
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
		float m_GlobalSoundEffectVolume = 1.0f;
		float m_GlobalMusicVolume = 1.0f;

		ResourcePoolType m_ActivedPool = ResourcePoolType::Global;
		ResourcePool m_GlobalResourcePool;
		ResourcePool m_StageResourcePool;
	public:
		float GetGlobalImageScaleFactor()const LNOEXCEPT{ return m_GlobalImageScaleFactor; }
		void SetGlobalImageScaleFactor(float s)LNOEXCEPT{ m_GlobalImageScaleFactor = s; }
		float GetGlobalSoundEffectVolume()const LNOEXCEPT{ return m_GlobalSoundEffectVolume; }
		void SetGlobalSoundEffectVolume(float s)LNOEXCEPT{ m_GlobalSoundEffectVolume = s; }
		float GetGlobalMusicVolume()const LNOEXCEPT{ return m_GlobalMusicVolume; }
		void SetGlobalMusicVolume(float s)LNOEXCEPT{ m_GlobalMusicVolume = s; }

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
			m_GlobalSoundEffectVolume = 1.0f;
			m_GlobalMusicVolume = 1.0f;
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

		/// @brief Ѱ������
		fcyRefPointer<ResMusic> FindMusic(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResMusic> tRet;
			if (!(tRet = m_StageResourcePool.GetMusic(name)))
				tRet = m_GlobalResourcePool.GetMusic(name);
			return tRet;
		}

		/// @brief Ѱ����Ч
		fcyRefPointer<ResSound> FindSound(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResSound> tRet;
			if (!(tRet = m_StageResourcePool.GetSound(name)))
				tRet = m_GlobalResourcePool.GetSound(name);
			return tRet;
		}

		/// @brief Ѱ������
		fcyRefPointer<ResParticle> FindParticle(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResParticle> tRet;
			if (!(tRet = m_StageResourcePool.GetParticle(name)))
				tRet = m_GlobalResourcePool.GetParticle(name);
			return tRet;
		}

		/// @brief Ѱ������
		fcyRefPointer<ResFont> FindSpriteFont(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResFont> tRet;
			if (!(tRet = m_StageResourcePool.GetSpriteFont(name)))
				tRet = m_GlobalResourcePool.GetSpriteFont(name);
			return tRet;
		}

		/// @brief Ѱ������
		fcyRefPointer<ResFont> FindTTFFont(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResFont> tRet;
			if (!(tRet = m_StageResourcePool.GetTTFFont(name)))
				tRet = m_GlobalResourcePool.GetTTFFont(name);
			return tRet;
		}

		/// @brief Ѱ��shader
		fcyRefPointer<ResFX> FindFX(const char* name)LNOEXCEPT
		{
			fcyRefPointer<ResFX> tRet;
			if (!(tRet = m_StageResourcePool.GetFX(name)))
				tRet = m_GlobalResourcePool.GetFX(name);
			return tRet;
		}
	public:
		ResourceMgr();
	};
}
