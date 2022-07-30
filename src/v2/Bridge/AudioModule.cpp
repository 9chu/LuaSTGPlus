/**
 * @file
 * @author 9chu
 * @date 2022/4/18
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/AudioModule.hpp>

#include <lstg/Core/Logging.hpp>
#include "detail/Helper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

LSTG_DEF_LOG_CATEGORY(AudioModule);

void AudioModule::PlaySound(const char* name, double vol, std::optional<double> pan /* =0.0 */)
{
    // TODO
//    const char* s = luaL_checkstring(L, 1);
//    ResSound* p = LRES.FindSound(s);
//    if (!p)
//        return luaL_error(L, "sound '%s' not found.", s);
//    p->Play((float)luaL_checknumber(L, 2) * LRES.GetGlobalSoundEffectVolume(), (float)luaL_optnumber(L, 3, 0.));
//    return 0;
}

void AudioModule::StopSound(const char* name)
{
    // TODO
//    const char* s = luaL_checkstring(L, 1);
//    ResSound* p = LRES.FindSound(s);
//    if (!p)
//        return luaL_error(L, "sound '%s' not found.", s);
//    p->Stop();
//    return 0;
}

void AudioModule::PauseSound(const char* name)
{
    // TODO
//    const char* s = luaL_checkstring(L, 1);
//    ResSound* p = LRES.FindSound(s);
//    if (!p)
//        return luaL_error(L, "sound '%s' not found.", s);
//    p->Pause();
//    return 0;
}

void AudioModule::ResumeSound(const char* name)
{
    // TODO
//    const char* s = luaL_checkstring(L, 1);
//    ResSound* p = LRES.FindSound(s);
//    if (!p)
//        return luaL_error(L, "sound '%s' not found.", s);
//    p->Resume();
//    return 0;
}

const char* AudioModule::GetSoundState(const char* name)
{
    // TODO
//    const char* s = luaL_checkstring(L, 1);
//    ResSound* p = LRES.FindSound(s);
//    if (!p)
//        return luaL_error(L, "sound '%s' not found.", s);
//    if (p->IsPlaying())
//        lua_pushstring(L, "playing");
//    else if (p->IsStopped())
//        lua_pushstring(L, "stopped");
//    else
//        lua_pushstring(L, "paused");
//    return 1;
    return "stopped";
}

void AudioModule::PlayMusic(const char* name, std::optional<double> vol /* =1.0 */, std::optional<double> position /* =0 */)
{
    // TODO
//    const char* s = luaL_checkstring(L, 1);
//    ResMusic* p = LRES.FindMusic(s);
//    if (!p)
//        return luaL_error(L, "music '%s' not found.", s);
//    p->Play((float)luaL_optnumber(L, 2, 1.) * LRES.GetGlobalMusicVolume(), luaL_optnumber(L, 3, 0.));
//    return 0;
}

void AudioModule::StopMusic(const char* name)
{
    // TODO
//    const char* s = luaL_checkstring(L, 1);
//    ResMusic* p = LRES.FindMusic(s);
//    if (!p)
//        return luaL_error(L, "music '%s' not found.", s);
//    p->Stop();
//    return 0;
}

void AudioModule::PauseMusic(const char* name)
{
    // TODO
//    const char* s = luaL_checkstring(L, 1);
//    ResMusic* p = LRES.FindMusic(s);
//    if (!p)
//        return luaL_error(L, "music '%s' not found.", s);
//    p->Pause();
//    return 0;
}

void AudioModule::ResumeMusic(const char* name)
{
    // TODO
//    const char* s = luaL_checkstring(L, 1);
//    ResMusic* p = LRES.FindMusic(s);
//    if (!p)
//        return luaL_error(L, "music '%s' not found.", s);
//    p->Resume();
//    return 0;
}

const char* AudioModule::GetMusicState(const char* name)
{
    // TODO
//    const char* s = luaL_checkstring(L, 1);
//    ResMusic* p = LRES.FindMusic(s);
//    if (!p)
//        return luaL_error(L, "music '%s' not found.", s);
//    if (p->IsPlaying())
//        lua_pushstring(L, "playing");
//    else if (p->IsStopped())
//        lua_pushstring(L, "stopped");
//    else
//        lua_pushstring(L, "paused");
//    return 1;
    return "stopped";
}

void AudioModule::UpdateSound()
{
    LSTG_LOG_DEPRECATED(AudioModule, UpdateSound);
}

void AudioModule::SetSEVolume(double vol)
{
    // TODO
//    float x = static_cast<float>(luaL_checknumber(L, 1));
//    LRES.SetGlobalSoundEffectVolume(max(min(x, 1.f), 0.f));
//    return 0;
}

void AudioModule::SetBGMVolume(LuaStack& stack)
{
    // TODO
//    if (lua_gettop(L) == 1)
//    {
//        float x = static_cast<float>(luaL_checknumber(L, 1));
//        LRES.SetGlobalMusicVolume(max(min(x, 1.f), 0.f));
//    }
//    else
//    {
//        const char* s = luaL_checkstring(L, 1);
//        float x = static_cast<float>(luaL_checknumber(L, 2));
//        ResMusic* p = LRES.FindMusic(s);
//        if (!p)
//            return luaL_error(L, "music '%s' not found.", s);
//        p->SetVolume(x * LRES.GetGlobalMusicVolume());
//    }
//    return 0;
}
