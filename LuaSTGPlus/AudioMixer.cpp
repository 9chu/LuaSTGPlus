#include "AudioMixer.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

using namespace std;
using namespace LuaSTGPlus;

AudioMixer::AudioMixer()
{
}

AudioMixer::~AudioMixer()
{
}

fResult AudioMixer::Read(fData pBuffer, fuInt SizeToRead, fuInt* pSizeRead)
{
	LASSERT(SizeToRead % GetBlockAlign() == 0);

	m_Section.Lock();
	if (m_NewChannels.size() > 0)
	{
		for (auto i = m_NewChannels.begin(); i != m_NewChannels.end(); ++i)
		{
			auto iter = m_Channels.find(i->buffer);
			if (iter != m_Channels.end())
				m_Channels.erase(iter);

			m_Channels.emplace(i->buffer, *i);
		}

		m_NewChannels.clear();
	}
	m_Section.UnLock();

	fuInt tSampleCount = SizeToRead / GetBlockAlign();
	fShort* pSampleOutBuffer = reinterpret_cast<fShort*>(pBuffer);
	for (fuInt i = 0; i < tSampleCount; ++i)
	{
		int tChannelCountPerSample = 0;
		float tOutputLeft = 0.f, tOutputRight = 0.f;

		auto c = m_Channels.begin();
		while (c != m_Channels.end())
		{
			Channel& chn = c->second;

			// 混合采样数据
			fuInt tChannelSampleCount = chn.buffer->GetSampleCount();
			fuInt tLastPosition = chn.position++;

			float tChannelLeft = chn.buffer->GetSamples()[tLastPosition].first / 32768.f;
			float tChannelRight = chn.buffer->GetSamples()[tLastPosition].second / 32768.f;

			// 音量
			tOutputLeft += tChannelLeft * chn.volume * 0.707f;
			tOutputRight += tChannelRight * chn.volume * 0.707f;

			++tChannelCountPerSample;

			if (chn.position >= tChannelSampleCount)
				c = m_Channels.erase(c);
			else
				++c;
		}

		// 取均值
		tOutputLeft = max(min(tOutputLeft, 1.f), -1.f);
		tOutputRight = max(min(tOutputRight, 1.f), -1.f);
		
		// 写出采样
		pSampleOutBuffer[i * 2] = static_cast<fShort>(tOutputLeft * 32768.f);
		pSampleOutBuffer[(i * 2) + 1] = static_cast<fShort>(tOutputRight * 32768.f);
	}
	
	if (pSizeRead)
		*pSizeRead = SizeToRead;
	return FCYERR_OK;
}

void AudioMixer::PlaySound(fcyRefPointer<ResSound> snd, float volume, float pan)
{
	m_Section.Lock();

	volume = max(min(volume, 1.f), 0.f);
	pan = max(min(pan, 1.f), -1.f);

	try
	{
		m_NewChannels.push_back(Channel(snd, volume, pan));
	}
	catch (const bad_alloc&)
	{
		LERROR("AudioMixer: 无法分配内存");
	}

	m_Section.UnLock();
}
