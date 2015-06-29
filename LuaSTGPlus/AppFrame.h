/// @file AppFrame.h
/// @brief 定义应用程序框架
#pragma once
#include "Global.h"
#include "ResourceMgr.h"
#include "GameObjectPool.h"

namespace LuaSTGPlus
{
	/// @brief 应用程序状态
	enum class AppStatus
	{
		NotInitialized,
		Initializing,
		Initialized,
		Running,
		Aborted,
		Destroyed
	};

	/// @brief 当前激活的渲染器
	enum class GraphicsType
	{
		Graph2D,
		Graph3D
	};

	/// @brief 应用程序框架
	class AppFrame :
		public f2dEngineEventListener
	{
	public:
		static LNOINLINE AppFrame& GetInstance();
	private:
		AppStatus m_iStatus = AppStatus::NotInitialized;

		// 资源管理器
		ResourceMgr m_ResourceMgr;

		// 对象池
		std::unique_ptr<GameObjectPool> m_GameObjectPool;

		// Lua虚拟机
		lua_State* L = nullptr;
		std::vector<char> m_TempBuffer;  // 临时缓冲区

		// 选项与值
		bool m_OptionWindowed = true;
		fuInt m_OptionFPSLimit = 60;
		bool m_OptionVsync = true;
		bool m_OptionVsyncOrg = true;  // 用于保存设备垂直同步信息
		fcyVec2 m_OptionResolution = fcyVec2(640.f, 480.f);
		bool m_OptionSplash = false;
		std::wstring m_OptionTitle = L"LuaSTGPlus";
		fDouble m_fFPS = 0.;

		// 引擎
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
	public: // 脚本调用接口，含义参见API文档
		void SetWindowed(bool v)LNOEXCEPT;
		void SetFPS(fuInt v)LNOEXCEPT;
		void SetVsync(bool v)LNOEXCEPT;
		void SetResolution(fuInt width, fuInt height)LNOEXCEPT;
		void SetSplash(bool v)LNOEXCEPT;
		LNOINLINE void SetTitle(const char* v)LNOEXCEPT;  // UTF8编码

		/// @brief 使用新的视频参数更新显示模式
		/// @note 若切换失败则进行回滚
		LNOINLINE bool UpdateVideoParameters()LNOEXCEPT;

		/// @brief 获取当前的FPS
		double GetFPS()LNOEXCEPT { return m_fFPS; }

		/// @brief 执行资源包中的文件
		/// @note 该函数为脚本系统使用
		LNOINLINE void LoadScript(const char* path)LNOEXCEPT;

		/// @brief 检查按键是否按下
		fBool GetKeyState(int VKCode)LNOEXCEPT;

		/// @brief 获得最后一次字符输入（UTF-8）
		LNOINLINE int GetLastChar(lua_State* L)LNOEXCEPT;

		/// @brief 获得最后一次按键输入
		int GetLastKey()LNOEXCEPT { return m_LastKey; }
	public:  // 渲染器接口
		/// @brief 通知开始渲染
		bool BeginScene()LNOEXCEPT;

		/// @brief 通知结束渲染
		bool EndScene()LNOEXCEPT;

		/// @brief 清屏
		void ClearScreen(const fcyColor& c)LNOEXCEPT
		{
			m_pRenderDev->Clear(c);
		}

		/// @brief 设置视口
		bool SetViewport(double left, double right, double bottom, double top)LNOEXCEPT
		{
			if (FCYFAILED(m_pRenderDev->SetViewport(fcyRect(
				static_cast<float>((int)left),
				static_cast<float>(m_pRenderDev->GetBufferHeight() - (int)top),
				static_cast<float>((int)right),
				static_cast<float>(m_pRenderDev->GetBufferHeight() - (int)bottom)
			))))
			{
				LWARNING("设置视口(left: %lf, right: %lf, bottom: %lf, top: %lf)失败", left, right, bottom, top);
				return false;
			}
			return true;
		}

		/// @brief 设置正投影矩阵。
		void SetOrtho(float left, float right, float bottom, float top)LNOEXCEPT
		{
			if (m_GraphType == GraphicsType::Graph2D)
			{
				m_Graph2D->SetWorldTransform(fcyMatrix4::GetTranslateMatrix(fcyVec3(-0.5f, -0.5f, 0.f)));
				m_Graph2D->SetViewTransform(fcyMatrix4::GetIdentity());
				m_Graph2D->SetProjTransform(fcyMatrix4::GetOrthoOffCenterLH(left, right, bottom, top, 0.f, 100.f));
			}
		}

		/// @brief 设置透视投影矩阵
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

		/// @brief 设置雾值
		/// @note 扩展方法，视情况移除。
		void SetFog(float start, float end, fcyColor color);

		/// @brief 渲染图像
		bool Render(ResSprite* p, float x, float y, float rot = 0, float hscale = 1, float vscale = 1, float z = 0.5)LNOEXCEPT
		{
			LASSERT(p);
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render: 只有2D渲染器可以执行该方法");
				return false;
			}

			// 设置混合
			updateGraph2DBlendMode(p->GetBlendMode());

			// 渲染
			f2dSprite* pSprite = p->GetSprite();
			pSprite->SetZ(z);
			pSprite->Draw2(m_Graph2D, fcyVec2(x, y), fcyVec2(hscale, vscale), rot, false);
			return true;
		}

