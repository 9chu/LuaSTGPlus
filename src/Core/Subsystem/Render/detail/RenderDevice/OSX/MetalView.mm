/**
 * @file
 * @date 2022/3/9
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "MetalView.hpp"

#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>
#include <lstg/Core/Subsystem/Render/RenderDevice.hpp>  // for RenderDeviceInitializeFailedException

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::RenderDevice::OSX;

MetalView::MetalView(/* NSWindow* */ void* nsWindow, bool highDpi)
{
    assert([(NSObject*)nsWindow isKindOfClass:[NSWindow class]]);

    // --- 创建 MetalView
    NSWindow* window = (NSWindow*)nsWindow;
    m_pMetalView = [window contentView];
    assert(m_pMetalView);  // SDL总是会创建一个 ContentView

    assert([NSThread isMainThread]);
    CALayer* layer = m_pMetalView.layer;
    if (nullptr != layer && [layer isKindOfClass:NSClassFromString(@"CAMetalLayer")])
    {
        m_pMetalLayer = (CAMetalLayer*)layer;
        [m_pMetalLayer retain];
    }
    else
    {
        [m_pMetalView setWantsLayer:YES];
        m_pMetalLayer = [CAMetalLayer layer];
        if (!m_pMetalLayer)
            LSTG_THROW(RenderDeviceInitializeFailedException, "CAMetalLayer returns nil");
        [m_pMetalView setLayer:m_pMetalLayer];
    }

    // 计算 HighDPI 大小
    NSSize size = m_pMetalView.bounds.size;
    NSSize backingSize = size;
    if (highDpi)
        backingSize = [m_pMetalView convertSizeToBacking:size];
    m_pMetalLayer.contentsScale = backingSize.height / size.height;
}

MetalView::~MetalView()
{
    [m_pMetalLayer release];
}
