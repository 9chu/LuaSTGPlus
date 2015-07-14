#include "ResourceMgr.h"
#include "AppFrame.h"

#include "Utility.h"

#include <iowin32.h>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

using namespace std;
using namespace LuaSTGPlus;

#if (defined LDEVVERSION) || (defined LDEBUG)
#define LDEBUG_RESOURCETIMER float tResourceLoadingTime
#define LDEBUG_RESOURCESCOPE TimerScope tLoadingTimer(tResourceLoadingTime)
#define LDEBUG_RESOURCEHINT(t, path) \
    LAPP.SendResourceLoadedHint(t, m_iType, name, path, tResourceLoadingTime)
#else
#define LDEBUG_RESOURCETIMER
#define LDEBUG_RESOURCESCOPE
#define LDEBUG_RESOURCEHINT
#endif

fcyMemPool<sizeof(ResParticle::ParticlePool)> ResParticle::s_MemoryPool(512);  // Ԥ����512������

////////////////////////////////////////////////////////////////////////////////
/// ResAnimation
////////////////////////////////////////////////////////////////////////////////
ResAnimation::ResAnimation(const char* name, fcyRefPointer<ResTexture> tex, float x, float y, float w, float h,
	int n, int m, int intv, double a, double b, bool rect)
	: Resource(ResourceType::Animation, name), m_Interval(intv), m_HalfSizeX(a), m_HalfSizeY(b), m_bRectangle(rect)
{
	LASSERT(LAPP.GetRenderer());

	// �ָ�����
	for (int j = 0; j < m; ++j)  // ��
	{
		for (int i = 0; i < n; ++i)  // ��
		{
			fcyRefPointer<f2dSprite> t;
			if (FCYFAILED(LAPP.GetRenderer()->CreateSprite2D(tex->GetTexture(), fcyRect(
				x + w * i, y + h * j, x + w * (i + 1), y + h * (j + 1)
				), &t)))
			{
				throw fcyException("ResAnimation::ResAnimation", "CreateSprite2D failed.");
			}
			t->SetZ(0.5f);
			t->SetColor(0xFFFFFFFF);
			m_ImageSequences.push_back(t);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
/// ResParticle
////////////////////////////////////////////////////////////////////////////////
ResParticle::ResParticle(const char* name, const ParticleInfo& pinfo, fcyRefPointer<f2dSprite> sprite, BlendMode bld, double a, double b, bool rect)
	: Resource(ResourceType::Particle, name), m_ParticleInfo(pinfo), m_BindedSprite(sprite), m_BlendMode(bld), m_HalfSizeX(a), m_HalfSizeY(b), m_bRectangle(rect)
{
}

ResParticle::ParticlePool* ResParticle::AllocInstance()LNOEXCEPT
{
	// �� ���棺Ǳ��bad_alloc���´�����ʱ���账��
	ParticlePool* pRet = new(s_MemoryPool.Alloc()) ParticlePool(this);
	pRet->SetBlendMode(m_BlendMode);
	return pRet;
}

void ResParticle::FreeInstance(ResParticle::ParticlePool* p)LNOEXCEPT
{
	p->~ParticlePool();
	s_MemoryPool.Free(p);
}

ResParticle::ParticlePool::ParticlePool(fcyRefPointer<ResParticle> ref)
	: m_pInstance(ref), m_fEmission((float)ref->GetParticleInfo().nEmission) {}

void ResParticle::ParticlePool::Update(float delta)
{
	static fcyRandomWELL512 s_ParticleRandomizer;

	const ParticleInfo& pInfo = m_pInstance->GetParticleInfo();

	if (m_iStatus == Status::Alive)
	{
		m_fAge += delta;
		if (m_fAge >= pInfo.fLifetime && pInfo.fLifetime >= 0.f)
			m_iStatus = Status::Sleep;
	}

	// ������������
	size_t i = 0;
	while (i < m_iAlive)
	{
		ParticleInstance& tInst = m_ParticlePool[i];
		tInst.fAge += delta;
		if (tInst.fAge >= tInst.fTerminalAge)
		{
			--m_iAlive;
			if (m_iAlive > i + 1)
				memcpy(&tInst, &m_ParticlePool[m_iAlive], sizeof(ParticleInstance));
			continue;
		}

		// �����߼��ٶȺ�������ٶ�
		fcyVec2 vecAccel = (tInst.vecLocation - m_vCenter).GetNormalize();
		fcyVec2 vecAccel2 = vecAccel;
		vecAccel *= tInst.fRadialAccel;
		// vecAccel2.Rotate(M_PI_2);
		std::swap(vecAccel2.x, vecAccel2.y);
		vecAccel2.x = -vecAccel2.x;
		vecAccel2 *= tInst.fTangentialAccel;

		// �����ٶ�
		tInst.vecVelocity += (vecAccel + vecAccel2) * delta;
		tInst.vecVelocity.y += tInst.fGravity * delta;

		// ����λ��
		tInst.vecLocation += tInst.vecVelocity * delta;

		// ���������ʹ�С
		tInst.fSpin += tInst.fSpinDelta * delta;
		tInst.fSize += tInst.fSizeDelta * delta;
		tInst.colColor[0] += tInst.colColorDelta[0] * delta;
		tInst.colColor[1] += tInst.colColorDelta[1] * delta;
		tInst.colColor[2] += tInst.colColorDelta[2] * delta;
		tInst.colColor[3] += tInst.colColorDelta[3] * delta;
		
		++i;
	}

	// �����µ�����
	if (m_iStatus == Status::Alive)
	{
		float fParticlesNeeded = m_fEmission * delta + m_fEmissionResidue;
		fuInt nParticlesCreated = (fuInt)fParticlesNeeded;
		m_fEmissionResidue = fParticlesNeeded - (float)nParticlesCreated;

		for (fuInt i = 0; i < nParticlesCreated; ++i)
		{
			if (m_iAlive >= m_ParticlePool.size())
				break;

			ParticleInstance& tInst = m_ParticlePool[m_iAlive++];
			tInst.fAge = 0.0f;
			tInst.fTerminalAge = s_ParticleRandomizer.GetRandFloat(pInfo.fParticleLifeMin, pInfo.fParticleLifeMax);

			tInst.vecLocation = m_vPrevCenter + (m_vCenter - m_vPrevCenter) * s_ParticleRandomizer.GetRandFloat(0.0f, 1.0f);
			tInst.vecLocation.x += s_ParticleRandomizer.GetRandFloat(-2.0f, 2.0f);
			tInst.vecLocation.y += s_ParticleRandomizer.GetRandFloat(-2.0f, 2.0f);

			float ang = /* pInfo.fDirection */ (m_fRotation - (float)LPI_HALF) - (float)LPI_HALF + s_ParticleRandomizer.GetRandFloat(0, pInfo.fSpread) - pInfo.fSpread / 2.0f;
			tInst.vecVelocity.x = cos(ang);
			tInst.vecVelocity.y = sin(ang);
			tInst.vecVelocity *= s_ParticleRandomizer.GetRandFloat(pInfo.fSpeedMin, pInfo.fSpeedMax);

			tInst.fGravity = s_ParticleRandomizer.GetRandFloat(pInfo.fGravityMin, pInfo.fGravityMax);
			tInst.fRadialAccel = s_ParticleRandomizer.GetRandFloat(pInfo.fRadialAccelMin, pInfo.fRadialAccelMax);
			tInst.fTangentialAccel = s_ParticleRandomizer.GetRandFloat(pInfo.fTangentialAccelMin, pInfo.fTangentialAccelMax);

			tInst.fSize = s_ParticleRandomizer.GetRandFloat(pInfo.fSizeStart, pInfo.fSizeStart + (pInfo.fSizeEnd - pInfo.fSizeStart) * pInfo.fSizeVar);
			tInst.fSizeDelta = (pInfo.fSizeEnd - tInst.fSize) / tInst.fTerminalAge;

			tInst.fSpin = /* pInfo.fSpinStart */ m_fRotation + s_ParticleRandomizer.GetRandFloat(0, pInfo.fSpinEnd) - pInfo.fSpinEnd / 2.0f;
			tInst.fSpinDelta = pInfo.fSpinVar;

			tInst.colColor[0] = s_ParticleRandomizer.GetRandFloat(pInfo.colColorStart[0], pInfo.colColorStart[0] + (pInfo.colColorEnd[0] - pInfo.colColorStart[0])*pInfo.fColorVar);
			tInst.colColor[1] = s_ParticleRandomizer.GetRandFloat(pInfo.colColorStart[1], pInfo.colColorStart[1] + (pInfo.colColorEnd[1] - pInfo.colColorStart[1])*pInfo.fColorVar);
			tInst.colColor[2] = s_ParticleRandomizer.GetRandFloat(pInfo.colColorStart[2], pInfo.colColorStart[2] + (pInfo.colColorEnd[2] - pInfo.colColorStart[2])*pInfo.fColorVar);
			tInst.colColor[3] = s_ParticleRandomizer.GetRandFloat(pInfo.colColorStart[3], pInfo.colColorStart[3] + (pInfo.colColorEnd[3] - pInfo.colColorStart[3])*pInfo.fAlphaVar);

			tInst.colColorDelta[0] = (pInfo.colColorEnd[0] - tInst.colColor[0]) / tInst.fTerminalAge;
			tInst.colColorDelta[1] = (pInfo.colColorEnd[1] - tInst.colColor[1]) / tInst.fTerminalAge;
			tInst.colColorDelta[2] = (pInfo.colColorEnd[2] - tInst.colColor[2]) / tInst.fTerminalAge;
			tInst.colColorDelta[3] = (pInfo.colColorEnd[3] - tInst.colColor[3]) / tInst.fTerminalAge;
		}
	}

	m_vPrevCenter = m_vCenter;
}

void ResParticle::ParticlePool::Render(f2dGraphics2D* graph, float scaleX, float scaleY)
{
	f2dSprite* p = m_pInstance->GetBindedSprite();
	const ParticleInfo& pInfo = m_pInstance->GetParticleInfo();
	fcyColor tOrgColor = p->GetColor(0U);

	for (size_t i = 0; i < m_iAlive; ++i)
	{
		ParticleInstance& pInst = m_ParticlePool[i];
		
		if (pInfo.colColorStart[0] < 0)  // r < 0
			p->SetColor(fcyColor(
				(int)(pInst.colColor[3] * 255),
				tOrgColor.r,
				tOrgColor.g,
				tOrgColor.b
			));
		else
			p->SetColor(fcyColor(pInst.colColor[3], pInst.colColor[0], pInst.colColor[1], pInst.colColor[2]));
		p->Draw2(graph, fcyVec2(pInst.vecLocation.x, pInst.vecLocation.y), fcyVec2(scaleX * pInst.fSize, scaleY * pInst.fSize), pInst.fSpin, false);
	}

	p->SetColor(tOrgColor);
}

////////////////////////////////////////////////////////////////////////////////
/// ResFont
////////////////////////////////////////////////////////////////////////////////
void ResFont::HGEFont::ReadDefine(const std::wstring& data, std::unordered_map<wchar_t, f2dGlyphInfo>& out, std::wstring& tex)
{
	out.clear();
	tex.clear();

	std::vector<std::wstring> tLines;
	fcyStringHelper::StringSplit(data, L"\n", true, tLines);
	for (auto& i : tLines)
	{
		i = fcyStringHelper::Trim(i);
	}

	// ��һ�б�����HGEFONT
	if (tLines.size() <= 1 || tLines[0] != L"[HGEFONT]")
		throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");

	for (size_t i = 1; i < tLines.size(); ++i)
	{
		wstring& tLine = tLines[i];
		if (tLine.size() == 0)
			continue;

		wstring::size_type tPos;
		if (string::npos == (tPos = tLine.find_first_of(L"=")))
			throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");
		wstring tKey = tLine.substr(0, tPos);
		wstring tValue = tLine.substr(tPos + 1, tLine.size() - tPos - 1);
		if (tKey == L"Bitmap")
			tex = tValue;
		else if (tKey == L"Char")
		{
			wchar_t c;
			int c_hex;
			float x, y, w, h, left_offset, right_offset;
			if (7 != swscanf(tValue.c_str(), L"\"%c\",%f,%f,%f,%f,%f,%f", &c, &x, &y, &w, &h, &left_offset, &right_offset))
			{
				if (7 != swscanf(tValue.c_str(), L"%X,%f,%f,%f,%f,%f,%f", &c_hex, &x, &y, &w, &h, &left_offset, &right_offset))
					throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");
				c = static_cast<wchar_t>(c_hex);
			}

			// ���㵽f2d����ƫ����
			f2dGlyphInfo tInfo = {
				fcyRect(x, y, x + w, y + h),
				fcyVec2(w, h),
				fcyVec2(-left_offset, h),
				fcyVec2(w + left_offset + right_offset, 0)
			};
			if (out.find(c) != out.end())
				throw fcyException("ResFont::HGEFont::readDefine", "Duplicated character defination.");
			out.emplace(c, tInfo);
		}
		else
			throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");
	}

	if (tex.empty())
		throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");
}

ResFont::HGEFont::HGEFont(std::unordered_map<wchar_t, f2dGlyphInfo>&& org, fcyRefPointer<f2dTexture2D> pTex)
	: m_Charset(std::move(org)), m_pTex(pTex)
{
	// �����������ΪLineHeight
	m_fLineHeight = 0;
	for (auto i = m_Charset.begin(); i != m_Charset.end(); ++i)
		m_fLineHeight = ::max(m_fLineHeight, i->second.GlyphSize.y);

	// ������������
	for (auto& i : m_Charset)
	{
		i.second.GlyphPos.a.x /= pTex->GetWidth();
		i.second.GlyphPos.b.x /= pTex->GetWidth();
		i.second.GlyphPos.a.y /= pTex->GetHeight();
		i.second.GlyphPos.b.y /= pTex->GetHeight();
	}
}

fFloat ResFont::HGEFont::GetLineHeight()
{
	return m_fLineHeight + 1.f;
}

fFloat ResFont::HGEFont::GetAscender()
{
	return m_fLineHeight;
}

fFloat ResFont::HGEFont::GetDescender()
{
	return 0.f;
}

f2dTexture2D* ResFont::HGEFont::GetCacheTexture()
{
	return m_pTex;
}

fResult ResFont::HGEFont::CacheString(fcStrW String)
{
	return FCYERR_OK;  // �������岻��Ҫʵ��CacheString
}

fResult ResFont::HGEFont::QueryGlyph(f2dGraphics* pGraph, fCharW Character, f2dGlyphInfo* InfoOut)
{
	unordered_map<wchar_t, f2dGlyphInfo>::iterator i = m_Charset.find(Character);

	if (i == m_Charset.end())
		return FCYERR_OBJNOTEXSIT;

	*InfoOut = i->second;

	return FCYERR_OK;
}

ResFont::ResFont(const char* name, fcyRefPointer<f2dFontProvider> pFont)
	: Resource(ResourceType::SpriteFont, name), m_pFontProvider(pFont)
{
}

////////////////////////////////////////////////////////////////////////////////
/// ResMusic
////////////////////////////////////////////////////////////////////////////////
fResult ResMusic::BGMWrapper::Read(fData pBuffer, fuInt SizeToRead, fuInt* pSizeRead)
{
	fResult tFR;

	// ��õ���������С
	fuInt tBlockAlign = GetBlockAlign();

	// ������Ҫ��ȡ�Ĳ�������
	fuInt tSampleToRead = SizeToRead / tBlockAlign;

	// �����Ƶ����
	while (tSampleToRead)
	{
		// ��õ�ǰ������λ��(����)
		fuInt tCurSample = (fuInt)GetPosition() / tBlockAlign;

		// ����ȡλ���Ƿ񳬳�ѭ����
		if (tCurSample + tSampleToRead > m_pLoopEndSample)
		{
			// �����δ�������
			if (tCurSample < m_pLoopEndSample)
			{
				fuInt tVaildSample = m_pLoopEndSample - tCurSample;
				fuInt tVaildSize = tVaildSample * tBlockAlign;

				if (FAILED(tFR = m_pDecoder->Read(pBuffer, tVaildSize, pSizeRead)))
					return tFR;

				// ָ�����
				pBuffer += tVaildSize;

				// ���ٲ���
				tSampleToRead -= tVaildSample;
			}

			// ����ѭ��ͷ
			SetPosition(FCYSEEKORIGIN_BEG, m_pLoopStartSample * tBlockAlign);
		}
		else
		{
			// ֱ���������
			if (FAILED(tFR = m_pDecoder->Read(pBuffer, tSampleToRead * tBlockAlign, pSizeRead)))
				return tFR;

			break;
		}
	}

	if (pSizeRead)
		*pSizeRead = SizeToRead;

	return FCYERR_OK;
}

ResMusic::BGMWrapper::BGMWrapper(fcyRefPointer<f2dSoundDecoder> pOrg, fDouble LoopStart, fDouble LoopEnd)
	: m_pDecoder(pOrg)
{
	LASSERT(pOrg);

	// �������
	m_TotalSample = m_pDecoder->GetBufferSize() / m_pDecoder->GetBlockAlign();

	if (LoopStart < 0)
		LoopStart = 0;
	m_pLoopStartSample = (fuInt)(LoopStart * m_pDecoder->GetSamplesPerSec());

	if (LoopEnd <= 0)
		m_pLoopEndSample = m_TotalSample;
	else
		m_pLoopEndSample = min(m_TotalSample, (fuInt)(LoopEnd * m_pDecoder->GetSamplesPerSec()));

	if (m_pLoopEndSample < m_pLoopStartSample)
		std::swap(m_pLoopStartSample, m_pLoopEndSample);

	if (m_pLoopEndSample == m_pLoopStartSample)
		throw fcyException("ResMusic::BGMWrapper::BGMWrapper", "Invalid loop period.");
}

////////////////////////////////////////////////////////////////////////////////
/// ResFX
////////////////////////////////////////////////////////////////////////////////
ResFX::ResFX(const char* name, fcyRefPointer<f2dEffect> shader)
	: Resource(ResourceType::FX, name), m_pShader(shader)
{
	// ɨ�����еı���ע��
	for (fuInt i = 0; i < m_pShader->GetParamCount(); ++i)
	{
		f2dEffectParam* pParam = m_pShader->GetParam(i);

		if (pParam->GetType() == F2DEPTYPE_VALUE)
		{
			f2dEffectParamValue* pValue = (f2dEffectParamValue*)pParam;

			// ע�Ͱ�
			f2dEffectParam* pAnnotation = pValue->GetAnnotation("binding");
			if (pAnnotation && pAnnotation->GetType() == F2DEPTYPE_VALUE)
			{
				f2dEffectParamValue* pAnnotationVal = (f2dEffectParamValue*)pAnnotation;
				if (pAnnotationVal->GetValueType() == F2DEPVTYPE_STRING && pAnnotationVal->GetString())
				{
					fcStr tBindingVar = pAnnotationVal->GetString();  // ���󶨵Ľű�����
					m_pBindingVar[tBindingVar].push_back(pValue);
				}
			}

			// �����
			fcStr pSemantic = pValue->GetSemantic();
			if (pSemantic)
			{
				if (_stricmp(pSemantic, "POSTEFFECTTEXTURE") == 0)
				{
					if (pValue->GetValueType() != F2DEPVTYPE_TEXTURE2D)
						throw fcyException("ResFX::ResFX", "Invalid binding for 'POSTEFFECTTEXTURE'.");
					m_pBindingPostEffectTexture.push_back(pValue);
				}
				else if (_stricmp(pSemantic, "VIEWPORT") == 0)
				{
					if (pValue->GetValueType() != F2DEPVTYPE_VECTOR)
						throw fcyException("ResFX::ResFX", "Invalid binding for 'VIEWPORT'.");
					m_pBindingViewport.push_back(pValue);
				}
				else if (_stricmp(pSemantic, "SCREENSIZE") == 0)
				{
					if (pValue->GetValueType() != F2DEPVTYPE_VECTOR)
						throw fcyException("ResFX::ResFX", "Invalid binding for 'SCREENSIZE'.");
					m_pBindingScreenSize.push_back(pValue);
				}
			}
		}
	}
}

void ResFX::SetPostEffectTexture(f2dTexture2D* val)LNOEXCEPT
{
	for (auto& p : m_pBindingPostEffectTexture)
		p->SetTexture(val);
}

void ResFX::SetViewport(fcyRect rect)LNOEXCEPT
{
	fcyVec4 vec(rect.a.x, rect.a.y, rect.b.x, rect.b.y);
	for (auto& p : m_pBindingViewport)
		p->SetVector(vec);
}

void ResFX::SetScreenSize(fcyVec2 size)LNOEXCEPT
{
	fcyVec4 vec(0.f, 0.f, size.x, size.y);
	for (auto& p : m_pBindingScreenSize)
		p->SetVector(vec);
}

void ResFX::SetValue(const char* key, float val)LNOEXCEPT
{
	auto i = m_pBindingVar.find(key);
	if (i != m_pBindingVar.end())
	{
		for (auto& p : i->second)
		{
			if (p->GetValueType() == F2DEPVTYPE_FLOAT)
				p->SetFloat(val);
		}
	}
}

void ResFX::SetValue(const char* key, fcyColor val)LNOEXCEPT
{
	auto i = m_pBindingVar.find(key);
	if (i != m_pBindingVar.end())
	{
		fcyVec4 tColorVec(val.r / 255.f, val.g / 255.f, val.b / 255.f, val.a / 255.f);

		for (auto& p : i->second)
		{
			if (p->GetValueType() == F2DEPVTYPE_VECTOR)
				p->SetVector(tColorVec);
		}
	}
}

void ResFX::SetValue(const char* key, f2dTexture2D* val)LNOEXCEPT
{
	auto i = m_pBindingVar.find(key);
	if (i != m_pBindingVar.end())
	{
		for (auto& p : i->second)
		{
			if (p->GetValueType() == F2DEPVTYPE_TEXTURE2D)
				p->SetTexture(val);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
/// ResourcePool
////////////////////////////////////////////////////////////////////////////////
void ResourcePool::Clear()LNOEXCEPT
{
	m_TexturePool.clear();
	m_SpritePool.clear();
	m_AnimationPool.clear();
	m_MusicPool.clear();
	m_SoundSpritePool.clear();
	m_ParticlePool.clear();
	m_SpriteFontPool.clear();
	m_TTFFontPool.clear();
	m_FXPool.clear();

#if (defined LDEVVERSION) || (defined LDEBUG)
	LAPP.SendResourceClearedHint(m_iType);
#endif
}

void ResourcePool::RemoveResource(ResourceType t, const char* name)LNOEXCEPT
{
	switch (t)
	{
	case ResourceType::Texture:
		removeResource(m_TexturePool, name);
		break;
	case ResourceType::Sprite:
		removeResource(m_SpritePool, name);
		break;
	case ResourceType::Animation:
		removeResource(m_AnimationPool, name);
		break;
	case ResourceType::Music:
		removeResource(m_MusicPool, name);
		break;
	case ResourceType::SoundEffect:
		removeResource(m_SoundSpritePool, name);
		break;
	case ResourceType::Particle:
		removeResource(m_ParticlePool, name);
		break;
	case ResourceType::SpriteFont:
		removeResource(m_SpriteFontPool, name);
		break;
	case ResourceType::TrueTypeFont:
		removeResource(m_TTFFontPool, name);
		break;
	case ResourceType::FX:
		removeResource(m_FXPool, name);
		break;
	default:
		return;
	}

#if (defined LDEVVERSION) || (defined LDEBUG)
	LAPP.SendResourceRemovedHint(t, m_iType, name);
#endif
}

void ResourcePool::ExportResourceList(lua_State* L, ResourceType t)const LNOEXCEPT
{
	int cnt = 1;
	switch (t)
	{
	case ResourceType::Texture:
		lua_newtable(L);  // t
		for (auto& i : m_TexturePool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Sprite:
		lua_newtable(L);  // t
		for (auto& i : m_SpritePool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Animation:
		lua_newtable(L);  // t
		for (auto& i : m_AnimationPool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Music:
		lua_newtable(L);  // t
		for (auto& i : m_MusicPool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::SoundEffect:
		lua_newtable(L);  // t
		for (auto& i : m_SoundSpritePool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Particle:
		lua_newtable(L);  // t
		for (auto& i : m_ParticlePool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::SpriteFont:
		lua_newtable(L);  // t
		for (auto& i : m_SpriteFontPool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::TrueTypeFont:
		lua_newtable(L);  // t
		for (auto& i : m_TTFFontPool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::FX:
		lua_newtable(L);  // t
		for (auto& i : m_FXPool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	default:
		lua_pushnil(L);
		break;
	}
}

bool ResourcePool::LoadTexture(const char* name, const std::wstring& path, bool mipmaps)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderDev());

		if (m_TexturePool.find(name) != m_TexturePool.end())
		{
			LWARNING("LoadTexture: ����'%m'�Ѵ��ڣ���ͼʹ��'%s'���صĲ����ѱ�ȡ��", name, path.c_str());
			return true;
		}

		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
			return false;

		fcyRefPointer<f2dTexture2D> tTexture;
		if (FCYFAILED(LAPP.GetRenderDev()->CreateTextureFromMemory((fcData)tDataBuf->GetInternalBuffer(), tDataBuf->GetLength(), 0, 0, false, mipmaps, &tTexture)))
		{
			LERROR("LoadTexture: ���ļ�'%s'��������'%m'ʧ��", path.c_str(), name);
			return false;
		}

		try
		{
			fcyRefPointer<ResTexture> tRes;
			tRes.DirectSet(new ResTexture(name, tTexture));
			m_TexturePool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadTexture: �ڴ治��");
			return false;
		}

#ifdef LSHOWRESLOADINFO
		LINFO("LoadTexture: ����'%s'��װ�� -> '%m' (%s)", path.c_str(), name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Texture, path.c_str());
	return true;
}

bool ResourcePool::LoadTexture(const char* name, const char* path, bool mipmaps)LNOEXCEPT
{
	try
	{
		return LoadTexture(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), mipmaps);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadTexture: ת������ʱ�޷������ڴ�");
		return false;
	}
}

bool ResourcePool::LoadImage(const char* name, const char* texname,
	double x, double y, double w, double h, double a, double b, bool rect)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderer());

		if (m_SpritePool.find(name) != m_SpritePool.end())
		{
			LWARNING("LoadImage: ͼ��'%m'�Ѵ��ڣ����ز����ѱ�ȡ��", name);
			return true;
		}

		fcyRefPointer<ResTexture> pTex = m_pMgr->FindTexture(texname);
		if (!pTex)
		{
			LWARNING("LoadImage: ����ͼ��'%m'ʧ��, �޷��ҵ�����'%m'", name, texname);
			return false;
		}

		fcyRefPointer<f2dSprite> pSprite;
		fcyRect tRect((float)x, (float)y, (float)(x + w), (float)(y + h));
		if (FCYFAILED(LAPP.GetRenderer()->CreateSprite2D(pTex->GetTexture(), tRect, &pSprite)))
		{
			LERROR("LoadImage: �޷�������'%m'����ͼ��'%m' (CreateSprite2D failed)", texname, name);
			return false;
		}

		try
		{
			fcyRefPointer<ResSprite> tRes;
			tRes.DirectSet(new ResSprite(name, pSprite, a, b, rect));
			m_SpritePool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadImage: �ڴ治��");
			return false;
		}

#ifdef LSHOWRESLOADINFO
		LINFO("LoadImage: ͼ��'%m'��װ�� (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Sprite, L"N/A");
	return true;
}

bool ResourcePool::LoadAnimation(const char* name, const char* texname,
	double x, double y, double w, double h, int n, int m, int intv, double a, double b, bool rect)
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		if (m_AnimationPool.find(name) != m_AnimationPool.end())
		{
			LWARNING("LoadAnimation: ����'%m'�Ѵ��ڣ����ز����ѱ�ȡ��", name);
			return true;
		}

		fcyRefPointer<ResTexture> pTex = m_pMgr->FindTexture(texname);
		if (!pTex)
		{
			LWARNING("LoadAnimation: ���ض���'%m'ʧ��, �޷��ҵ�����'%m'", name, texname);
			return false;
		}

		try
		{
			fcyRefPointer<ResAnimation> tRes;
			tRes.DirectSet(new ResAnimation(name, pTex, (float)x, (float)y, (float)w, (float)h, n, m, intv, a, b, rect));
			m_AnimationPool.emplace(name, tRes);
		}
		catch (const fcyException&)
		{
			LERROR("LoadAnimation: ���춯��'%m'ʱʧ��", name);
			return false;
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadAnimation: �ڴ治��");
			return false;
		}

#ifdef LSHOWRESLOADINFO
		LINFO("LoadAnimation: ����'%m'��װ�� (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Animation, L"N/A");
	return true;
}

bool ResourcePool::LoadMusic(const char* name, const std::wstring& path, double start, double end)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetSoundSys());

		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
			return false;

		try
		{
			fcyRefPointer<f2dSoundDecoder> tDecoder;
			if (FCYFAILED(LAPP.GetSoundSys()->CreateOGGVorbisDecoder(tDataBuf, &tDecoder)))
			{
				tDataBuf->SetPosition(FCYSEEKORIGIN_BEG, 0);
				if (FCYFAILED(LAPP.GetSoundSys()->CreateWaveDecoder(tDataBuf, &tDecoder)))
				{
					LERROR("LoadMusic: �޷������ļ�'%s'", path.c_str());
					return false;
				}
			}

			fcyRefPointer<ResMusic::BGMWrapper> tWrapperedBuffer;
			tWrapperedBuffer.DirectSet(new ResMusic::BGMWrapper(tDecoder, start, end));

			fcyRefPointer<f2dSoundBuffer> tBuffer;
			if (FCYFAILED(LAPP.GetSoundSys()->CreateDynamicBuffer(tWrapperedBuffer, false, &tBuffer)))
			{
				LERROR("LoadMusic: �޷�������Ƶ���������ļ�'%s' (f2dSoundSys::CreateDynamicBuffer failed.)", path.c_str());
				return false;
			}

			fcyRefPointer<ResMusic> tRes;
			tRes.DirectSet(new ResMusic(name, tBuffer));
			m_MusicPool.emplace(name, tRes);
		}
		catch (const fcyException& e)
		{
			LERROR("LoadMusic: �����ļ�'%s'����Ƶ����ʱ�������󣬸�ʽ��֧�֣� (�쳣��Ϣ'%m' Դ'%m')", path.c_str(), e.GetDesc(), e.GetSrc());
			return false;
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadMusic: �ڴ治��");
			return false;
		}

#ifdef LSHOWRESLOADINFO
		LINFO("LoadMusic: BGM'%m'��װ�� (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Music, path.c_str());
	return true;
}

LNOINLINE bool ResourcePool::LoadMusic(const char* name, const char* path, double start, double end)LNOEXCEPT
{
	try
	{
		return LoadMusic(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), start, end);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadMusic: ת������ʱ�޷������ڴ�");
		return false;
	}
}

bool ResourcePool::LoadSound(const char* name, const std::wstring& path)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetSoundSys());

		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
			return false;

		try
		{
			fcyRefPointer<f2dSoundDecoder> tDecoder;
			if (FCYFAILED(LAPP.GetSoundSys()->CreateWaveDecoder(tDataBuf, &tDecoder)))
			{
				tDataBuf->SetPosition(FCYSEEKORIGIN_BEG, 0);
				if (FCYFAILED(LAPP.GetSoundSys()->CreateOGGVorbisDecoder(tDataBuf, &tDecoder)))
				{
					LERROR("LoadSound: �޷������ļ�'%s'", path.c_str());
					return false;
				}
			}

			fcyRefPointer<f2dSoundBuffer> tBuffer;
			if (FCYFAILED(LAPP.GetSoundSys()->CreateStaticBuffer(tDecoder, false, &tBuffer)))
			{
				LERROR("LoadSound: �޷�������Ƶ���������ļ�'%s' (f2dSoundSys::CreateStaticBuffer failed.)", path.c_str());
				return false;
			}

			fcyRefPointer<ResSound> tRes;
			tRes.DirectSet(new ResSound(name, tBuffer));
			m_SoundSpritePool.emplace(name, tRes);
		}
		catch (const fcyException& e)
		{
			LERROR("LoadSound: �����ļ�'%s'����Ƶ����ʱ�������󣬸�ʽ��֧�֣� (�쳣��Ϣ'%m' Դ'%m')", path.c_str(), e.GetDesc(), e.GetSrc());
			return false;
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadSound: �ڴ治��");
			return false;
		}

#ifdef LSHOWRESLOADINFO
		LINFO("LoadSound: ��Ч'%m'��װ�� (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::SoundEffect, path.c_str());
	return true;
}

LNOINLINE bool ResourcePool::LoadSound(const char* name, const char* path)LNOEXCEPT
{
	try
	{
		return LoadSound(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8));
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadSound: ת������ʱ�޷������ڴ�");
		return false;
	}
}

bool ResourcePool::LoadParticle(const char* name, const std::wstring& path, const char* img_name, double a, double b, bool rect)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderer());

		if (m_ParticlePool.find(name) != m_ParticlePool.end())
		{
			LWARNING("LoadParticle: ����'%m'�Ѵ��ڣ����ز����ѱ�ȡ��", name);
			return true;
		}

		fcyRefPointer<ResSprite> pSprite = m_pMgr->FindSprite(img_name);
		fcyRefPointer<f2dSprite> pClone;
		if (!pSprite)
		{
			LWARNING("LoadParticle: ��������'%m'ʧ��, �޷��ҵ�����'%m'", name, img_name);
			return false;
		}
		else
		{
			// ��¡һ���������
			if (FCYFAILED(LAPP.GetRenderer()->CreateSprite2D(pSprite->GetSprite()->GetTexture(), pSprite->GetSprite()->GetTexRect(), pSprite->GetSprite()->GetHotSpot(), &pClone)))
			{
				LERROR("LoadParticle: ��¡ͼƬ'%m'ʧ��", img_name);
				return false;
			}
			pClone->SetColor(0, pSprite->GetSprite()->GetColor(0U));
			pClone->SetColor(1, pSprite->GetSprite()->GetColor(1U));
			pClone->SetColor(2, pSprite->GetSprite()->GetColor(2U));
			pClone->SetColor(3, pSprite->GetSprite()->GetColor(3U));
			pClone->SetZ(pSprite->GetSprite()->GetZ());
		}

		fcyRefPointer<fcyMemStream> outBuf;
		if (!LRES.LoadFile(path.c_str(), outBuf))
			return false;
		if (outBuf->GetLength() != sizeof(ResParticle::ParticleInfo))
		{
			LERROR("LoadParticle: ���Ӷ����ļ�'%s'��ʽ����ȷ", path.c_str());
			return false;
		}

		try
		{
			ResParticle::ParticleInfo tInfo;
			memcpy(&tInfo, outBuf->GetInternalBuffer(), sizeof(ResParticle::ParticleInfo));
			tInfo.iBlendInfo = (tInfo.iBlendInfo >> 16) & 0x00000003;

			BlendMode tBlendInfo = BlendMode::AddAlpha;
			if (tInfo.iBlendInfo & 1)  // ADD
			{
				if (tInfo.iBlendInfo & 2)  // ALPHA
					tBlendInfo = BlendMode::AddAlpha;
				else
					tBlendInfo = BlendMode::AddAdd;
			}
			else  // MUL
			{
				if (tInfo.iBlendInfo & 2)  // ALPHA
					tBlendInfo = BlendMode::MulAlpha;
				else
					tBlendInfo = BlendMode::MulAdd;
			}

			fcyRefPointer<ResParticle> tRes;
			tRes.DirectSet(new ResParticle(name, tInfo, pClone, tBlendInfo, a, b, rect));
			m_ParticlePool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadParticle: �ڴ治��");
			return false;
		}
#ifdef LSHOWRESLOADINFO
		LINFO("LoadParticle: ����'%m'��װ�� (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::Particle, path.c_str());
	return true;
}

LNOINLINE bool ResourcePool::LoadParticle(const char* name, const char* path, const char* img_name, double a, double b, bool rect)LNOEXCEPT
{
	try
	{
		return LoadParticle(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), img_name, a, b, rect);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadParticle: ת������ʱ�޷������ڴ�");
		return false;
	}
}

bool ResourcePool::LoadSpriteFont(const char* name, const std::wstring& path, bool mipmaps)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderer());

		if (m_SpriteFontPool.find(name) != m_SpriteFontPool.end())
		{
			LWARNING("LoadFont: ����'%m'�Ѵ��ڣ����ز����ѱ�ȡ��", name);
			return true;
		}

		std::unordered_map<wchar_t, f2dGlyphInfo> tOutputCharset;
		std::wstring tOutputTextureName;

		// ��ȡ�ļ�
		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
			return false;

		// ת������
		wstring tFileData;
		try
		{
			if (tDataBuf->GetLength() > 0)
			{
				// stupid
				tFileData = fcyStringHelper::MultiByteToWideChar(string((const char*)tDataBuf->GetInternalBuffer(), (size_t)tDataBuf->GetLength()), CP_UTF8);
			}
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: ת������ʱ�޷������ڴ�");
			return false;
		}

		// ��ȡHGE���嶨��
		try
		{
			ResFont::HGEFont::ReadDefine(tFileData, tOutputCharset, tOutputTextureName);
		}
		catch (const fcyException& e)
		{
			LERROR("LoadFont: װ��HGE���嶨���ļ�'%s'ʧ�� (�쳣��Ϣ'%m' Դ'%m')", path.c_str(), e.GetDesc(), e.GetSrc());
			return false;
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: �ڴ治��");
			return false;
		}

		// װ������
		try
		{
			if (!m_pMgr->LoadFile((fcyPathParser::GetPath(path) + tOutputTextureName).c_str(), tDataBuf))
				return false;
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: �ڴ治��");
			return false;
		}

		fcyRefPointer<f2dTexture2D> tTexture;
		if (FCYFAILED(LAPP.GetRenderDev()->CreateTextureFromMemory((fcData)tDataBuf->GetInternalBuffer(), tDataBuf->GetLength(), 0, 0, false, mipmaps, &tTexture)))
		{
			LERROR("LoadFont: ���ļ�'%s'��������'%m'ʧ��", tOutputTextureName.c_str(), name);
			return false;
		}

		// ��������
		try
		{
			fcyRefPointer<f2dFontProvider> tFontProvider;
			tFontProvider.DirectSet(new ResFont::HGEFont(std::move(tOutputCharset), tTexture));

			fcyRefPointer<ResFont> tRes;
			tRes.DirectSet(new ResFont(name, tFontProvider));
			m_SpriteFontPool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: �ڴ治��");
			return false;
		}
#ifdef LSHOWRESLOADINFO
		LINFO("LoadFont: ��������'%m'��װ�� (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::SpriteFont, path.c_str());
	return true;
}

bool ResourcePool::LoadSpriteFont(const char* name, const std::wstring& path, const std::wstring& tex_path, bool mipmaps)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderer());

		if (m_SpriteFontPool.find(name) != m_SpriteFontPool.end())
		{
			LWARNING("LoadFont: ����'%m'�Ѵ��ڣ����ز����ѱ�ȡ��", name);
			return true;
		}

		// ��ȡ�ļ�
		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
			return false;

		// ת������
		wstring tFileData;
		try
		{
			if (tDataBuf->GetLength() > 0)
			{
				// stupid
				tFileData = fcyStringHelper::MultiByteToWideChar(string((const char*)tDataBuf->GetInternalBuffer(), (size_t)tDataBuf->GetLength()), CP_UTF8);
			}
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: ת������ʱ�޷������ڴ�");
			return false;
		}

		// װ������
		try
		{
			if (!m_pMgr->LoadFile((fcyPathParser::GetPath(path) + tex_path).c_str(), tDataBuf))
			{
				if (!m_pMgr->LoadFile(tex_path.c_str(), tDataBuf))
					return false;
			}
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: �ڴ治��");
			return false;
		}

		fcyRefPointer<f2dTexture2D> tTexture;
		if (FCYFAILED(LAPP.GetRenderDev()->CreateTextureFromMemory((fcData)tDataBuf->GetInternalBuffer(), tDataBuf->GetLength(), 0, 0, false, mipmaps, &tTexture)))
		{
			LERROR("LoadFont: ���ļ�'%s'��������'%m'ʧ��", tex_path.c_str(), name);
			return false;
		}

		// ��������
		try
		{
			fcyRefPointer<f2dFontProvider> tFontProvider;
			if (FCYFAILED(LAPP.GetRenderer()->CreateFontFromTex(tFileData.c_str(), tTexture, &tFontProvider)))
			{
				LERROR("LoadFont: ���ļ�'%s'������������ʧ��", path.c_str());
				return false;
			}

			fcyRefPointer<ResFont> tRes;
			tRes.DirectSet(new ResFont(name, tFontProvider));
			m_SpriteFontPool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFont: �ڴ治��");
			return false;
		}
#ifdef LSHOWRESLOADINFO
		LINFO("LoadFont: ��������'%m'��װ�� (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::SpriteFont, path.c_str());
	return true;
}

LNOINLINE bool ResourcePool::LoadSpriteFont(const char* name, const char* path, bool mipmaps)LNOEXCEPT
{
	try
	{
		return LoadSpriteFont(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), mipmaps);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadSpriteFont: ת������ʱ�޷������ڴ�");
		return false;
	}
}

LNOINLINE bool ResourcePool::LoadSpriteFont(const char* name, const char* path, const char* tex_path, bool mipmaps)LNOEXCEPT
{
	try
	{
		return LoadSpriteFont(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), fcyStringHelper::MultiByteToWideChar(tex_path, CP_UTF8), mipmaps);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadSpriteFont: ת������ʱ�޷������ڴ�");
		return false;
	}
}

bool ResourcePool::LoadTTFFont(const char* name, const std::wstring& path, float width, float height)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderer());

		if (m_TTFFontPool.find(name) != m_TTFFontPool.end())
		{
			LWARNING("LoadTTFFont: ����'%m'�Ѵ��ڣ����ز����ѱ�ȡ��", name);
			return true;
		}

		fcyRefPointer<f2dFontProvider> tFontProvider;

		// ��ȡ�ļ�
		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
		{
			LINFO("LoadTTFFont: �޷���·��'%s'�ϼ������壬������ϵͳ����Դ�������ϵͳ����", path.c_str());
			if (FCYFAILED(LAPP.GetRenderer()->CreateSystemFont(path.c_str(), 0, fcyVec2(width, height), F2DFONTFLAG_NONE, &tFontProvider)))
			{
				LERROR("LoadTTFFont: ����ʧ�ܣ��޷���·��'%s'�ϼ�������", path.c_str());
				return false;
			}
		}

		// ��������
		try
		{
			if (!tFontProvider)
			{
				if (FCYFAILED(LAPP.GetRenderer()->CreateFontFromFile(tDataBuf, 0, fcyVec2(width, height), F2DFONTFLAG_NONE, &tFontProvider)))
				{
					LERROR("LoadTTFFont: ���ļ�'%s'������������ʧ��", path.c_str());
					return false;
				}
			}

			fcyRefPointer<ResFont> tRes;
			tRes.DirectSet(new ResFont(name, tFontProvider));
			tRes->SetBlendMode(BlendMode::AddAlpha);
			m_TTFFontPool.emplace(name, tRes);
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadTTFFont: �ڴ治��");
			return false;
		}
