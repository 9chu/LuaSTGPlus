#include "RemoteDebuggerClient.h"

using namespace std;
using namespace Bencode;
using namespace LuaSTGPlus;

#if (defined LDEVVERSION) || (defined LDEBUG)

RemoteDebuggerClient::RemoteDebuggerClient(fuShort p, const char* addr)
	: m_DebuggerPort(p)
{
	WSADATA tData = { 0 };
	int tRet = WSAStartup(MAKEWORD(2, 2), &tData);
	if (tRet != 0)
		throw fcyException("RemoteDebuggerClient::RemoteDebuggerClient", "WSAStartup failed.");
	
	// 创建socket
	S = socket(AF_INET, SOCK_DGRAM, 0);
	if (!S)
		throw fcyException("RemoteDebuggerClient::RemoteDebuggerClient", "Create socket failed.");

	// 设置调试器地址
	m_DebuggerAddr = inet_addr(addr);

	// 创建数据包
	m_SendPacketCache = make_shared<Value>(ValueType::Dictionary);
	m_SendPacketCache->VDict["processId"] = make_shared<Value>(static_cast<int>(GetCurrentProcessId()));
}

RemoteDebuggerClient::~RemoteDebuggerClient()
{
	closesocket(S);
	WSACleanup();
}

void RemoteDebuggerClient::sendUdpMessage(UdpMessageType type, std::shared_ptr<Value> val)
{
	m_SendPacketCache->VDict["msgType"] = make_shared<Value>(static_cast<int>(type));
	m_SendPacketCache->VDict["args"] = val;

	Encoder tEncoder;
	tEncoder << *m_SendPacketCache;
	const std::string& tData = *tEncoder;

	struct sockaddr_in tEndpoint;
	int tEndpointLen = sizeof(tEndpoint);
	memset(&tEndpoint, 0, sizeof(tEndpoint));
	tEndpoint.sin_family = AF_INET;
	tEndpoint.sin_port = htons(m_DebuggerPort);  // 监听端口
	tEndpoint.sin_addr.S_un.S_addr = m_DebuggerAddr;

	// 并不理会sendto成功与否
	sendto(S, tData.c_str(), tData.size(), 0, (sockaddr*)&tEndpoint, tEndpointLen);
}

void RemoteDebuggerClient::SendPerformanceCounter(float FPS, float ObjCount, float FrameTime, float RenderTime)
{
	shared_ptr<Value> tMessage = make_shared<Value>(ValueType::Dictionary);
	tMessage->VDict["fps"] = make_shared<Value>(static_cast<int>(FPS * 1000.f));
	tMessage->VDict["objects"] = make_shared<Value>(static_cast<int>(ObjCount * 1000.f));
	tMessage->VDict["frametime"] = make_shared<Value>(static_cast<int>(FrameTime * 1000.f * 1000.f));
	tMessage->VDict["rendertime"] = make_shared<Value>(static_cast<int>(RenderTime * 1000.f * 1000.f));
	
	sendUdpMessage(UdpMessageType::PerformanceUpdate, tMessage);
}

void RemoteDebuggerClient::SendResourceLoadedHint(ResourceType Type, ResourcePoolType PoolType, const char* Name, const wchar_t* Path, float LoadingTime)
{
	shared_ptr<Value> tMessage = make_shared<Value>(ValueType::Dictionary);
	tMessage->VDict["type"] = make_shared<Value>(static_cast<int>(Type));
	tMessage->VDict["pool"] = make_shared<Value>(static_cast<int>(PoolType));
	tMessage->VDict["name"] = make_shared<Value>(Name);
	tMessage->VDict["path"] = make_shared<Value>(fcyStringHelper::WideCharToMultiByte(Path, CP_UTF8));
	tMessage->VDict["time"] = make_shared<Value>(static_cast<int>(LoadingTime * 1000.f));

	sendUdpMessage(UdpMessageType::ResourceLoaded, tMessage);
}

void RemoteDebuggerClient::SendResourceRemovedHint(ResourceType Type, ResourcePoolType PoolType, const char* Name)
{
	shared_ptr<Value> tMessage = make_shared<Value>(ValueType::Dictionary);
	tMessage->VDict["type"] = make_shared<Value>(static_cast<int>(Type));
	tMessage->VDict["pool"] = make_shared<Value>(static_cast<int>(PoolType));
	tMessage->VDict["name"] = make_shared<Value>(Name);

	sendUdpMessage(UdpMessageType::ResourceRemoved, tMessage);
}

void RemoteDebuggerClient::SendResourceClearedHint(ResourcePoolType PoolType)
{
	shared_ptr<Value> tMessage = make_shared<Value>(ValueType::Dictionary);
	tMessage->VDict["pool"] = make_shared<Value>(static_cast<int>(PoolType));

	sendUdpMessage(UdpMessageType::ResourceCleared, tMessage);
}

#endif
