#include "Bencode.h"

using namespace Bencode;
using namespace std;

////////////////////////////////////////////////////////////////////////////////

Value::Value()
{}

Value::Value(ValueType t)
	: Type(t)
{}

Value::Value(IntType v)
	: Type(ValueType::Int), VInt(v)
{}

Value::Value(StringType v)
	: Type(ValueType::String), VString(v)
{}

Value::Value(ListType v)
	: Type(ValueType::List), VList(v)
{}

Value::Value(DictType v)
	: Type(ValueType::Dictionary), VDict(v)
{}

Value::Value(const Value& org)
	: Type(org.Type), VInt(org.VInt), VString(org.VString), VList(org.VList), VDict(org.VDict)
{}

Value::Value(Value&& org)
	: Type(org.Type), VInt(org.VInt), VString(org.VString), VList(org.VList), VDict(org.VDict)
{}

Value& Value::operator=(const Value& org)
{
	Type = org.Type;
	VInt = org.VInt;
	VString = org.VString;
	VList = org.VList;
	VDict = org.VDict;
	return *this;
}

Value& Value::operator=(Value&& org)
{
	Type = org.Type;
	VInt = org.VInt;
	VString = org.VString;
	VList = org.VList;
	VDict = org.VDict;
	return *this;
}

////////////////////////////////////////////////////////////////////////////////

Encoder::Encoder()
{
	Reset();
}

void Encoder::makeData(const Value& v)
{
	switch (v.Type)
	{
	case ValueType::Null:
		m_DataBuf.push_back('n');
		break;
	case ValueType::Int:
		{
			m_DataBuf.push_back('i');
			m_DataBuf += to_string(v.VInt);
			m_DataBuf.push_back('e');
			break;
		}
	case ValueType::String:
		{
			m_DataBuf += to_string(v.VString.length());
			m_DataBuf.push_back(':');
			m_DataBuf += v.VString;
			break;
		}
	case ValueType::List:
		m_DataBuf.push_back('l');
		for (auto i : v.VList)
			makeData(*i.get());
		m_DataBuf.push_back('e');
		break;
	case ValueType::Dictionary:
		m_DataBuf.push_back('d');
		for (auto i : v.VDict)
		{
			makeData(i.first);
			makeData(*i.second.get());
		}	
		m_DataBuf.push_back('e');
		break;
	default:
		throw logic_error("unexpected type.");
	}
}

const std::string& Encoder::operator<<(const Value& v)
{
	Reset();
	makeData(v);
	return m_DataBuf;
}

const std::string& Encoder::operator*()
{
	return m_DataBuf;
}

void Encoder::Reset()
{
	m_DataBuf.clear();
}

////////////////////////////////////////////////////////////////////////////////

enum class DecoderInternalState
{
	Start,

	ParseNull,
	ParseIntSign,
	ParseIntPart,
	ParseString,
	ParseStringContent
};

enum class DictReadState
{
	KeyExpected,
	ValueExpected
};

Decoder::Decoder()
{
	Reset();
}

