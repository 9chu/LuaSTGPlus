/**
 * @file
 * @date 2022/9/18
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/DebugGUI/MixerWindow.hpp>

#include <imgui.h>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Math/Decibel.hpp>
#include <lstg/Core/Subsystem/AudioSystem.hpp>
#include <lstg/Core/Subsystem/Audio/AudioEngine.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::DebugGUI;

using namespace lstg::Subsystem::Audio;

LSTG_DEF_LOG_CATEGORY(MixerWindow);

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
    auto& audioSystem = *AppBase::GetInstance().GetSubsystem<Subsystem::AudioSystem>();
    auto& audioEngine = audioSystem.GetEngine();

    // 绘制所有的 Bus 条
    char tmpBuffer[64];
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
                ImGui::SetTooltip("VOL: %.2f dB (%.2f)", Math::LinearToDecibelSafe(newVolume), newVolume);
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
        if (ImGui::IsItemActive() || ImGui::IsItemHovered())
            ImGui::SetTooltip("PAN: %.2f", newPan);
        if (newPan != oldPan)
            audioEngine.BusSetPan(busId, newPan);

        // 静音
        bool muted = audioEngine.BusIsMuted(busId);
        if (muted)
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
        if (ImGui::Button("MUTE", ImVec2(size.x, 20.f)))
            audioEngine.BusSetMuted(busId, !muted);
        if (muted)
            ImGui::PopStyleColor(1);

        // 效果
        ImGui::Text("FX");
        ImGui::BeginChild("ChildFX", ImVec2(size.x, 50.f), false, ImGuiWindowFlags_HorizontalScrollbar);
        {
            auto buttonWidth = ImGui::GetContentRegionAvail().x;

            auto pluginCount = audioEngine.BusGetPluginCount(busId);
            for (size_t i = 0; i < pluginCount; ++i)
            {
                auto plugin = audioEngine.BusGetPlugin(busId, i);

                ImGui::PushID(static_cast<int>(i));

                if (ImGui::BeginPopupContextItem("FXMenu"))
                {
                    size_t params = plugin->GetParameterCount();
                    if (ImGui::BeginTable("##table", static_cast<int>(params)))
                    {
                        for (size_t j = 0; j < params; ++j)
                        {
                            ImGui::PushID(static_cast<int>(j));

                            ImGui::TableNextRow();

                            const auto& info = plugin->GetParameterInfo(j);
                            if (info.Desc.index() == 0)
                            {
                                const auto& sliderInfo = std::get<DspPluginSliderParameter>(info.Desc);

                                auto oldSliderValue = plugin->GetSliderParameter(info.Id);
                                assert(oldSliderValue);

                                if (oldSliderValue)
                                {
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::Text("%s", info.DisplayName.c_str());

                                    auto newSliderValue = *oldSliderValue;
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::SetNextItemWidth(100.f);
                                    ImGui::SliderFloat("##val", &newSliderValue, sliderInfo.MinValue, sliderInfo.MaxValue);
                                    if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                                        ImGui::SetTooltip("%s: %.2f", info.DisplayName.c_str(), newSliderValue);
                                    if (newSliderValue != *oldSliderValue)
                                        plugin->SetSliderParameter(info.Id, newSliderValue);
                                }
                            }
                            else if (info.Desc.index() == 1)
                            {
                                const auto& enumInfo = std::get<DspPluginEnumParameter>(info.Desc);
                                if (MakeKeysFromMapOptions(enumInfo.Options))
                                {
                                    auto oldEnumValue = plugin->GetEnumParameter(info.Id);
                                    assert(oldEnumValue);

                                    if (oldEnumValue)
                                    {
                                        int selected = -1;
                                        for (const auto& p : enumInfo.Options)
                                        {
                                            if (p.second == *oldEnumValue)
                                            {
                                                for (size_t k = 0; k < m_stTmpOptions.size(); ++k)
                                                {
                                                    if (m_stTmpOptions[k] == p.first)
                                                    {
                                                        selected = static_cast<int>(k);
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                        assert(selected != -1);

                                        ImGui::TableSetColumnIndex(0);
                                        ImGui::Text("%s", info.DisplayName.c_str());

                                        ImGui::TableSetColumnIndex(1);
                                        ImGui::SetNextItemWidth(100.f);
                                        if (ImGui::Combo("##combo", &selected, m_stTmpOptions.data(), static_cast<int>(m_stTmpOptions.size())))
                                        {
                                            auto it = enumInfo.Options.find(m_stTmpOptions[selected]);
                                            assert(it != enumInfo.Options.end());
                                            plugin->SetEnumParameter(info.Id, it->second);
                                        }
                                    }
                                }
                            }

                            ImGui::PopID();
                        }
                        ImGui::EndTable();
                    }

                    if (ImGui::Selectable("Delete"))
                        audioEngine.BusRemovePlugin(busId, i);

                    ImGui::EndPopup();
                }

                if (ImGui::Button(plugin->GetName(), ImVec2(buttonWidth, 20.f)))
                    ImGui::OpenPopup("FXMenu");

                ImGui::PopID();
            }

            if (ImGui::BeginPopupContextItem("AddFXMenu"))
            {
                audioSystem.VisitDspPlugins([&](const string& name) {
                    if (ImGui::Selectable(name.c_str()))
                    {
                        auto ret = audioSystem.CreateDspPlugin(name);
                        if (!ret)
                        {
                            LSTG_LOG_ERROR_CAT(MixerWindow, "Create dsp plugin {} error: {}", name, ret.GetError());
                        }
                        else
                        {
                            auto ret2 = audioEngine.BusInsertPlugin(busId, std::move(*ret));
                            if (!ret2)
                                LSTG_LOG_ERROR_CAT(MixerWindow, "Insert dsp plugin {} error: {}", name, ret2.GetError());
                        }
                    }
                });

                ImGui::EndPopup();
            }

            // 添加按钮
            if (ImGui::Button("+", ImVec2(buttonWidth, 20.f)))
                ImGui::OpenPopup("AddFXMenu");
        }
        ImGui::EndChild();

        // 发送
        ImGui::Text("SEND");
        ImGui::BeginChild("ChildSend", ImVec2(size.x, 50.f), false, ImGuiWindowFlags_HorizontalScrollbar);
        {
            auto buttonWidth = ImGui::GetContentRegionAvail().x;

            BusSendStages stages[3] = { BusSendStages::BeforeVolume, BusSendStages::AfterVolume, BusSendStages::AfterPan };
            const char* stageNames[3] = { "BEFORE VOL", "AFTER VOL", "AFTER PAN" };
            for (size_t i = 0; i < extent_v<decltype(stages)>; ++i)
            {
                auto sendTargetCount = audioEngine.BusGetSendTargetCount(busId, stages[i]);
                for (size_t j = 0; j < sendTargetCount; ++j)
                {
                    auto sendTarget = audioEngine.BusGetSendTarget(busId, stages[i], j);
                    if (!sendTarget)
                        continue;

                    ImGui::PushID(static_cast<int>(j * 10 + i));

                    float sendVolume = sendTarget->Volume;
                    if (ImGui::BeginPopupContextItem("SendTargetMenu"))
                    {
                        if (ImGui::Selectable("Delete"))
                            audioEngine.BusRemoveSendTarget(busId, stages[i], j);
                        ImGui::DragFloat("##Value", &sendVolume, 0.001f, 0.0f, 1.0f);
                        if (sendVolume != sendTarget->Volume)
                        {
                            if (audioEngine.BusSetSendTargetVolume(busId, stages[i], j, sendVolume))
                                sendTarget->Volume = sendVolume;
                        }
                        ImGui::EndPopup();
                    }

                    // Bus 按钮
                    ::sprintf(tmpBuffer, "BUS %d", static_cast<int>(sendTarget->Target));
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(static_cast<float>(i + 1) / 7.0f, 0.6f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(static_cast<float>(i + 1) / 7.0f, 0.7f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(static_cast<float>(i + 1) / 7.0f, 0.8f, 0.8f));
                    if (ImGui::Button(tmpBuffer, ImVec2(buttonWidth, 20.f)))
                        ImGui::OpenPopup("SendTargetMenu");
                    if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s\n%.2f dB (%.2f)", stageNames[i], Math::LinearToDecibelSafe(sendTarget->Volume),
                            sendTarget->Volume);
                    }
                    ImGui::PopStyleColor(3);

                    ImGui::PopID();
                }
            }

            if (ImGui::BeginPopupContextItem("AddSendTargetMenu"))
            {
                for (size_t i = 0; i < extent_v<decltype(stages)>; ++i)
                {
                    if (ImGui::BeginMenu(stageNames[i]))
                    {
                        for (BusId j = 0; j < AudioEngine::kBusChannelCount; ++j)
                        {
                            sprintf(tmpBuffer, "BUS %d", static_cast<int>(j));
                            if (ImGui::MenuItem(tmpBuffer))
                            {
                                BusSend send;
                                send.Volume = 1.f;
                                send.Target = j;
                                audioEngine.BusAddSendTarget(busId, stages[i], send);
                            }
                        }

                        ImGui::EndMenu();
                    }
                }
                ImGui::EndPopup();
            }

            // 添加按钮
            if (ImGui::Button("+", ImVec2(buttonWidth, 20.f)))
                ImGui::OpenPopup("AddSendTargetMenu");
        }
        ImGui::EndChild();

        // 输出
        auto target = audioEngine.BusGetOutputTarget(busId);
        auto newTarget = target;
        if (ImGui::BeginPopupContextItem("BusTargetSelector"))
        {
            if (ImGui::Selectable("MASTER"))
                newTarget = static_cast<BusId>(-1);
            for (BusId i = 0; i < AudioEngine::kBusChannelCount; ++i)
            {
                sprintf(tmpBuffer, "BUS %d", static_cast<int>(i));
                if (ImGui::Selectable(tmpBuffer))
                    newTarget = i;
            }
            ImGui::EndPopup();
        }
        if (target == static_cast<BusId>(-1))
            sprintf(tmpBuffer, "MASTER");
        else
            sprintf(tmpBuffer, "BUS %d", static_cast<int>(target));
        if (ImGui::Button(tmpBuffer, ImVec2(size.x, 20.f)))
            ImGui::OpenPopup("BusTargetSelector");
        if (ImGui::IsItemActive() || ImGui::IsItemHovered())
            ImGui::SetTooltip("OUTPUT");
        if (target != newTarget)
            audioEngine.BusSetOutputTarget(busId, newTarget);

        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::PopID();
    }
}

Result<void> MixerWindow::MakeKeysFromMapOptions(const std::map<std::string, int32_t>& options) noexcept
{
    try
    {
        m_stTmpOptions.clear();
        for (const auto& p : options)
            m_stTmpOptions.push_back(p.first.c_str());
        return {};
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}
