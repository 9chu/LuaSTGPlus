/**
 * @file
 * @date 2022/9/18
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/DebugGUI/MixerWindow.hpp>

#include <imgui.h>
#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Math/Decibel.hpp>
#include <lstg/Core/Subsystem/AudioSystem.hpp>
#include <lstg/Core/Subsystem/Audio/AudioEngine.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::DebugGUI;

using namespace lstg::Subsystem::Audio;

static const DebugWindowFlags kWindowStyle = DebugWindowFlags::NoSavedSettings | DebugWindowFlags::HorizontalScrollbar;

MixerWindow::MixerWindow()
    : Window("MixerWindow", "Mixer", kWindowStyle)
{
}

void MixerWindow::OnPrepareWindow() noexcept
{
    ImGui::SetNextWindowPos(ImVec2(5.f, 60.f), ImGuiCond_FirstUseEver);
}

void MixerWindow::OnRender() noexcept
{
    auto& audioEngine = AppBase::GetInstance().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();

    // 绘制所有的 Bus 条
    for (size_t busId = 0; busId < AudioEngine::kBusChannelCount; ++busId)
    {
        ImGui::PushID(static_cast<int>(busId));
        ImGui::BeginGroup();
        ImGui::AlignTextToFramePadding();  // https://github.com/ocornut/imgui/issues/3254

        ImGui::Text("BUS %d", static_cast<int>(busId));

        // 音量与音量条
        {
            ImGui::BeginGroup();

            // 音量
            float peakVolumesRaw[ISoundDecoder::kChannels];
            float peakVolumesDisp[ISoundDecoder::kChannels];
            for (size_t channel = 0; channel < ISoundDecoder::kChannels; ++channel)
            {
                peakVolumesRaw[channel] = audioEngine.BusGetPeakVolume(busId, channel);

                auto db = Math::LinearToDecibelSafe(peakVolumesRaw[channel]);
                // 限制到 [-60, 15]
                db = std::max(-60.f, std::min(15.f, db));
                peakVolumesDisp[channel] = db + 60.f;
            }
            ImGui::PlotHistogram("##values", peakVolumesDisp, IM_ARRAYSIZE(peakVolumesDisp), 0, nullptr, 0.f, 75.f, ImVec2(25.f, 100.f));
            if (ImGui::IsItemActive() || ImGui::IsItemHovered())
            {
                // 覆盖图表的 Tooltip
                ImGui::SetTooltip("L: %.2f dB\nR: %.2f dB", Math::LinearToDecibelSafe(peakVolumesRaw[0]),
                    Math::LinearToDecibelSafe(peakVolumesRaw[1]));
            }
            ImGui::SameLine();

            // 音量条
            auto oldVolume = audioEngine.BusGetVolume(busId);
            auto newVolume = oldVolume;
            ImGui::VSliderFloat("##vol", ImVec2(20.f, 100.f), &newVolume, 0.f, 1.f, "");
            if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                ImGui::SetTooltip("%.2f dB (%.2f)", Math::LinearToDecibelSafe(newVolume), newVolume);
            if (newVolume != oldVolume)
                audioEngine.BusSetVolume(busId, newVolume);

            ImGui::EndGroup();
        }

        // 平衡条
        ImVec2 size = ImGui::GetItemRectSize();
        ImGui::SetNextItemWidth(size.x);
        auto oldPan = audioEngine.BusGetPan(busId);
        auto newPan = oldPan;
        ImGui::SliderFloat("##pan", &newPan, -1.f, 1.f);
        if (newPan != oldPan)
            audioEngine.BusSetPan(busId, newPan);

        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::PopID();
    }
}