#ifdef LSHOWRESLOADINFO
		LINFO("LoadTTFFont: truetype����'%m'��װ�� (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::TrueTypeFont, path.c_str());
	return true;
}

LNOINLINE bool ResourcePool::LoadTTFFont(const char* name, const char* path, float width, float height)LNOEXCEPT
{
	try
	{
		return LoadTTFFont(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8), width, height);
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadTTFFont: ת������ʱ�޷������ڴ�");
		return false;
	}
}

bool ResourcePool::LoadFX(const char* name, const std::wstring& path)LNOEXCEPT
{
	LDEBUG_RESOURCETIMER;

	{
		LDEBUG_RESOURCESCOPE;

		LASSERT(LAPP.GetRenderDev());

		if (m_FXPool.find(name) != m_FXPool.end())
		{
			LWARNING("LoadFX: FX'%m'�Ѵ��ڣ����ز����ѱ�ȡ��", name);
			return true;
		}

		// ��ȡ�ļ�
		fcyRefPointer<fcyMemStream> tDataBuf;
		if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
			return false;

		try
		{
			fcyRefPointer<f2dEffect> tEffect;
			if (FCYFAILED(LAPP.GetRenderDev()->CreateEffect(tDataBuf, false, &tEffect)))
			{
				LERROR("LoadFX: ����shader���ļ�'%s'ʧ�� (lasterr=%m)", path.c_str(), LAPP.GetEngine()->GetLastErrDesc());
				return false;
			}

			fcyRefPointer<ResFX> tRes;
			tRes.DirectSet(new ResFX(name, tEffect));
			m_FXPool.emplace(name, tRes);
		}
		catch (const fcyException& e)
		{
			LERROR("LoadFX: �󶨱������ļ�'%s'ʧ�� (�쳣��Ϣ'%m' Դ'%m')", path.c_str(), e.GetDesc(), e.GetSrc());
			return false;
		}
		catch (const bad_alloc&)
		{
			LERROR("LoadFX: �ڴ治��");
			return false;
		}
#ifdef LSHOWRESLOADINFO
		LINFO("LoadFX: FX'%m'��װ�� (%s)", name, getResourcePoolTypeName());
#endif
	}

	LDEBUG_RESOURCEHINT(ResourceType::FX, path.c_str());
	return true;
}