		/// @brief 渲染动画
		bool Render(ResAnimation* p, int ani_timer, float x, float y, float rot = 0, float hscale = 1, float vscale = 1)LNOEXCEPT
		{
			LASSERT(p);
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render: 只有2D渲染器可以执行该方法");
				return false;
			}

			// 设置混合
			updateGraph2DBlendMode(p->GetBlendMode());

			// 渲染
			f2dSprite* pSprite = p->GetSprite(((fuInt)ani_timer / p->GetInterval()) % p->GetCount());
			pSprite->Draw2(m_Graph2D, fcyVec2(x, y), fcyVec2(hscale, vscale), rot, false);
			return true;
		}

		/// @brief 渲染粒子
		bool Render(ResParticle::ParticlePool* p, float hscale = 1, float vscale = 1)LNOEXCEPT
		{
			LASSERT(p);
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render: 只有2D渲染器可以执行该方法");
				return false;
			}

			// 设置混合
			updateGraph2DBlendMode(p->GetBlendMode());

			// 渲染
			p->Render(m_Graph2D, hscale, vscale);
			return true;
		}

		/// @brief 渲染图像
		bool Render(const char* name, float x, float y, float rot = 0, float hscale = 1, float vscale = 1, float z = 0.5)LNOEXCEPT
		{
			fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
			if (!p)
			{
				LERROR("Render: 找不到图像资源'%m'", name);
				return false;
			}
			return Render(p, x, y, rot, hscale, vscale, z);
		}

		/// @brief 渲染图像
		bool RenderRect(const char* name, float x1, float y1, float x2, float y2)LNOEXCEPT
		{
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render: 只有2D渲染器可以执行该方法");
				return false;
			}

			fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
			if (!p)
			{
				LERROR("Render: 找不到图像资源'%m'", name);
				return false;
			}

			// 设置混合
			updateGraph2DBlendMode(p->GetBlendMode());

			// 渲染
			f2dSprite* pSprite = p->GetSprite();
			pSprite->SetZ(0.5f);
			pSprite->Draw(m_Graph2D, fcyRect(x1, y1, x2, y2), false);
			return true;
		}

		/// @brief 渲染图像
		bool Render4V(const char* name, float x1, float y1, float z1, float x2, float y2, float z2, 
			float x3, float y3, float z3, float x4, float y4, float z4)LNOEXCEPT
		{
			if (m_GraphType != GraphicsType::Graph2D)
			{
				LERROR("Render: 只有2D渲染器可以执行该方法");
				return false;
			}

			fcyRefPointer<ResSprite> p = m_ResourceMgr.FindSprite(name);
			if (!p)
			{
				LERROR("Render: 找不到图像资源'%m'", name);
				return false;
			}
			
			// 设置混合
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

		/// @brief 初始化框架
		/// @note 该函数必须在一开始被调用，且仅能调用一次
		/// @return 失败返回false
		bool Init()LNOEXCEPT;
		/// @brief 终止框架并回收资源
		/// @note 该函数可以由框架自行调用，且仅能调用一次
		void Shutdown()LNOEXCEPT;

		/// @brief 执行框架，进入游戏循环
		void Run()LNOEXCEPT;

		/// @brief 保护模式执行脚本
		/// @note 该函数仅限框架调用，为主逻辑最外层调用。若脚本运行时发生错误，该函数负责截获错误发出错误消息。
		bool SafeCallScript(const char* source, size_t len, const char* desc)LNOEXCEPT;

		/// @brief 保护模式调用全局函数
		/// @note 该函数仅限框架调用，为主逻辑最外层调用。若脚本运行时发生错误，该函数负责截获错误发出错误消息。
		bool SafeCallGlobalFunction(const char* name, int argc = 0, int retc = 0)LNOEXCEPT;
	protected:  // fancy2d逻辑循环回调
		fBool OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump);
		fBool OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController);
	public:
		AppFrame()LNOEXCEPT;
		~AppFrame()LNOEXCEPT;
	};
}
