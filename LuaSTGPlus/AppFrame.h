/// @file AppFrame.h
/// @brief ����Ӧ�ó�����
#pragma once
#include "Global.h"
#include "SplashWindow.h"
#include "ResourceMgr.h"
#include "GameObjectPool.h"
#include "UnicodeStringEncoding.h"

#if (defined LDEVVERSION) || (defined LDEBUG)
#include "RemoteDebuggerClient.h"
#endif

namespace LuaSTGPlus
{
	/// @brief Ӧ�ó���״̬
	enum class AppStatus
	{
		NotInitialized,
		Initializing,
		Initialized,
		Running,
		Aborted,
		Destroyed
	};

	/// @brief ��ǰ�������Ⱦ��
	enum class GraphicsType
	{
		Graph2D,
		Graph3D
	};

	/// @brief Ӧ�ó�����
	class AppFrame :
		public f2dEngineEventListener
	{
	public:
		static LNOINLINE AppFrame& GetInstance();
	private:
		class GdiPlusScope
		{
		private:
			ULONG_PTR m_gdiplusToken;
		public:
			GdiPlusScope()
			{
				Gdiplus::GdiplusStartupInput StartupInput;
				GdiplusStartup(&m_gdiplusToken, &StartupInput, NULL);
			}
			~GdiPlusScope()
			{
				Gdiplus::GdiplusShutdown(m_gdiplusToken);
			}
		};
	private:
		AppStatus m_iStatus = AppStatus::NotInitialized;

#if (defined LDEVVERSION) || (defined LDEBUG)
		// Զ�˵�����
		std::unique_ptr<RemoteDebuggerClient> m_DebuggerClient;

		// ���ܼ�����
		float m_UpdateTimer = 0.f;
		float m_RenderTimer = 0.f;

		float m_PerformanceUpdateTimer = 0.f;  // ��¼���ܲ������ۼƲ���ʱ��
		float m_PerformanceUpdateCounter = 0.f;  // ��¼��������
		float m_FPSTotal = 0.f;  // ��¼�ڲ���ʱ�����ۼƵ�FPS
		float m_ObjectTotal = 0.f;  // ��¼�ڲ���ʱ�����ۼƵĶ�����
		float m_UpdateTimerTotal = 0.f;  // ��¼�ڲ���ʱ�����ۼƵĸ���ʱ��
		float m_RenderTimerTotal = 0.f;  // ��¼�ڲ���ʱ�����ۼƵ���Ⱦʱ��

		bool m_bShowCollider = false;
#endif

		// ���봰��
		GdiPlusScope m_GdiScope;
		SplashWindow m_SplashWindow;

		// ��Դ������
		ResourceMgr m_ResourceMgr;

		// �����
		std::unique_ptr<GameObjectPool> m_GameObjectPool;

		// Lua�����
		lua_State* L = nullptr;

		// ѡ����ֵ
		bool m_bSplashWindowEnabled = false;
		bool m_OptionWindowed = true;
		fuInt m_OptionFPSLimit = 60;
		bool m_OptionVsync = true;
		fcyVec2 m_OptionResolution = fcyVec2(640.f, 480.f);
		bool m_OptionSplash = false;
		std::wstring m_OptionTitle = L"LuaSTGPlus";
		fDouble m_fFPS = 0.;

		// ����
		fcyRefPointer<f2dEngine> m_pEngine;
		f2dWindow* m_pMainWindow = nullptr;
		f2dRenderer* m_pRenderer = nullptr;
		f2dRenderDevice* m_pRenderDev = nullptr;
		f2dSoundSys* m_pSoundSys = nullptr;
		f2dInputSys* m_pInputSys = nullptr;
		
		GraphicsType m_GraphType = GraphicsType::Graph2D;
		bool m_bRenderStarted = false;

		// 2Dģʽ
		BlendMode m_Graph2DLastBlendMode;
		f2dBlendState m_Graph2DBlendState;
		F2DGRAPH2DBLENDTYPE m_Graph2DColorBlendState;
		fcyRefPointer<f2dGeometryRenderer> m_GRenderer;
		fcyRefPointer<f2dFontRenderer> m_FontRenderer;
		fcyRefPointer<f2dGraphics2D> m_Graph2D;

		// 3Dģʽ
		BlendMode m_Graph3DLastBlendMode;
		f2dBlendState m_Graph3DBlendState;
		fcyRefPointer<f2dGraphics3D> m_Graph3D;

		// PostEffect����
		bool m_bPostEffectCaptureStarted = false;
		fcyRefPointer<f2dTexture2D> m_PostEffectBuffer;

		// RenderTarget����
		std::vector<fcyRefPointer<f2dTexture2D>> m_stRenderTargetStack;

		fcyRefPointer<f2dInputKeyboard> m_Keyboard;
		fcyRefPointer<f2dInputJoystick> m_Joystick[2];
		fCharW m_LastChar;
		fInt m_LastKey;
		fBool m_KeyStateMap[256];
		fcyVec2 m_MousePosition;
		fBool m_MouseState[3];
	private:
		void updateGraph2DBlendMode(BlendMode m)
		{
			if (m != m_Graph2DLastBlendMode)
			{
				switch (m)
				{
				case BlendMode::AddAdd:
					m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
					m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
					m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
					break;
				case BlendMode::AddSub:
					m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
					m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_SUBTRACT;
					m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
					break;
				case BlendMode::AddRev:
					m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
					m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_REVSUBTRACT;
					m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
					break;
				case BlendMode::MulAdd:
					m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
					m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
					m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_MODULATE;
					break;
				case BlendMode::MulAlpha:
					m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
					m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
					m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_MODULATE;
					break;
				case BlendMode::MulSub:
					m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
					m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_SUBTRACT;
					m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_MODULATE;
					break;
				case BlendMode::MulRev:
					m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
					m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_REVSUBTRACT;
					m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_MODULATE;
					break;
				case BlendMode::AddAlpha:
				default:
					m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
					m_Graph2DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
					m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
					break;
				}
				m_Graph2DLastBlendMode = m;
				m_Graph2D->SetBlendState(m_Graph2DBlendState);
				m_Graph2D->SetColorBlendType(m_Graph2DColorBlendState);
			}
		}
		void updateGraph3DBlendMode(BlendMode m)
		{
			if (m != m_Graph3DLastBlendMode)
			{
				switch (m)
				{
				case BlendMode::AddAdd:
				case BlendMode::MulAdd:
					m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
					m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
					break;
				case BlendMode::AddSub:
				case BlendMode::MulSub:
					m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
					m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_SUBTRACT;
					break;
				case BlendMode::AddRev:
				case BlendMode::MulRev:
					m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
					m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_REVSUBTRACT;
					break;
				case BlendMode::AddAlpha:
				case BlendMode::MulAlpha:
				default:
					m_Graph3DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
					m_Graph3DBlendState.BlendOp = F2DBLENDOPERATOR_ADD;
					break;
				}
				m_Graph3DLastBlendMode = m;
				m_Graph3D->SetBlendState(m_Graph3DBlendState);
			}
		}
#if (defined LDEVVERSION) || (defined LDEBUG)
	public: // �����ýӿ�
		void SendResourceLoadedHint(ResourceType Type, ResourcePoolType PoolType, const char* Name, const wchar_t* Path, float LoadingTime)
		{
			if (m_DebuggerClient)
				m_DebuggerClient->SendResourceLoadedHint(Type, PoolType, Name, Path, LoadingTime);
		}
		void SendResourceRemovedHint(ResourceType Type, ResourcePoolType PoolType, const char* Name)
		{
			if (m_DebuggerClient)
				m_DebuggerClient->SendResourceRemovedHint(Type, PoolType, Name);
		}
		void SendResourceClearedHint(ResourcePoolType PoolType)
		{
			if (m_DebuggerClient)
				m_DebuggerClient->SendResourceClearedHint(PoolType);
		}
#endif
	public: // �ű����ýӿڣ�����μ�API�ĵ�
		LNOINLINE void ShowSplashWindow(const char* imgPath = nullptr)LNOEXCEPT;  // UTF8����

		void SetWindowed(bool v)LNOEXCEPT;
		void SetFPS(fuInt v)LNOEXCEPT;
		void SetVsync(bool v)LNOEXCEPT;
		void SetResolution(fuInt width, fuInt height)LNOEXCEPT;
		void SetSplash(bool v)LNOEXCEPT;
		LNOINLINE void SetTitle(const char* v)LNOEXCEPT;  // UTF8����

		/// @brief ʹ���µ���Ƶ����������ʾģʽ
		/// @note ���л�ʧ������лع�
		LNOINLINE bool ChangeVideoMode(int width, int height, bool windowed, bool vsync)LNOEXCEPT;

		/// @brief ��ȡ��ǰ��FPS
		double GetFPS()LNOEXCEPT { return m_fFPS; }

		/// @brief ִ����Դ���е��ļ�
		/// @note �ú���Ϊ�ű�ϵͳʹ��
		LNOINLINE void LoadScript(const char* path)LNOEXCEPT;

		/// @brief ��鰴���Ƿ���
		fBool GetKeyState(int VKCode)LNOEXCEPT;

		/// @brief ������һ���ַ����루UTF-8��
		LNOINLINE int GetLastChar(lua_State* L)LNOEXCEPT;

		/// @brief ������һ�ΰ�������
		int GetLastKey()LNOEXCEPT { return m_LastKey; }

		/// @brief ��ȡ���λ�ã��Դ������½�Ϊԭ�㣩
		fcyVec2 GetMousePosition()LNOEXCEPT { return m_MousePosition; }

		/// @brief �������Ƿ���
		fBool GetMouseState(int button)LNOEXCEPT
		{
			if (button >= 0 && button < 3)
				return m_MouseState[button];
			return false;
		}
	public:  // ��Ⱦ���ӿ�
		/// @brief ֪ͨ��ʼ��Ⱦ
		bool BeginScene()LNOEXCEPT;

		/// @brief ֪ͨ������Ⱦ
		bool EndScene()LNOEXCEPT;

		/// @brief ����
		void ClearScreen(const fcyColor& c)LNOEXCEPT
		{
			m_pRenderDev->Clear(c);
		}

		/// @brief �����ӿ�
		bool SetViewport(double left, double right, double bottom, double top)LNOEXCEPT
		{
			if (FCYFAILED(m_pRenderDev->SetViewport(fcyRect(
				static_cast<float>((int)left),
				static_cast<float>((int)m_pRenderDev->GetBufferHeight() - (int)top),
				static_cast<float>((int)right),
				static_cast<float>((int)m_pRenderDev->GetBufferHeight() - (int)bottom)
			))))
			{
				LERROR("�����ӿ�(left: %lf, right: %lf, bottom: %lf, top: %lf)ʧ��", left, right, bottom, top);
				return false;
			}
			return true;
		}

		/// @brief ������ͶӰ����
		void SetOrtho(float left, float right, float bottom, float top)LNOEXCEPT
		{
			if (m_GraphType == GraphicsType::Graph2D)
			{
				// luastg��lua�����Ѿ�������������
				// m_Graph2D->SetWorldTransform(fcyMatrix4::GetTranslateMatrix(fcyVec3(-0.5f, -0.5f, 0.f)));
				m_Graph2D->SetWorldTransform(fcyMatrix4::GetIdentity());
				m_Graph2D->SetViewTransform(fcyMatrix4::GetIdentity());
				m_Graph2D->SetProjTransform(fcyMatrix4::GetOrthoOffCenterLH(left, right, bottom, top, 0.f, 100.f));
			}
		}

		/// @brief ����͸��ͶӰ����
		void SetPerspective(float eyeX, float eyeY, float eyeZ, float atX, float atY, float atZ, 
			float upX, float upY, float upZ, float fovy, float aspect, float zn, float zf)LNOEXCEPT
		{
			if (m_GraphType == GraphicsType::Graph2D)
			{
				m_Graph2D->SetWorldTransform(fcyMatrix4::GetIdentity());
				m_Graph2D->SetViewTransform(fcyMatrix4::GetLookAtLH(fcyVec3(eyeX, eyeY, eyeZ), fcyVec3(atX, atY, atZ), fcyVec3(upX, upY, upZ)));
				m_Graph2D->SetProjTransform(fcyMatrix4::GetPespctiveLH(aspect, fovy, zn, zf));
			}
		}

		/// @brief ������ֵ
		/// @note ��չ������������Ƴ���
		void SetFog(float start, float end, fcyColor color);

		/// @brief ��Ⱦͼ��
		bool Render(ResSprite* p, float x, float y, float rot = 0, float hscale = 1, float vscale = 1, float z = 0.5)LNOEXCEPT
		{
			LASSERT(p);
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render: ֻ��2D��Ⱦ������ִ�и÷���");
				return false;
			}

			// ���û��
			updateGraph2DBlendMode(p->GetBlendMode());

			// ��Ⱦ
			f2dSprite* pSprite = p->GetSprite();
			pSprite->SetZ(z);
			pSprite->Draw2(m_Graph2D, fcyVec2(x, y), fcyVec2(hscale, vscale), rot, false);
			return true;
		}