LNOINLINE bool ResourcePool::LoadFX(const char* name, const char* path)LNOEXCEPT
{
	try
	{
		return LoadFX(name, fcyStringHelper::MultiByteToWideChar(path, CP_UTF8));
	}
	catch (const bad_alloc&)
	{
		LERROR("LoadFX: ת������ʱ�޷������ڴ�");
		return false;
	}
}

////////////////////////////////////////////////////////////////////////////////
/// ResourcePack
////////////////////////////////////////////////////////////////////////////////
template <typename T>
void pathUniform(T begin, T end)
{
	while (begin != end)
	{
		int c = *begin;
		if (c == '/')
			*begin = '\\';
		else if (c >= 'A' && c <= 'Z')
			*begin = c - 'A' + 'a';
		else if (c == 0)
			break;
		++begin;
	}
}

ResourcePack::ResourcePack(const wchar_t* path, const char* passwd)
	: m_Path(path), m_PathLowerCase(path), m_Password(passwd ? passwd : "")
{
	pathUniform(m_PathLowerCase.begin(), m_PathLowerCase.end());

	zlib_filefunc64_def tZlibFileFunc;
	memset(&tZlibFileFunc, 0, sizeof(tZlibFileFunc));
	fill_wfopen64_filefunc(&tZlibFileFunc);
	m_zipFile = unzOpen2_64(reinterpret_cast<const char*>(path), &tZlibFileFunc);
	if (!m_zipFile)
	{
		LERROR("ResourcePack: �޷�����Դ��'%s' (unzOpenʧ��)", path);
		throw fcyException("ResourcePack::ResourcePack", "Can't open resource pack.");
	}
}

