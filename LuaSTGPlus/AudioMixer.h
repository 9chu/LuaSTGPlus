#pragma once
#include "Global.h"
#include "ResourceMgr.h"

#ifdef PlaySound
#undef PlaySound
#endif

namespace LuaSTGPlus
{
	/// @brief 混音器
	/// @note 使用f2dSoundDecoder实现音频Pull流、始终保证输出44100\16bit\立体声PCM采样
	class AudioMixer :
		public fcyRefObjImpl<f2dSoundDecoder>
	{
	private:
		/// @brief 通道结构
		struct Channel
		{
			fcyRefPointer<ResSound> buffer;
			size_t position;
			float volume;
			float pan;
			Channel(fcyRefPointer<ResSound> b, float v, float p)
				: buffer(b), position(0), volume(v), pan(p) {}
			Channel(const Channel& org)
				: buffer(org.buffer), position(org.position), volume(org.volume), pan(org.pan) {}
			Channel(Channel&& org)
				: buffer(org.buffer), position(org.position), volume(org.volume), pan(org.pan) {}
		};
	private:
		fcyCriticalSection m_Section;
		std::unordered_map<ResSound*, Channel> m_Channels;
		std::vector<Channel> m_NewChannels;
	public:
		fuInt GetBufferSize() { return 0; }  // disabled
		fuInt GetAvgBytesPerSec() { return GetBlockAlign() * GetSamplesPerSec(); }
		fuShort GetBlockAlign() { return GetChannelCount() * sizeof(int16_t); }
		fuShort GetChannelCount() { return 2; }  // 始终返回2
		fuInt GetSamplesPerSec() { return 44100; }  // 始终返回44100
		fuShort GetFormatTag() { return 1; }
		fuShort GetBitsPerSample() { return 16; }  // 16bits
		fLen GetPosition() { return 0; }  // disabled
		fResult SetPosition(F2DSEEKORIGIN Origin, fInt Offset) { return FCYERR_NOTSUPPORT; }

		/// @brief 推流函数
		fResult Read(fData pBuffer, fuInt SizeToRead, fuInt* pSizeRead);

		/// @brief 播放函数
		void PlaySound(fcyRefPointer<ResSound> snd, float volume = 1.f, float pan = 0.f);
	public:
		AudioMixer();
		~AudioMixer();
	};
}
