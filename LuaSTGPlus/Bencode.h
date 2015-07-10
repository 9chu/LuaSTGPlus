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

	/// @brief ֵ����
	enum class ValueType
	{
		Null,       ///< @brief ��ֵ
		Int,        ///< @brief ����
		String,     ///< @brief �ַ���
		List,       ///< @brief �б�
		Dictionary  ///< @brief �ֵ�
	};

	using IntType = int32_t;
	using StringType = std::string;
	using ListType = std::vector<std::shared_ptr<Value>>;
	using DictType = std::map<StringType, std::shared_ptr<Value>>;

	/// @brief ֵ
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

	/// @brief Bencode������
	class Encoder
	{
	private:
		std::string m_DataBuf;
	private:
		void makeData(const Value& v);
	public:
		/// @brief ����Bencode����
		const std::string& operator<<(const Value& v);
		/// @brief ��ȡ�ڲ�������
		const std::string& operator*();
		/// @brief ����
		void Reset();
	private:
		Encoder& operator=(const Encoder&);
		Encoder(const Encoder&);
	public:
		Encoder();
	};

	/// @brief Bencode������
	class Decoder
	{
	private:
		Value m_RootValue;

		// === �ڲ�״̬ ===
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
		/// @brief  �����ַ����н���
		/// @return ���ܲ���һ���������򷵻�true
		bool operator<<(char c);
		/// @brief  ��ȡ���������
		const Value& operator->();
		const Value& operator*();
		/// @brief ����״̬
		void Reset();
	private:
		Decoder& operator=(const Decoder&);
		Decoder(const Decoder&);
	public:
		Decoder();
	};
}