		/// @brief ��Ⱦ����
		bool Render(ResAnimation* p, int ani_timer, float x, float y, float rot = 0, float hscale = 1, float vscale = 1)LNOEXCEPT
		{
			LASSERT(p);
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render: ֻ��2D��Ⱦ������ִ�и÷���");
				return false;
			}

			// ���û��
			updateGraph2DBlendMode(p->GetBlendMode());

			// ��Ⱦ
			f2dSprite* pSprite = p->GetSprite(((fuInt)ani_timer / p->GetInterval()) % p->GetCount());
			pSprite->Draw2(m_Graph2D, fcyVec2(x, y), fcyVec2(hscale, vscale), rot, false);
			return true;
		}

		/// @brief ��Ⱦ����
		bool Render(ResParticle::ParticlePool* p, float hscale = 1, float vscale = 1)LNOEXCEPT
		{
			LASSERT(p);
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render: ֻ��2D��Ⱦ������ִ�и÷���");
				return false;
			}

			// ���û��
			updateGraph2DBlendMode(p->GetBlendMode());

			// ��Ⱦ
			p->Render(m_Graph2D, hscale, vscale);
			return true;
		}

		/// @brief ��Ⱦͼ��
		bool Render(const char* name, float x, float y, float rot = 0, float hscale = 1, float vscale = 1, float z = 0.5)LNOEXCEPT
		{
			fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
			if (!p)
			{
				LERROR("Render: �Ҳ���ͼ����Դ'%m'", name);
				return false;
			}
			return Render(p, x, y, rot, hscale, vscale, z);
		}

		/// @brief ��Ⱦͼ��
		bool RenderRect(const char* name, float x1, float y1, float x2, float y2)LNOEXCEPT
		{
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("RenderRect: ֻ��2D��Ⱦ������ִ�и÷���");
				return false;
			}

			fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
			if (!p)
			{
				LERROR("RenderRect: �Ҳ���ͼ����Դ'%m'", name);
				return false;
			}

			// ���û��
			updateGraph2DBlendMode(p->GetBlendMode());

			// ��Ⱦ
			f2dSprite* pSprite = p->GetSprite();
			pSprite->SetZ(0.5f);
			pSprite->Draw(m_Graph2D, fcyRect(x1, y1, x2, y2), false);
			return true;
		}

		/// @brief ��Ⱦͼ��
		bool Render4V(const char* name, float x1, float y1, float z1, float x2, float y2, float z2, 
			float x3, float y3, float z3, float x4, float y4, float z4)LNOEXCEPT
		{
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render4V: ֻ��2D��Ⱦ������ִ�и÷���");
				return false;
			}

			fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
			if (!p)
			{
				LERROR("Render4V: �Ҳ���ͼ����Դ'%m'", name);
				return false;
			}
			
			// ���û��
			updateGraph2DBlendMode(p->GetBlendMode());

			f2dSprite* pSprite = p->GetSprite();
			pSprite->SetZ(0.5f);
			pSprite->Draw(m_Graph2D, fcyVec3(x1, y1, z1), fcyVec3(x2, y2, z2), fcyVec3(x3, y3, z3), fcyVec3(x4, y4, z4), false);
			return true;
		}

		/// @brief ��Ⱦ����
		bool RenderTexture(ResTexture* tex, BlendMode blend, const f2dGraphics2DVertex vertex[])LNOEXCEPT
		{
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("RenderTexture: ֻ��2D��Ⱦ������ִ�и÷���");
				return false;
			}
			
			// ���û��
			updateGraph2DBlendMode(blend);

			// ��������
			f2dGraphics2DVertex tVertex[4];
			memcpy(tVertex, vertex, sizeof(tVertex));

			// ����UV��[0,1]����
			for (int i = 0; i < 4; ++i)
			{
				tVertex[i].u /= (float)tex->GetTexture()->GetWidth();
				tVertex[i].v /= (float)tex->GetTexture()->GetHeight();
			}

			m_Graph2D->DrawQuad(tex->GetTexture(), tVertex, false);
			return true;
		}

		/// @brief ��Ⱦ����
		bool RenderTexture(const char* name, BlendMode blend, f2dGraphics2DVertex vertex[])LNOEXCEPT
		{
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("RenderTexture: ֻ��2D��Ⱦ������ִ�и÷���");
				return false;
			}

			fcyRefPointer<ResTexture> p = m_ResourceMgr.FindTexture(name);
			if (!p)
			{
				LERROR("RenderTexture: �Ҳ���������Դ'%m'", name);
				return false;
			}

			// ���û��
			updateGraph2DBlendMode(blend);

			// ����UV��[0,1]����
			for (int i = 0; i < 4; ++i)
			{
				vertex[i].u /= (float)p->GetTexture()->GetWidth();
				vertex[i].v /= (float)p->GetTexture()->GetHeight();
			}	

			m_Graph2D->DrawQuad(p->GetTexture(), vertex, false);
			return true;
		}

		/// @brief ��Ⱦ����
		bool RenderText(ResFont* p, wchar_t* strBuf, fcyRect rect, fcyVec2 scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign, bool bWordBreak)LNOEXCEPT;

		fcyVec2 CalcuTextSize(ResFont* p, const wchar_t* strBuf, fcyVec2 scale)LNOEXCEPT;

		LNOINLINE bool RenderText(const char* name, const char* str, float x, float y, float scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign)LNOEXCEPT;

		LNOINLINE bool RenderTTF(const char* name, const char* str, float left, float right, float bottom, float top, float scale, int format, fcyColor c)LNOEXCEPT;

		LNOINLINE void SnapShot(const char* path)LNOEXCEPT;

		bool CheckRenderTargetInUse(fcyRefPointer<f2dTexture2D> rt)LNOEXCEPT;

		bool CheckRenderTargetInUse(ResTexture* rt)LNOEXCEPT;

		bool PushRenderTarget(fcyRefPointer<f2dTexture2D> rt)LNOEXCEPT;

		LNOINLINE bool PushRenderTarget(ResTexture* rt)LNOEXCEPT;

		LNOINLINE bool PopRenderTarget()LNOEXCEPT;

		bool PostEffect(fcyRefPointer<f2dTexture2D> rt, ResFX* shader, BlendMode blend)LNOEXCEPT;

		LNOINLINE bool PostEffect(ResTexture* rt, ResFX* shader, BlendMode blend)LNOEXCEPT;

		LNOINLINE bool PostEffectCapture()LNOEXCEPT;

		LNOINLINE bool PostEffectApply(ResFX* shader, BlendMode blend)LNOEXCEPT;
	public:
		ResourceMgr& GetResourceMgr()LNOEXCEPT { return m_ResourceMgr; }
		GameObjectPool& GetGameObjectPool()LNOEXCEPT{ return *m_GameObjectPool.get(); }
		f2dEngine* GetEngine()LNOEXCEPT { return m_pEngine; }
		f2dRenderer* GetRenderer()LNOEXCEPT { return m_pRenderer; }
		f2dRenderDevice* GetRenderDev()LNOEXCEPT { return m_pRenderDev; }
		f2dSoundSys* GetSoundSys()LNOEXCEPT { return m_pSoundSys; }

		/// @brief ��ʼ�����
		/// @note �ú���������һ��ʼ�����ã��ҽ��ܵ���һ��
		/// @return ʧ�ܷ���false
		bool Init()LNOEXCEPT;
		/// @brief ��ֹ��ܲ�������Դ
		/// @note �ú��������ɿ�����е��ã��ҽ��ܵ���һ��
		void Shutdown()LNOEXCEPT;

		/// @brief ִ�п�ܣ�������Ϸѭ��
		void Run()LNOEXCEPT;

		/// @brief ����ģʽִ�нű�
		/// @note �ú������޿�ܵ��ã�Ϊ���߼��������á����ű�����ʱ�������󣬸ú�������ػ���󷢳�������Ϣ��
		bool SafeCallScript(const char* source, size_t len, const char* desc)LNOEXCEPT;

		/// @brief ����ģʽ����ȫ�ֺ���
		/// @note �ú������޿�ܵ��ã�Ϊ���߼��������á����ű�����ʱ�������󣬸ú�������ػ���󷢳�������Ϣ��
		bool SafeCallGlobalFunction(const char* name, int retc = 0)LNOEXCEPT;
	protected:  // fancy2d�߼�ѭ���ص�
		fBool OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump);
		fBool OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController);
	public:
		AppFrame()LNOEXCEPT;
		~AppFrame()LNOEXCEPT;
	};
}
