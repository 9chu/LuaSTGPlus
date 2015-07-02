#pragma once
#include "Global.h"
#include "ResourceMgr.h"

#ifdef PlaySound
#undef PlaySound
#endif

namespace LuaSTGPlus
{
	/// @brief ������
	/// @note ʹ��f2dSoundDecoderʵ����ƵPull����ʼ�ձ�֤���44100\16bit\������PCM����
	class AudioMixer :
		public fcyRefObjImpl<f2dSoundDecoder>
	{
	private:
		/// @brief ͨ���ṹ
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
		fuShort GetChannelCount() { return 2; }  // ʼ�շ���2
		fuInt GetSamplesPerSec() { return 44100; }  // ʼ�շ���44100
		fuShort GetFormatTag() { return 1; }
		fuShort GetBitsPerSample() { return 16; }  // 16bits
		fLen GetPosition() { return 0; }  // disabled
		fResult SetPosition(F2DSEEKORIGIN Origin, fInt Offset) { return FCYERR_NOTSUPPORT; }

		/// @brief ��������
		fResult Read(fData pBuffer, fuInt SizeToRead, fuInt* pSizeRead);

		/// @brief ���ź���
		void PlaySound(fcyRefPointer<ResSound> snd, float volume = 1.f, float pan = 0.f);
	public:
		AudioMixer();
		~AudioMixer();
	};
}