ResourcePack::~ResourcePack()
{
	unzClose(m_zipFile);
}

bool ResourcePack::LoadFile(const wchar_t* path, fcyRefPointer<fcyMemStream>& outBuf)LNOEXCEPT
{
	string tPathInUtf8;
	try
	{
		tPathInUtf8 = fcyStringHelper::WideCharToMultiByte(path, CP_UTF8);
		pathUniform(tPathInUtf8.begin(), tPathInUtf8.end());
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourcePack: ת����ԴĿ¼����ʱ�޷������ڴ�");
		return false;
	}

	int tStatus = unzGoToFirstFile(m_zipFile);
	while (UNZ_OK == tStatus)
	{
		unz_file_info tFileInfo;
		char tZipName[MAX_PATH];

		if (UNZ_OK == unzGetCurrentFileInfo(m_zipFile, &tFileInfo, tZipName, sizeof(tZipName), nullptr, 0, nullptr, 0))
		{
			// ��·����ͳһ��ת��
			pathUniform(tZipName, tZipName + MAX_PATH);

			// ���·���Ƿ�����
			if (strcmp(tPathInUtf8.c_str(), tZipName) == 0)
			{
#ifdef LSHOWRESLOADINFO
				LINFO("ResourcePack: ��Դ��'%s'�����ļ�'%s'", m_Path.c_str(), path);
#endif
				if (unzOpenCurrentFilePassword(m_zipFile, m_Password.length() > 0 ? m_Password.c_str() : nullptr) != UNZ_OK)
				{
					LERROR("ResourcePack: ���Դ���Դ��'%s'�е��ļ�'%s'ʧ��(�������?)", m_Path.c_str(), path);
					return false;
				}

				try
				{
					outBuf.DirectSet(new fcyMemStream(NULL, tFileInfo.uncompressed_size, true, false));
				}
				catch (const bad_alloc&)
				{
					unzCloseCurrentFile(m_zipFile);
					LERROR("ResourcePack: �޷������㹻�ڴ��ѹ��Դ��'%s'�е��ļ�'%s'", m_Path.c_str(), path);
					return false;
				}
				
				if (outBuf->GetLength() > 0)
				{
					if (unzReadCurrentFile(m_zipFile, outBuf->GetInternalBuffer(), tFileInfo.uncompressed_size) < 0)
					{
						unzCloseCurrentFile(m_zipFile);
						LERROR("ResourcePack: ��ѹ��Դ��'%s'�е��ļ�'%s'ʧ�� (unzReadCurrentFileʧ��)", m_Path.c_str(), path);
						return false;
					}
				}
				unzCloseCurrentFile(m_zipFile);

				return true;
			}
		}
		else
			LWARNING("ResourcePack: ����Դ��'%s'��Ѱ���ļ�ʱ�������� (unzGetCurrentFileInfoʧ��)", m_Path.c_str());

		tStatus = unzGoToNextFile(m_zipFile);
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
/// ResourceMgr
////////////////////////////////////////////////////////////////////////////////
ResourceMgr::ResourceMgr()
	: m_GlobalResourcePool(this, ResourcePoolType::Global), m_StageResourcePool(this, ResourcePoolType::Stage)
{
}

void ResourceMgr::ClearAllResource()LNOEXCEPT
{
	m_GlobalResourcePool.Clear();
	m_StageResourcePool.Clear();
	m_ActivedPool = ResourcePoolType::Global;
	m_GlobalImageScaleFactor = 1.;
	m_GlobalSoundEffectVolume = 1.0f;
	m_GlobalMusicVolume = 1.0f;
}

bool ResourceMgr::LoadPack(const wchar_t* path, const char* passwd)LNOEXCEPT
{
	try
	{
		wstring tPath = path;
		pathUniform(tPath.begin(), tPath.end());
		for (auto& i : m_ResPackList)
		{
			if (i.GetPathLowerCase() == tPath)
			{
				LWARNING("ResourceMgr: ��Դ��'%s'�Ѽ��أ������ظ�����", path);
				return true;
			}
		}
		m_ResPackList.emplace_front(path, passwd);
		LINFO("ResourceMgr: ��װ����Դ��'%s'", path);
		return true;
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: ������Դ��ʱ�޷������㹻�ڴ�");
	}
	catch (const fcyException&)
	{
	}
	return false;
}

void ResourceMgr::UnloadPack(const wchar_t* path)LNOEXCEPT
{
	try
	{
		wstring tPath = path;
		pathUniform(tPath.begin(), tPath.end());
		for (auto& i = m_ResPackList.begin(); i != m_ResPackList.end(); ++i)
		{
			if (i->GetPathLowerCase() == tPath)
			{
				m_ResPackList.erase(i);
				LINFO("ResourceMgr: ��ж����Դ��'%s'", path);
				return;
			}
		}
		LWARNING("ResourceMgr: ��Դ��'%s'δ���أ��޷�ж��", path);
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: ж����Դ��ʱ�޷������㹻�ڴ�");
	}
}

LNOINLINE bool ResourceMgr::LoadPack(const char* path, const char* passwd)LNOEXCEPT
{
	try
	{
		wstring tPath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
		return LoadPack(tPath.c_str(), passwd);
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: ת���ַ�����ʱ�޷������ڴ�");
		return false;
	}
}

LNOINLINE void ResourceMgr::UnloadPack(const char* path)LNOEXCEPT
{
	try
	{
		wstring tPath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
		UnloadPack(tPath.c_str());
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: ת���ַ�����ʱ�޷������ڴ�");
	}
}

LNOINLINE bool ResourceMgr::LoadFile(const wchar_t* path, fcyRefPointer<fcyMemStream>& outBuf)LNOEXCEPT
{
	// ���ԴӸ�����Դ������
	for (auto& i : m_ResPackList)
	{
		if (i.LoadFile(path, outBuf))
			return true;
	}

	// ���Դӱ��ؼ���
#ifdef LSHOWRESLOADINFO
	LINFO("ResourceMgr: ���Դӱ��ؼ����ļ�'%s'", path);
#endif
	fcyRefPointer<fcyFileStream> pFile;
	try
	{
		pFile.DirectSet(new fcyFileStream(path, false));
		outBuf.DirectSet(new fcyMemStream(NULL, pFile->GetLength(), true, false));
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: �޷������㹻�ڴ�ӱ��ؼ����ļ�'%s'", path);
		return false;
	}
	catch (const fcyException& e)
	{
		LERROR("ResourceMgr: װ�ر����ļ�'%s'ʧ�ܣ��ļ������ڣ� (�쳣��Ϣ'%m' Դ'%m')", path, e.GetDesc(), e.GetSrc());
		return false;
	}

	if (pFile->GetLength() > 0)
	{
		if (FCYFAILED(pFile->ReadBytes((fData)outBuf->GetInternalBuffer(), outBuf->GetLength(), nullptr)))
		{
			LERROR("ResourceMgr: ��ȡ�����ļ�'%s'ʧ�� (fcyFileStream::ReadBytesʧ��)", path);
			return false;
		}
	}

	return true;
}

LNOINLINE bool ResourceMgr::LoadFile(const char* path, fcyRefPointer<fcyMemStream>& outBuf)LNOEXCEPT
{
	try
	{
		wstring tPath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
		return LoadFile(tPath.c_str(), outBuf);
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: ת���ַ�����ʱ�޷������ڴ�");
	}
	return false;
}

bool ResourceMgr::ExtractRes(const wchar_t* path, const wchar_t* target)LNOEXCEPT
{
	fcyRefPointer<fcyMemStream> tBuf;

	// ��ȡ�ļ�
	if (LoadFile(path, tBuf))
	{
		// �򿪱����ļ�
		fcyRefPointer<fcyFileStream> pFile;
		try
		{
			pFile.DirectSet(new fcyFileStream(target, true));
			if (FCYFAILED(pFile->SetLength(0)))
			{
				LERROR("ResourceMgr: �޷�����ļ�'%s' (fcyFileStream::SetLength ʧ��)", target);
				return false;
			}
			if (tBuf->GetLength() > 0)
			{
				if (FCYFAILED(pFile->WriteBytes((fcData)tBuf->GetInternalBuffer(), tBuf->GetLength(), nullptr)))
				{
					LERROR("ResourceMgr: �޷����ļ�'%s'д������", target);
					return false;
				}
			}
		}
		catch (const bad_alloc&)
		{
			LERROR("ResourceMgr: �޷������㹻�ڴ������ļ�'%s'д������", target);
			return false;
		}
		catch (const fcyException& e)
		{
			LERROR("ResourceMgr: �򿪱����ļ�'%s'ʧ�� (�쳣��Ϣ'%m' Դ'%m')", target, e.GetDesc(), e.GetSrc());
			return false;
		}
	}
	return true;
}

LNOINLINE bool ResourceMgr::ExtractRes(const char* path, const char* target)LNOEXCEPT
{
	try
	{
		wstring tPath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
		wstring tTarget = fcyStringHelper::MultiByteToWideChar(target, CP_UTF8);
		return ExtractRes(tPath.c_str(), tTarget.c_str());
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: ת���ַ�����ʱ�޷������ڴ�");
	}
	return false;
}
