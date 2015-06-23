/// @file AppFrame.h
/// @brief ����Ӧ�ó�����
#pragma once
#include "Global.h"

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

	/// @brief Ӧ�ó�����
	class AppFrame
	{
	public:
		static LNOINLINE AppFrame& GetInstance();
	private:
		AppStatus m_iStatus = AppStatus::NotInitialized;

		// Lua�����
		lua_State* L = nullptr;

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
	private:
		std::string loadFileAsString(fcStrW path);
	public: // �ű����ýӿڣ�����μ�API�ĵ�
		void SetWindowed(bool v)LNOEXCEPT;
		void SetFPS(fuInt v)LNOEXCEPT;
		void SetVsync(bool v)LNOEXCEPT;
		void SetResolution(fuInt width, fuInt height)LNOEXCEPT;
		void SetSplash(bool v)LNOEXCEPT;
		LNOINLINE void SetTitle(const char* v)LNOEXCEPT;

		/// @brief ʹ���µ���Ƶ����������ʾģʽ
		/// @note ���л�ʧ������лع�
		LNOINLINE bool UpdateVideoParameters()LNOEXCEPT;

		/// @brief ��ȡ��ǰ��FPS
		double GetFPS()LNOEXCEPT { return m_fFPS; }
	public:
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
	public:
		AppFrame()LNOEXCEPT;
		~AppFrame()LNOEXCEPT;
	};
}
