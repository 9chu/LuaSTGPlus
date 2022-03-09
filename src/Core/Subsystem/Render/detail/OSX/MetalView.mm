/**
 * @file
 * @date 2022/3/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "MetalView.hpp"

#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>
#include <lstg/Core/Subsystem/Render/RenderDevice.hpp>  // for RenderDeviceInitializeFailedException

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::OSX;

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
}

MetalView::~MetalView()
{
    [m_pMetalLayer release];
}
