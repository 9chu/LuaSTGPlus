#pragma once
#include "Global.h"
#include "ResourceMgr.h"
#include "Bencode.h"

namespace LuaSTGPlus
{
	/// @brief Ô¶¶Ëµ÷ÊÔÆ÷¿Í»§¶Ë
	class RemoteDebuggerClient
	{
	private:
		enum class UdpMessageType
		{
			PerformanceUpdate = 1,
			ResourceLoaded = 2,
			ResourceRemoved = 3,
			ResourceCleared = 4
		};
	private:
		SOCKET S;
		fuShort m_DebuggerPort;
		fuInt m_DebuggerAddr;

		std::shared_ptr<Bencode::Value> m_SendPacketCache;
	private:
		void sendUdpMessage(UdpMessageType type, std::shared_ptr<Bencode::Value> val);
	public:
		void SendPerformanceCounter(float FPS, float ObjCount, float FrameTime, float RenderTime);
		void SendResourceLoadedHint(ResourceType Type, ResourcePoolType PoolType, const char* Name, const wchar_t* Path, float LoadingTime);
		void SendResourceRemovedHint(ResourceType Type, ResourcePoolType PoolType, const char* Name);
		void SendResourceClearedHint(ResourcePoolType PoolType);
	protected:
		RemoteDebuggerClient& operator=(const RemoteDebuggerClient&);
		RemoteDebuggerClient(const RemoteDebuggerClient&);
	public:
		RemoteDebuggerClient(fuShort port, const char* addr="127.0.0.1");
		~RemoteDebuggerClient();
	};
}
