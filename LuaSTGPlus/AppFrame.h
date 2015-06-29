/// @file AppFrame.h
/// @brief ����Ӧ�ó�����
#pragma once
#include "Global.h"
#include "ResourceMgr.h"
#include "GameObjectPool.h"

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
		AppStatus m_iStatus = AppStatus::NotInitialized;

		// ��Դ������
		ResourceMgr m_ResourceMgr;

		// �����
		std::unique_ptr<GameObjectPool> m_GameObjectPool;

		// Lua�����
		lua_State* L = nullptr;
		std::vector<char> m_TempBuffer;  // ��ʱ������

		// ѡ����ֵ
		bool m_OptionWindowed = true;
		fuInt m_OptionFPSLimit = 60;
		bool m_OptionVsync = true;
		bool m_OptionVsyncOrg = true;  // ���ڱ����豸��ֱͬ����Ϣ
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
		
		GraphicsType m_GraphType = GraphicsType::Graph2D;
		bool m_bRenderStarted = false;

		BlendMode m_Graph2DLastBlendMode;
		f2dBlendState m_Graph2DBlendState;
		F2DGRAPH2DBLENDTYPE m_Graph2DColorBlendState;
		fcyRefPointer<f2dGraphics2D> m_Graph2D;

		fCharW m_LastChar;
		fInt m_LastKey;
		fBool m_KeyStateMap[256];
	private:
		void updateGraph2DBlendMode(BlendMode m)
		{
			if (m != m_Graph2DLastBlendMode)
			{
				switch (m)
				{
				case BlendMode::AddAdd:
					m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
					m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
					break;
				case BlendMode::MulAdd:
					m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_ONE;
					m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_MODULATE;
					break;
				case BlendMode::MulAlpha:
					m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
					m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_MODULATE;
					break;
				case BlendMode::AddAlpha:
				default:
					m_Graph2DBlendState.DestBlend = F2DBLENDFACTOR_INVSRCALPHA;
					m_Graph2DColorBlendState = F2DGRAPH2DBLENDTYPE_ADD;
					break;
				}
				m_Graph2DLastBlendMode = m;
				m_Graph2D->SetBlendState(m_Graph2DBlendState);
				m_Graph2D->SetColorBlendType(m_Graph2DColorBlendState);
			}
		}
	public: // �ű����ýӿڣ�����μ�API�ĵ�
		void SetWindowed(bool v)LNOEXCEPT;
		void SetFPS(fuInt v)LNOEXCEPT;
		void SetVsync(bool v)LNOEXCEPT;
		void SetResolution(fuInt width, fuInt height)LNOEXCEPT;
		void SetSplash(bool v)LNOEXCEPT;
		LNOINLINE void SetTitle(const char* v)LNOEXCEPT;  // UTF8����

		/// @brief ʹ���µ���Ƶ����������ʾģʽ
		/// @note ���л�ʧ������лع�
		LNOINLINE bool UpdateVideoParameters()LNOEXCEPT;

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
				static_cast<float>(m_pRenderDev->GetBufferHeight() - (int)top),
				static_cast<float>((int)right),
				static_cast<float>(m_pRenderDev->GetBufferHeight() - (int)bottom)
			))))
			{
				LWARNING("�����ӿ�(left: %lf, right: %lf, bottom: %lf, top: %lf)ʧ��", left, right, bottom, top);
				return false;
			}
			return true;
		}

		/// @brief ������ͶӰ����
		void SetOrtho(float left, float right, float bottom, float top)LNOEXCEPT
		{
			if (m_GraphType == GraphicsType::Graph2D)
			{
				m_Graph2D->SetWorldTransform(fcyMatrix4::GetTranslateMatrix(fcyVec3(-0.5f, -0.5f, 0.f)));
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
				LERROR("Render: ֻ��2D��Ⱦ������ִ�и÷���");
				return false;
			}

			fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
			if (!p)
			{
				LERROR("Render: �Ҳ���ͼ����Դ'%m'", name);
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
				LERROR("Render: ֻ��2D��Ⱦ������ִ�и÷���");
				return false;
			}

			fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
			if (!p)
			{
				LERROR("Render: �Ҳ���ͼ����Դ'%m'", name);
				return false;
			}
			
			// ���û��
			updateGraph2DBlendMode(p->GetBlendMode());

			f2dSprite* pSprite = p->GetSprite();
			pSprite->SetZ(0.5f);
			pSprite->Draw(m_Graph2D, fcyVec3(x1, y1, z1), fcyVec3(x2, y2, z2), fcyVec3(x3, y3, z3), fcyVec3(x4, y4, z4), false);
			return true;
		}
	public:
		ResourceMgr& GetResourceMgr()LNOEXCEPT { return m_ResourceMgr; }
		GameObjectPool& GetGameObjectPool()LNOEXCEPT { return *m_GameObjectPool.get(); }
		f2dRenderer* GetRenderer()LNOEXCEPT { return m_pRenderer; }
		f2dRenderDevice* GetRenderDev()LNOEXCEPT { return m_pRenderDev; }

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
		bool SafeCallGlobalFunction(const char* name, int argc = 0, int retc = 0)LNOEXCEPT;
	protected:  // fancy2d�߼�ѭ���ص�
		fBool OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump);
		fBool OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController);
	public:
		AppFrame()LNOEXCEPT;
		~AppFrame()LNOEXCEPT;
	};
}
