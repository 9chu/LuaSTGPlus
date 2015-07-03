/// @file LuaWrapper.h
/// @brief lua包装层 用于导出C++函数和类
#pragma once
#include "Global.h"

namespace LuaSTGPlus
{
	/// @brief 颜色包装
	class ColorWrapper
	{
	public:
		/// @brief 向lua注册包装类
		static void Register(lua_State* L)LNOEXCEPT;
		/// @brief 创建一个颜色类并推入堆栈
		static fcyColor* CreateAndPush(lua_State* L);
	};

	/// @brief 随机数发生器包装
	class RandomizerWrapper
	{
	public:
		/// @brief 向lua注册包装类
		static void Register(lua_State* L)LNOEXCEPT;
		/// @brief 创建一个颜色类并推入堆栈
		static fcyRandomWELL512* CreateAndPush(lua_State* L);
	};

	class GameObjectBentLaser;

	/// @brief 曲线激光包装
	class BentLaserWrapper
	{
	public:
		/// @brief 向lua注册包装类
		static void Register(lua_State* L)LNOEXCEPT;
		/// @brief 创建一个曲线激光类并推入堆栈
		static GameObjectBentLaser* CreateAndPush(lua_State* L);
	};

	/// @brief 内建函数包装
	class BuiltInFunctionWrapper
	{
	public:
		/// @brief 向lua注册包装类
		static void Register(lua_State* L)LNOEXCEPT;
	};
}