bool Decoder::closeValue()
{
	DecoderInternalState tState = (DecoderInternalState)m_State;
	m_State = (int)DecoderInternalState::Start;

	Value tValue;
	switch (tState)
	{
	case DecoderInternalState::Start:
		if (m_StackValue.empty())
			throw logic_error("unexpected end of object.");
		tValue = std::move(m_StackValue.top());
		m_StackValue.pop();
		if (tValue.Type == ValueType::Dictionary)
		{
			if (m_DictReadState.top() != (int)DictReadState::KeyExpected)
				throw logic_error("key-value pair expected.");
			m_DictReadState.pop();
		}
		break;
	case DecoderInternalState::ParseNull:
		tValue.Type = ValueType::Null;
		break;
	case DecoderInternalState::ParseIntSign:
	case DecoderInternalState::ParseIntPart:
		tValue.Type = ValueType::Int;
		tValue.VInt = m_TempIntNeg ? -m_TempInt : m_TempInt;
		break;
	case DecoderInternalState::ParseString:
	case DecoderInternalState::ParseStringContent:
		tValue.Type = ValueType::String;
		tValue.VString = m_TempString;
		break;
	default:
		throw logic_error("internal error.");
	}

	if (m_StackValue.empty())
	{
		m_RootValue = std::move(tValue);
		return true;
	}

	switch (m_StackValue.top().Type)
	{
	case ValueType::List:
		m_StackValue.top().VList.emplace_back(make_shared<Value>(std::move(tValue)));
		break;
	case ValueType::Dictionary:
		if (m_DictReadState.top() == (int)DictReadState::KeyExpected)
		{
			if (tValue.Type != ValueType::String)
				throw logic_error("key must be a string.");
			m_DictKeyState.emplace(std::move(tValue.VString));
			m_DictReadState.top() = (int)DictReadState::ValueExpected;
		}
		else
		{
			m_StackValue.top().VDict.emplace(pair<StringType, shared_ptr<Value>>(
				m_DictKeyState.top(), make_shared<Value>(std::move(tValue))));
			m_DictKeyState.pop();
			m_DictReadState.top() = (int)DictReadState::KeyExpected;
		}
		break;
	default:
		throw logic_error("internal error.");
	}

	return false;
}

bool Decoder::operator<<(char c)
{
	try
	{
		switch ((DecoderInternalState)m_State)
		{
		case DecoderInternalState::Start:
			switch (c)
			{
			case 'n':
				m_State = (int)DecoderInternalState::ParseNull;
				if (closeValue())
					return true;
				break;
			case 'i':
				m_TempInt = 0;
				m_TempIntNeg = false;
				m_State = (int)DecoderInternalState::ParseIntSign;
				break;
			case 'l':
				m_StackValue.emplace(Value(ValueType::List));
				break;
			case 'd':
				m_StackValue.emplace(Value(ValueType::Dictionary));
				m_DictReadState.emplace((int)DictReadState::KeyExpected);
				break;
			case 'e':
				if (closeValue())
					return true;
				break;
			default:
				if (c >= '0' && c <= '9')
				{
					m_TempInt = c - '0';
					m_TempString.clear();
					m_State = (int)DecoderInternalState::ParseString;
				}
				else
					throw logic_error("unexpected character.");
			}
			break;
		case DecoderInternalState::ParseIntSign:
			if (c == '-')
			{
				m_TempIntNeg = true;
				m_State = (int)DecoderInternalState::ParseIntPart;
			}
			else if (c >= '0' && c <= '9')
			{
				m_TempInt = c - '0';
				m_State = (int)DecoderInternalState::ParseIntPart;
			}	
			else
				throw logic_error("unexpected character.");
			break;
		case DecoderInternalState::ParseIntPart:
			if (c >= '0' && c <= '9')
				m_TempInt = m_TempInt * 10 + c - '0';
			else if (c == 'e')
			{
				if (closeValue())
					return true;
			}
			else
				throw logic_error("unexpected character.");
			break;
		case DecoderInternalState::ParseString:
			if (c >= '0' && c <= '9')
				m_TempInt = m_TempInt * 10 + c - '0';
			else if (c == ':')
			{
				if (m_TempInt <= 0)
				{
					if (closeValue())
						return true;
				}
				else
					m_State = (int)DecoderInternalState::ParseStringContent;
			}
			else
				throw logic_error("unexpected character.");
			break;
		case DecoderInternalState::ParseStringContent:
			m_TempString.push_back(c);
			--m_TempInt;
			if (m_TempInt <= 0)
			{
				if (closeValue())
					return true;
			}
			break;
		default:
			throw logic_error("internal error.");
		}
	}
	catch (...)
	{
		Reset();
		throw;
	}
	return false;
}

const Value& Decoder::operator->()
{
	return m_RootValue;
}

const Value& Decoder::operator*()
{
	return m_RootValue;
}

void Decoder::Reset()
{
	m_RootValue.Type = ValueType::Null;
	m_State = (int)DecoderInternalState::Start;
	while (!m_StackValue.empty()) m_StackValue.pop();
	while (!m_DictKeyState.empty()) m_DictKeyState.pop();
	while (!m_DictReadState.empty()) m_DictReadState.pop();
}
