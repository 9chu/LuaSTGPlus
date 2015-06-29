#include "ResourceMgr.h"
#include "AppFrame.h"

#include <iowin32.h>

using namespace std;
using namespace LuaSTGPlus;

FixedObjectPool<ResParticle::PARTICLE_POD, LPARTICLESYS_MAX> ResParticle::s_MemoryPool;

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

ResParticle::ResParticle(const char* name, const ParticleInfo& pinfo, fcyRefPointer<f2dSprite> sprite, BlendMode bld, double a, double b, bool rect)
	: Resource(ResourceType::Particle, name), m_ParticleInfo(pinfo), m_BindedSprite(sprite), m_BlendMode(bld), m_HalfSizeX(a), m_HalfSizeY(b), m_bRectangle(rect)
{
}

ResParticle::ParticlePool* ResParticle::AllocInstance()LNOEXCEPT
{
	size_t id;
	if (!s_MemoryPool.Alloc(id))
		return nullptr;
	ParticlePool* pRet = new(s_MemoryPool.Data(id)) ParticlePool(id, this);
	pRet->SetBlendMode(m_BlendMode);
	return pRet;
}

void ResParticle::FreeInstance(ResParticle::ParticlePool* p)LNOEXCEPT
{
	p->~ParticlePool();
	s_MemoryPool.Free(p->m_iId);
}

ResParticle::ParticlePool::ParticlePool(size_t id, fcyRefPointer<ResParticle> ref)
	: m_iId(id), m_pInstance(ref), m_fEmission((float)ref->GetParticleInfo().nEmission) {}

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
/// ResourcePool
////////////////////////////////////////////////////////////////////////////////
void ResourcePool::ExportResourceList(lua_State* L, ResourceType t)const LNOEXCEPT
{
	int cnt = 1;
	switch (t)
	{
	case ResourceType::Texture:
		lua_newtable(L);  // t
		for (auto i : m_TexturePool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Sprite:
		lua_newtable(L);  // t
		for (auto i : m_SpritePool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Animation:
		lua_newtable(L);  // t
		for (auto i : m_AnimationPool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::Music:
		lua_newtable(L);  // t
		break;
	case ResourceType::SoundEffect:
		lua_newtable(L);  // t
		break;
	case ResourceType::Particle:
		lua_newtable(L);  // t
		for (auto i : m_ParticlePool)
		{
			lua_pushstring(L, i.second->GetResName().c_str());  // t s
			lua_rawseti(L, -2, cnt++);  // t
		}
		break;
	case ResourceType::SpriteFont:
		lua_newtable(L);  // t
		break;
	case ResourceType::TrueType:
		lua_newtable(L);  // t
		break;
	default:
		lua_pushnil(L);
		break;
	}
}

bool ResourcePool::LoadTexture(const char* name, const std::wstring& path, bool mipmaps)LNOEXCEPT
{
	LASSERT(LAPP.GetRenderDev());

	if (m_TexturePool.find(name) != m_TexturePool.end())
	{
		LWARNING("LoadTexture: ����'%m'�Ѵ��ڣ���ͼʹ��'%s'���صĲ����ѱ�ȡ��", name, path.c_str());
		return true;
	}
	
	vector<char> tDataBuf;
	if (!m_pMgr->LoadFile(path.c_str(), tDataBuf))
	{
		LERROR("LoadTexture: �޷�װ���ļ�'%s'", path.c_str());
		return false;
	}
	
	fcyRefPointer<f2dTexture2D> tTexture;
	if (FCYFAILED(LAPP.GetRenderDev()->CreateTextureFromMemory((fcData)tDataBuf.data(), tDataBuf.size(), 0, 0, false, mipmaps, &tTexture)))
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
	return true;
}

bool ResourcePool::LoadAnimation(const char* name, const char* texname,
	double x, double y, double w, double h, int n, int m, int intv, double a, double b, bool rect)
{
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
	catch(const fcyException&)
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
	return true;
}

bool ResourcePool::LoadParticle(const char* name, const std::wstring& path, const char* img_name, double a, double b, bool rect)LNOEXCEPT
{
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

	std::vector<char> outBuf;
	if (!LRES.LoadFile(path.c_str(), outBuf))
		return false;
	if (outBuf.size() != sizeof(ResParticle::ParticleInfo))
	{
		LERROR("LoadParticle: ���Ӷ����ļ�'%s'��ʽ����ȷ", path.c_str());
		return false;
	}

	try
	{
		ResParticle::ParticleInfo tInfo;
		memcpy(&tInfo, outBuf.data(), sizeof(ResParticle::ParticleInfo));
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

	zlib_filefunc_def tZlibFileFunc;
	memset(&tZlibFileFunc, 0, sizeof(tZlibFileFunc));
	fill_win32_filefunc(&tZlibFileFunc);
	m_zipFile = unzOpen2(reinterpret_cast<const char*>(path), &tZlibFileFunc);
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

bool ResourcePack::LoadFile(const wchar_t* path, std::vector<char>& outBuf)LNOEXCEPT
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
					outBuf.resize(tFileInfo.uncompressed_size);
				}
				catch (const bad_alloc&)
				{
					unzCloseCurrentFile(m_zipFile);
					LERROR("ResourcePack: �޷������㹻�ڴ��ѹ��Դ��'%s'�е��ļ�'%s'", m_Path.c_str(), path);
					return false;
				}
				
				if (unzReadCurrentFile(m_zipFile, outBuf.data(), tFileInfo.uncompressed_size) < 0)
				{
					unzCloseCurrentFile(m_zipFile);
					LERROR("ResourcePack: ��ѹ��Դ��'%s'�е��ļ�'%s'ʧ�� (unzReadCurrentFileʧ��)", m_Path.c_str(), path);
					return false;
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
		for (auto i = m_ResPackList.begin(); i != m_ResPackList.end(); ++i)
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

LNOINLINE bool ResourceMgr::LoadFile(const wchar_t* path, std::vector<char>& outBuf)LNOEXCEPT
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
		outBuf.resize((size_t)pFile->GetLength());
	}
	catch (const bad_alloc&)
	{
		LERROR("ResourceMgr: �޷������㹻�ڴ�ӱ��ؼ����ļ�'%s'", path);
		return false;
	}
	catch (const fcyException& e)
	{
		LERROR("ResourceMgr: װ�ر����ļ�'%s'ʧ�� (�쳣��Ϣ'%m' Դ'%m')", path, e.GetDesc(), e.GetSrc());
		return false;
	}

	if (pFile->GetLength() > 0)
	{
		if (FCYFAILED(pFile->ReadBytes((fData)outBuf.data(), outBuf.size(), nullptr)))
		{
			LERROR("ResourceMgr: ��ȡ�����ļ�'%s'ʧ�� (fcyFileStream::ReadBytesʧ��)", path);
			return false;
		}
	}

	return true;
}

LNOINLINE bool ResourceMgr::LoadFile(const char* path, std::vector<char>& outBuf)LNOEXCEPT
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
	vector<char> tBuf;

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
			if (tBuf.size() > 0)
			{
				if (FCYFAILED(pFile->WriteBytes((fcData)tBuf.data(), tBuf.size(), nullptr)))
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
