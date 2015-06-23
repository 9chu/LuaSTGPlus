/// @file LogSystem.h
/// @brief 定义日志系统
/// @note 由于日志系统将被Common.h包含，因此独立于Common.h
#pragma once
#include <fstream>

namespace LuaSTGPlus
{
	/// @brief 日志级别
	enum class LogType
	{
		Information,
		Warning,
		Error
	};

	/// @brief 日志系统
	class LogSystem
	{
	public:
		/// @brief 获取日志系统实例
		static __declspec(noinline) LogSystem& GetInstance();
	private:
		std::fstream m_LogFile;
	public:
		/// @brief 记录日志
		/// @param type 日志类型
		/// @param info 格式化文本
		__declspec(noinline) void Log(LogType type, const wchar_t* info, ...)throw();
	public:
		LogSystem();
		~LogSystem();
	};
}
