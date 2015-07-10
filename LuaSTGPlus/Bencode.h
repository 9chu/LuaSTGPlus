// Bencode
// https://github.com/9chu/Bencode
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <memory>
#include <stdexcept>

namespace Bencode
{
	struct Value;

	/// @brief 值类型
	enum class ValueType
	{
		Null,       ///< @brief 空值
		Int,        ///< @brief 整数
		String,     ///< @brief 字符串
		List,       ///< @brief 列表
		Dictionary  ///< @brief 字典
	};

	using IntType = int32_t;
	using StringType = std::string;
	using ListType = std::vector<std::shared_ptr<Value>>;
	using DictType = std::map<StringType, std::shared_ptr<Value>>;

	/// @brief 值
	struct Value
	{
		ValueType  Type = ValueType::Null;

		IntType    VInt = 0;
		StringType VString;
		ListType   VList;
		DictType    VDict;

		Value& operator=(const Value& org);
		Value& operator=(Value&& org);

		Value();
		Value(ValueType t);
		Value(IntType v);
		Value(StringType v);
		Value(ListType v);
		Value(DictType v);
		Value(const Value& org);
		Value(Value&& org);
	};

	/// @brief Bencode编码器
	class Encoder
	{
	private:
		std::string m_DataBuf;
	private:
		void makeData(const Value& v);
	public:
		/// @brief 构造Bencode数据
		const std::string& operator<<(const Value& v);
		/// @brief 获取内部缓冲区
		const std::string& operator*();
		/// @brief 重置
		void Reset();
	private:
		Encoder& operator=(const Encoder&);
		Encoder(const Encoder&);
	public:
		Encoder();
	};

	/// @brief Bencode解码器
	class Decoder
	{
	private:
		Value m_RootValue;

		// === 内部状态 ===
		int m_State;
		std::stack<Value> m_StackValue;
		std::stack<int> m_DictReadState;
		std::stack<StringType> m_DictKeyState;
		StringType m_TempString;
		IntType m_TempInt;
		bool m_TempIntNeg;
	private:
		bool closeValue();
	public:
		/// @brief  输入字符进行解码
		/// @return 若能产生一个根对象则返回true
		bool operator<<(char c);
		/// @brief  获取解码的数据
		const Value& operator->();
		const Value& operator*();
		/// @brief 重置状态
		void Reset();
	private:
		Decoder& operator=(const Decoder&);
		Decoder(const Decoder&);
	public:
		Decoder();
	};
}
