/// @file AppFrame.h
/// @brief 定义应用程序框架
#pragma once
#include "Global.h"

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

	/// @brief 应用程序框架
	class AppFrame
	{
	public:
		static LNOINLINE AppFrame& GetInstance();
	private:
		AppStatus m_iStatus = AppStatus::NotInitialized;

		// Lua虚拟机
		lua_State* L = nullptr;

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
	private:
		std::string loadFileAsString(fcStrW path);
	public: // 脚本调用接口，含义参见API文档
		void SetWindowed(bool v)LNOEXCEPT;
		void SetFPS(fuInt v)LNOEXCEPT;
		void SetVsync(bool v)LNOEXCEPT;
		void SetResolution(fuInt width, fuInt height)LNOEXCEPT;
		void SetSplash(bool v)LNOEXCEPT;
		LNOINLINE void SetTitle(const char* v)LNOEXCEPT;

		/// @brief 使用新的视频参数更新显示模式
		/// @note 若切换失败则进行回滚
		LNOINLINE bool UpdateVideoParameters()LNOEXCEPT;

		/// @brief 获取当前的FPS
		double GetFPS()LNOEXCEPT { return m_fFPS; }
	public:
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
	public:
		AppFrame()LNOEXCEPT;
		~AppFrame()LNOEXCEPT;
	};
}
