# 内建方法

所有内建方法均位于全局`lstg`模块中。

## 数学方法

下述数学函数均以角度制为基准，含义同C语言库函数。

- sin(ang)
- cos(ang)
- asin(v)
- acos(v)
- tan(ang)
- atan(v)
- atan2(y,x)

## 系统方法

### SetWindowed

设置是否窗口化。

- 签名：`SetWindowed(windowed: boolean)`
- 参数
    - windowed：设置是否窗口化（true 表示窗口化，false 表示非窗口化）

::: tip
LuaSTGPlus 默认以窗口化模式启动。全屏模式下总是以无边框窗口模式运行。
:::

### SetFPS

设置目标 FPS。

- 签名：`SetFPS(fps: number)`
- 参数
    - fps：目标 FPS

::: tip
LuaSTGPlus 默认锁 60 FPS。
:::

### GetFPS

获取实时 FPS。

- 签名：`GetFPS(): number`
- 返回值：FPS 值

### SetVsync

设置是否垂直同步。

- 签名：`SetVsync(vsync: boolean)`
- 参数
    - vsync：是否垂直同步

::: tip
LuaSTGPlus 默认不开启垂直同步。
:::

### SetResolution

设置分辨率。

从`v2`开始，`SetResolution`将用于设置期望的分辨率。当平台支持改变窗口大小时，窗口大小会设置成该分辨率大小；当平台不支持改变窗口大小或窗口位于全屏模式且分辨率不等于期望大小时，LuaSTGPlus 会自动调整渲染视口来适配期望的长宽比。

- 签名：`SetResolution(width: number, height: number)`
- 参数
    - width：宽度
    - height：高度

::: tip
LuaSTGPlus 默认分辨率为 640x480。
:::

### ChangeVideoMode

改变视频选项。

从`v2`开始，该方法等价于`SetResolution`、`SetWindowed`和`SetVsync`的组合。

- 签名：`ChangeVideoMode(width: number, height: number, windowed: boolean, vsync: boolean): boolean`
- 参数
    - width：宽度
    - height：高度
    - windowed：是否窗口模式
    - vsync：是否垂直同步
- 返回值：是否成功

### SetSplash

设置是否显示光标。

- 签名：`SetSplash(value: boolean)`
- 参数
    - value：是否显示

### SetTitle

设置窗口标题。

- 签名：`SetTitle(value: string)`
- 参数
    - value：窗口标题

::: tip
默认窗口标题为“LuaSTGPlus”。
:::

### SystemLog

写出日志。

- 签名：`SystemLog(what: string)`
- 参数
    - what：需要写出的内容

### Print

将若干值写出日志。

- 签名：`Print(...)`
- 参数
    - ...：可变参，将会施加 tostring 操作后写出日志

### LoadPack

加载指定位置的 Zip 资源包，可选填密码。

- 签名：`LoadPack(path: string, password?: string)`
- 参数
    - path：文件路径，请参考[资产加载规则](../../guide/Subsystem/AssetSystem.md#资产加载规则)
    - password：可选密码

::: tip
后加载的资源包有较高的查找优先级。这意味着可以通过该机制加载资源包来覆盖基础资源包中的文件，用于打补丁等场景。

详见[文件子系统](../../guide/Subsystem/VfsSystem.md)章节。
:::

### UnloadPack

卸载指定位置的资源包，要求路径名必须一致。

- 签名：`UnloadPack(path: string)`
- 参数
    - path：文件路径

### ExtractRes <Badge type="warning" vertical="middle" text="deprecated" />

将资源包中的数据解压到本地。

- 签名：`ExtractRes(path: string, target: string)`
- 参数
    - path：资源包路径
    - target：解压目的地

::: warning
该方法已废弃，不再有任何效果。
:::

### DoFile

执行指定路径的脚本。

方法会采取相对调用者路径的搜索规则加载脚本，请参考[资产加载规则](../../guide/Subsystem/AssetSystem.md#资产加载规则)。

- 签名：`DoFile(path: string)`
- 参数
    - path：脚本路径

::: tip
已执行过的脚本会再次执行。

推荐使用`import`/`require`机制代替该方法。
:::

### ShowSplashWindow <Badge type="warning" vertical="middle" text="deprecated" />

显示载入 LOGO 窗口。

- 签名：`ShowSplashWindow(path?: string)`
- 参数
    - path：LOGO 图片路径，当不设置时，使用内置图片显示载入窗口。

::: warning
该方法已废弃，不再有任何效果。
:::

## 资产管理方法

详见[资产子系统](../../guide/Subsystem/AssetSystem.md)。

### SetResourceStatus

设置当前活动的资产池。

活动资产池将会影响资产被加载到哪里。

- 签名：`SetResourceStatus(pool: string)`
- 参数
    - pool：资产池名称，可选`global`、`stage`、`none`。当设置`global`时，资产会被加载到**全局资产池**；当设置`stage`时，资产会被加载到**关卡资产池**；当设置`none`时，将会禁用加载过程

### RemoveResource

删除所有资产或某一资产。

若仅指定资产池，将删除该资产池中所有资产。

否则，删除指定类型的某一资产。

若资产仍在使用，则需等到相关使用方释放资产才会最终删除资产对象。

- 签名：`RemoveResource(pool: string, type?: number, name?: string)`
- 参数
    - pool：资产池名称，可选`global`、`stage`、`none`（设置为`none`不会有任何效果）
    - type：资产类型，详见[资产类型](../../guide/Subsystem/AssetSystem.md#资产类型)
    - name：资产名，当指定`type`时，必须填写该参数 

### CheckRes

检查某一资产位于哪个资产池。

从 LuaSTGPlus v2 起，检查会先经过关卡资产池，如果不存在，再在全局资产池中查找。

- 签名：`CheckRes(type: number, name: string): string?`
- 参数
    - type：资产类型，详见[资产类型](../../guide/Subsystem/AssetSystem.md#资产类型)
    - name：资产名
- 返回值
    - 资产池名称，取值`global`、`stage`。若资产不存在，返回`nil`

### EnumRes

枚举资产池中某种类型的资产。

依次返回**全局资产池**和**关卡资产池**中所有该类型的资产的名称。

- 签名：`EnumRes(type: number): [table, table]`
- 参数
    - type：资产类型，详见[资产类型](../../guide/Subsystem/AssetSystem.md#资产类型)
- 返回值
    - 全局资产池和关卡资产池中的资产名称

### GetTextureSize

获取纹理资产的大小。

注意该值经过 PPU 缩放，非实际大小。

- 签名：`GetTextureSize(name: string): [number, number]`
- 参数
    - name：纹理名称
- 返回值
    - 纹理的宽度和高度

### LoadTexture

加载纹理资产。

- 签名：`LoadTexture(name: string, path: string, mipmap?: boolean)`
- 参数
    - name：资产名
    - path：纹理路径，请参考[资产加载规则](../../guide/Subsystem/AssetSystem.md#资产加载规则)
    - mipmap：是否创建纹理链，默认取值`true`

### LoadImage

加载精灵资产。

- 签名：`LoadImage(name: string, textureName: string, x: number, y: number, w: number, h: number, a?: number, b?: number, rect?: boolean)`
- 参数
    - name：资产名
    - textureName：引用的纹理资产名
    - x：指定在纹理中（经过 PPU 缩放的）精灵左上角的横坐标（正方向向右）
    - y：指定在纹理中（经过 PPU 缩放的）精灵左上角的纵坐标（正方向向下）
    - w：指定在纹理中（经过 PPU 缩放的）精灵的宽度
    - h：指定在纹理中（经过 PPU 缩放的）精灵的高度
    - a：碰撞盒的横向半长
    - b：碰撞盒的纵向半长
    - rect：是否为矩形碰撞盒

### SetImageState

设置精灵混合状态。

- 签名：`SetImageState(name: string, blend: string, vertexColor1?: LSTGColor, vertexColor2?: LSTGColor, vertexColor3?: LSTGColor, vertexColor4?: LSTGColor)`
- 参数
    - name：资产名
    - blend：混合模式，详见[混合选项](../../guide/Subsystem/RenderSystem.md#混合选项)
    - vertexColor1：精灵左上角顶点色，当仅设置`vertexColor1`时，将会用于设置所有顶点的颜色
    - vertexColor2：精灵右上角顶点色
    - vertexColor3：精灵右下角顶点色
    - vertexColor4：精灵左下角顶点色

### SetImageCenter

设置精灵的锚点。

- 签名：`SetImageCenter(name: string, x: number, y: number)`
- 参数
    - name：资产名
    - x：相对于精灵左上角的横坐标
    - y：相对于精灵左上角的纵坐标

### SetImageScale <Badge type="warning" vertical="middle" text="deprecated" />

设置全局图像缩放系数。

该方法将在渲染精灵等对象时，在`scale`参数上增加一个系数。

由于该方法可能会造成混乱，我们决定在新版中移除该方法。

- 签名：`SetImageScale(factor: number)`
- 参数
    - factor：缩放系数

::: warning
该方法已废弃，不再有任何效果。
:::

### LoadAnimation

加载动画序列。

- 签名：`LoadAnimation(name: string, textureName: string, x: number, y: number. w: number, h: number, n: number, m: number, interval: number, a?: number, b?: number, rect?: boolean)`
- 参数
    - name：资产名
    - textureName：依赖的纹理资产名
    - x：同[LoadImage](./BuiltinMethods.md#loadimage)
    - y：同[LoadImage](./BuiltinMethods.md#loadimage)
    - w：指定一帧的宽度
    - h：指定一帧的高度
    - n：指定纵向分割数，列优先排列动画帧
    - m：指定横向分割数
    - interval：播放间隔（帧）
    - a：同[LoadImage](./BuiltinMethods.md#loadimage)
    - b：同[LoadImage](./BuiltinMethods.md#loadimage)
    - rect：同[LoadImage](./BuiltinMethods.md#loadimage)

### SetAnimationState

同[SetImageState](./BuiltinMethods.md#setimagestate)。

- 签名：`SetAnimationState(name: string, blend: string, vertexColor1?: LSTGColor, vertexColor2?: LSTGColor, vertexColor3?: LSTGColor, vertexColor4?: LSTGColor)`
- 参数
    - name：资产名
    - blend：混合模式，详见[混合选项](../../guide/Subsystem/RenderSystem.md#混合选项)
    - vertexColor1：动画序列左上角顶点色，当仅设置`vertexColor1`时，将会用于设置所有顶点的颜色
    - vertexColor2：动画序列右上角顶点色
    - vertexColor3：动画序列右下角顶点色
    - vertexColor4：动画序列左下角顶点色

### SetAnimationCenter

同[SetImageCenter](./BuiltinMethods.md#setimagecenter)。

- 签名：`SetImageCenter(name: string, x: number, y: number)`
- 参数
    - name：资产名
    - x：相对于动画序列左上角的横坐标
    - y：相对于动画序列左上角的纵坐标

### LoadPS

加载粒子效果。

- 签名：`LoadPS(name: string, path: string, imgName: string, a?: number, b?: number, rect?: boolean)`
- 参数
    - name：资产名
    - path：定义文件路径，请参考[资产加载规则](../../guide/Subsystem/AssetSystem.md#资产加载规则)
    - imgName：绑定的粒子精灵
    - a：同[LoadImage](./BuiltinMethods.md#loadimage)
    - b：同[LoadImage](./BuiltinMethods.md#loadimage)
    - rect：同[LoadImage](./BuiltinMethods.md#loadimage)

### LoadFont

加载纹理化字体。

- 签名：`LoadFont(name: string, path: string, mipmap?: boolean)`
- 参数
    - name：资产名
    - path：定义文件路径，请参考[资产加载规则](../../guide/Subsystem/AssetSystem.md#资产加载规则)
    - mipmap：是否创建 mipmap，默认为`true`

### SetFontState

设置纹理化字体的混合模式。

- 签名：`SetFontState(name: string, blend: string, color?: LSTGColor)`
- 参数
    - name：资产名
    - blend：混合模式，详见[混合选项](../../guide/Subsystem/RenderSystem.md#混合选项)
    - color：纹理化字体的顶点色

### SetFontState2 <Badge type="warning" vertical="middle" text="deprecated" />

设置 HGE 纹理化字体的参数。

LuaSTGPlus 不支持该方法。

::: warning
该方法已废弃，不再有任何效果。
:::

### LoadTTF

加载 TTF 字体。

- 签名：`LoadTTF(name: string, path: string, width: number, height?: number)`
- 参数
    - name：资产名
    - path：字体路径，请参考[资产加载规则](../../guide/Subsystem/AssetSystem.md#资产加载规则)
    - width：字体宽度
    - height：字体高度（废弃）

### RegTTF <Badge type="warning" vertical="middle" text="deprecated" />

向系统注册 TTF 字体。

LuaSTGPlus 不支持该方法。

::: warning
该方法已废弃，不再有任何效果。
:::

### LoadSound

加载[音效](../../guide/Subsystem/AssetSystem.md#音效)。

- 签名：`LoadSound(name: string, path: string)`
- 参数
    - name：资产名
    - path：音效路径，请参考[资产加载规则](../../guide/Subsystem/AssetSystem.md#资产加载规则)
    
### LoadMusic

加载[背景音乐](../../guide/Subsystem/AssetSystem.md#背景音乐)。

- 签名：`LoadMusic(name: string, path: string, end: number, loop: number)`
- 参数
    - name：资产名
    - path：背景音乐路径，请参考[资产加载规则](../../guide/Subsystem/AssetSystem.md#资产加载规则)
    - end：循环节结尾相对乐曲开始的时间（秒）
    - loop：循环节长度（秒）

### LoadFX

装载 Shader 效果文件。

- 签名：`LoadFX(name: string, path: string)`
- 参数
    - name：资产名
    - path：文件路径，请参考[资产加载规则](../../guide/Subsystem/AssetSystem.md#资产加载规则)

### CreateRenderTarget

创建 RenderTarget。

- 签名：`CreateRenderTarget(name: string)`
- 参数
    - name：资产名

### IsRenderTarget

检查纹理是否为 RenderTarget。

- 签名：`IsRenderTarget(name: string): boolean`
- 参数
    - name：纹理资产名
- 返回值：纹理资产是否为 RT

## 渲染方法

渲染方法均需要在`RenderFunc`中调用。

详见[渲染子系统](../../guide/Subsystem/RenderSystem.md)。

### BeginScene <Badge type="warning" vertical="middle" text="deprecated" />

通知渲染开始。

::: warning
该方法已废弃，不再有任何效果。
:::

### EndScene <Badge type="warning" vertical="middle" text="deprecated" />

通知渲染结束。

::: warning
该方法已废弃，不再有任何效果。
:::

### RenderClear

使用指定颜色清空屏幕。

同时会清除深度缓冲区。

- 签名：`RenderClear(color: LSTGColor)`
- 参数
    - color：清空用颜色

### SetViewport

设置视口，将影响裁剪和渲染。

- 签名：`SetViewport(left: number, right: number, bottom: number, top: number)`
- 参数
    - left：左侧位置（相对于设计分辨率）
    - right：右侧位置（相对于设计分辨率）
    - bottom：底部位置（相对于设计分辨率）
    - top：顶部位置（相对于设计分辨率）

### SetOrtho

设置正投影矩阵。最终 Z 轴被限制在[0, 1]区间内。

- 签名：`SetOrtho(left: number, right: number, bottom: number, top: number)`
- 参数
    - left：表示X轴最小值
    - right：表示X轴最大值
    - bottom：表示Y轴最小值
    - top：表示Y轴最大值

### SetPerspective

设置透视投影矩阵和观察矩阵。

- 签名：`SetPerspective(eyeX: number, eyeY: number, eyeZ: number, atX: number, atY: number, atZ: number, upX: number, upY: number, upZ: number, fovy: number, aspect: number, zn: number, zf: number)`
- 参数
    - eyeX：观察者 X 坐标
    - eyeY：观察者 Y 坐标
    - eyeZ：观察者 Z 坐标
    - atX：观察目标 X 坐标
    - atY：观察目标 Y 坐标
    - atZ：观察目标 Z 坐标
    - upX：上方向向量 X 坐标
    - upY：上方向向量 Y 坐标
    - upZ：上方向向量 Z 坐标
    - fovy：视角范围（弧度制）
    - aspect：宽高比
    - zn：Z 轴近裁剪面距离
    - zf：Z 轴远裁剪面距离

### SetFog

设置雾效果。

当参数为空，将关闭雾效果。

当参数1为`-1`，采用`Exp`公式，此时`far`设置为系数，`color`设置为颜色。

当参数1为`-2`，采用`Exp2`公式，此时`far`设置为系数，`color`设置为颜色。

否则，采用`Linear`公式，此时起止设置为`near`和`far`，`color`设置为颜色。

- 签名：`SetFog(near?: number, far?: number, color?: LSTGColor)`
- 参数
    - near：参数1，参见上述说明
    - far：参数2，参见上述说明
    - color：雾颜色，默认为`0x00FFFFFF`

### Render

渲染精灵。

- 签名：`Render(imageName: string, x: number, y: number, rot?: number, hscale?: number, vscale?: number, z?: number)`
- 参数
    - imageName：精灵资产名
    - x：中心点 X 坐标
    - y：中心点 Y 坐标
    - rot：旋转（弧度制），默认为0
    - hscale：X 轴缩放量，默认为1
    - vscale：Y 轴缩放量，默认为1，若指定了 hscale 但是没有指定 vscale，则 vscale = hscale
    - z：Z 坐标，默认为0.5

### RenderRect

在一个矩形范围内渲染图像（z = 0.5）。

- 签名：`RenderRect(imageName: string, left: number, right: number, bottom: number, top: number)`
- 参数
    - imageName：精灵资产名
    - left：左侧坐标值
    - right：右侧坐标值
    - bottom：底边坐标值
    - top：顶边坐标值

### Render4V

给出四个顶点渲染图像（z = 0.5）。

- 签名：`Render4V(imageName: string, x1: number, y1: number, z1:number, x2: number, y2: number, z2: number, x3: number, y3: number, z3: number, x4: number, y4: number, z4: number)`
- 参数
    - imageName：精灵资产名
    - x1：左上角坐标 X 值
    - y1：左上角坐标 Y 值
    - z1：左上角坐标 Z 值
    - x2：右上角坐标 X 值
    - y2：右上角坐标 Y 值
    - z2：右上角坐标 Z 值
    - x3：右下角坐标 X 值
    - y3：右下角坐标 Y 值
    - z3：右下角坐标 Z 值
    - x4：左下角坐标 X 值
    - y4：左下角坐标 Y 值
    - z4：左下角坐标 Z 值

### RenderTexture

直接渲染纹理。

顶点需要满足下述定义：

```lua
{
    [1] = X坐标,
    [2] = Y坐标,
    [3] = Z坐标,
    [4] = U坐标（受PPU影响）,
    [5] = V坐标（受PPU影响）,
    [6] = 顶点颜色,
}
```

- 签名：`RenderTexture(textureName: string, blend: string, vertex1: table, vertex2: table, vertex3: table, vertex4: table)`
- 参数
    - textureName：纹理资产名
    - blend：混合模式，详见[混合选项](../../guide/Subsystem/RenderSystem.md#混合选项)
    - vertex1：左上角顶点
    - vertex2：右上角顶点
    - vertex3：右下角顶点
    - vertex4：左下角顶点

### RenderText

使用纹理化字体渲染一段文字。

默认字体采取 0.5 倍行距。

对齐方式可取：

| 对齐方式 | 值 | 对齐方式 | 值 | 对齐方式 | 值 |
| ------- | -- | ------- | -- | ------- | -- |
| 左上     | 0  | 中上     | 1  | 右上     | 2  |
| 左中     | 4  | 中中     | 5  | 右中     | 6  |
| 左下     | 8  | 中下     | 9  | 右下     | 10 |

- 签名：`RenderText(name: string, text: string, x: number, y: number, scale?: number, align?: number)`
- 参数
    - name：资产名
    - text：文字
    - x：左侧坐标值
    - y：顶边坐标值
    - scale：缩放，默认取 1
    - align：对齐方式，默认取 5

### RenderTTF

渲染 TTF 字体。

格式可取：

| 说明     | 值 | 说明     | 值 |
| -------- | -- | -------- | -- |
| 顶对齐   | 0 | 左对齐   | 0 |
| 垂直居中 | 4 | 居中     | 1 |
| 底对齐   | 8 | 右对齐   | 2 |
| 断字     | 16 |

- 签名：`RenderTTF(name: string, text: string, left: number, right: number, bottom: number, top: number, fmt: number, blend: LSTGColor, scale?: number)`
- 参数
    - name：资产名
    - text：文字
    - left：渲染范围左边坐标值
    - right：渲染范围右边坐标值
    - bottom：渲染范围底边坐标值
    - top：渲染范围顶边坐标值
    - fmt：格式
    - blend：混合颜色
    - scale：缩放，默认取 1

### PushRenderTarget

将一个 RenderTarget 作为屏幕缓冲区，并推入栈。

- 签名：`PushRenderTarget(name: string)`
- 参数
    - name：资产名

### PopRenderTarget

将当前使用的 RenderTarget 从栈中移除，并切换到上一个 RenderTarget。

- 签名：`PopRenderTarget()`

### PostEffect

施加后处理效果。

参数列表可以满足如下格式：

```typescript
type args = Record<string, string | number | LSTGColor>
```

当传递`string`类型时，将会应用对应名称的纹理资源到 Sampler 上。
当传递`number`类型时，将会设置到对应的 Uniform 上。
当传递`LSTGColor`类型时，会转换到 uint32_t 并设置到对应的 Uniform 上。

- 签名：`PostEffect(name: string, fx: string, blend: string, args?: table)`
- 参数
    - name：纹理资产名
    - fx：FX 资产名
    - blend：混合模式，详见[混合选项](../../guide/Subsystem/RenderSystem.md#混合选项)，仅使用颜色混合分量
    - args：传递给 FX 的参数

### PostEffectCapture

开始捕获绘制数据。

从这一步开始，所有后续渲染操作都将在PostEffect缓冲区中进行。

这一操作等价于`PushRenderTarget(InternalPostEffectBuffer)`。

- 签名：`PostEffectCapture()`

### PostEffectApply

结束屏幕捕获并应用PostEffect。

这一操作等价于：
	
```lua
PopRenderTarget(InternalPostEffectBuffer)
PostEffect(InternalPostEffectBuffer, fx_name, blend, args)
```

由于需要配对`InternalPostEffectBuffer`，因此RenderTarget栈顶必须为`InternalPostEffectBuffer`。

换言之，代码必须满足：

```lua
PostEffectCapture(...)
...  -- 配对的Push/PopRenderTarget操作
PostEffectApply(...)
```

- 签名：`PostEffectApply(fx: string, blend: string, args?: table)`
- 参数
    - fx：FX 资产名
    - blend：混合模式
    - args：参数列表

## 音频方法

详见[音频子系统](../../guide/Subsystem/AudioSystem.md)。

### PlaySound

播放音效。

如果一个音效正在播放，则会打断过程从头重新开始播放。

- 签名：`PlaySound(name: string, vol: number, pan?: number)`
- 参数
    - name：资产名
    - vol：音量，取值范围[0, 1]
    - pan：平衡（默认为0），取值范围[-1, 1]

### StopSound

停止播放音效。

- 签名：`StopSound(name: string)`
- 参数
    - name：资产名

### PauseSound

暂停播放音效。

- 签名：`PauseSound(name: string)`
- 参数
    - name：资产名

### ResumeSound

继续播放音效。

- 签名：`ResumeSound(name: string)`
- 参数
    - name：资产名

### GetSoundState

获取音效播放状态。

- 签名：`GetSoundState(name: string): string`
- 参数
    - name：资产名
- 返回值：音效播放状态，可取`paused`、`playing`、`stopped`

### PlayMusic

播放背景音乐。

- 签名：`PlayMusic(name: string, vol?: number, position?: number)`
- 参数
    - name：资产名
    - vol：音量，默认为 1
    - position：播放起始位置（秒），默认为 0

### StopMusic

停止播放音乐。

- 签名：`StopMusic(name: string)`
- 参数
    - name：资产名

### PauseMusic

暂停播放音乐。

- 签名：`PauseMusic(name: string)`
- 参数
    - name：资产名

### ResumeMusic

继续播放音乐。

- 签名：`ResumeMusic(name: string)`
- 参数
    - name：资产名

### GetMusicState

获取音乐播放状态。

- 签名：`GetMusicState(name: string): string`
- 参数
    - name：资产名
- 返回值：音乐播放状态，可取`paused`、`playing`、`stopped`

### UpdateSound <Badge type="warning" vertical="middle" text="deprecated" />

更新音频系统状态。

::: warning
该方法已废弃，不再有任何效果。
:::

### SetSEVolume

设置音效整体的音量。

- 签名：`SetSEVolume(vol: number)`
- 参数
    - vol：音量，取值[0, 1]

### SetBGMVolume

当只有一个参数时，用于设置背景音乐整体的音量。

若有两个参数时，设置指定音乐的播放音量。

- 签名：`SetBGMVolume(arg: number|string, vol?: number)`
- 参数
    - arg：指定全局背景音乐音量大小或者指定资产名
    - vol：音量，取值[0, 1]

## 输入

### GetKeyState

检查按键是否按下。

- 签名：`GetKeyState(vkCode: number): boolean`
- 参数
    - vkCode：键扫描代码，采用微软定义的虚拟键代码

### GetLastKey

返回最后一次输入的按键的扫描代码。

- 签名：`GetLastKey(): number`
- 返回值：虚拟键代码

### GetLastChar

返回最后一次输入的字符。

- 签名：`GetLastChar(): string`
- 返回值：字符

### GetMousePosition

获取鼠标的位置，以窗口左下角为原点。

- 签名：`GetMousePosition(): [number, number]`
- 返回值：鼠标位置（X, Y）

### GetMouseState

检查鼠标按键是否按下。

- 签名：`GetMouseState(button: number): boolean`
- 参数
    - button：可取值0,1,2，表示鼠标左键、中键和右键

## 对象池管理方法

### ObjTable <Badge type="warning" vertical="middle" text="deprecated" />

该方法可以获得对象池所在的 table。

- 签名：`ObjTable(): table`

::: warning
该方法已废弃，不再有任何效果。
:::

### GetnObj

获取对象池中对象个数。

- 签名：`GetnObj(): number`
- 返回值：对象个数

### UpdateObjList <Badge type="warning" vertical="middle" text="deprecated" />

更新对象池。此时将所有对象排序并归类。

::: warning
该方法已废弃，不再有任何效果。
:::

### ObjFrame

更新对象列表中所有对象，并更新属性。

::: warning
禁止在协程上调用该方法。
:::

- 签名：`ObjFrame()`

::: tip
按照下列顺序更新这些属性：
- vx += ax
- vy += ay
- x += vx
- y += vy
- rot += omiga
- 更新绑定的粒子效果（若有）
:::

### ObjRender

渲染所有对象。

对象会按照`layer`进行排序，`layer`越小越先渲染。

::: warning
禁止在协程上调用该方法。
:::

- 签名：`ObjRender()`

### SetBound

设置场景边界。

当对象设置越界销毁时，超过场景边界的对象会被自动删除。

- 签名：`SetBound(left:number, right:number, bottom:number, top:number)`
- 参数
    - left：左边坐标值
    - right：右边坐标值
    - bottom：底边坐标值
    - top：顶边坐标值

### BoundCheck

执行边界检查。

注意`BoundCheck`只保证对象中心还在范围内，不进行碰撞盒检查。

::: warning
禁止在协程上调用该方法。
:::

- 签名：`BoundCheck()`

### CollisionCheck

对组 A 和 B 进行碰撞检测。

如果组 A 中对象与组 B 中对象发生碰撞，将执行 A 中对象的碰撞回调函数。

::: warning
禁止在协程上调用该方法。
:::

- 签名：`CollisionCheck(groupA: number, groupB: number)`

### UpdateXY

刷新对象的 dx, dy, lastx, lasty, rot（若navi=true）值。

::: warning
禁止在协程上调用该方法。
:::

- 签名：`UpdateXY()`

### AfterFrame

刷新对象的 timer 和 ani_timer，若对象被标记为 del 或 kill 将删除对象并回收资源。

::: warning
禁止在协程上调用该方法。
:::

::: tip
对象只有在AfterFrame调用后才会被清理，在此之前可以通过设置对象的 status 字段取消删除标记。
:::

- 签名：`AfterFrame()`

### New

创建新对象。

该方法使用 class 创建一个对象，并在构造对象后调用 class 的构造方法构造对象。

被创建的对象具有如下属性：

- x, y：坐标
- dx, dy：(只读)距离上一次更新的坐标增量
- rot：角度
- omiga：角度增量
- timer：计数器，每帧增加
- vx, vy：速度
- ax, ay：加速度
- layer：渲染层级
- group：碰撞组
- hide：是否隐藏
- bound：是否越界销毁
- navi：是否自动更新朝向
- colli：是否允许碰撞
- status：对象状态，返回"del"、"kill"、"normal"
- hscale, vscale：横向、纵向的缩放程度
- class：对象的父类
- a, b：碰撞盒大小
- rect：是否为矩形碰撞盒
- img：绑定的渲染对象，可以是精灵、动画序列、粒子效果        
- ani：(只读)动画计数器

被创建对象的索引 1 和 2 被用于存放类和 `id`

其中父类class需满足如下形式：

- is_class = true
- [1] = 初始化函数 (object, ...)
- [2] = 删除函数(DEL) (object, ...)
- [3] = 帧函数 (object)
- [4] = 渲染函数 (object)
- [5] = 碰撞函数 (object, object)
- [6] = 消亡函数(KILL) (object, ...)

上述回调函数将在对象触发相应事件时被调用
                
- 签名：`New(class: table)`
- 参数
    - class：对象所用的类

### Del

通知删除一个对象。将设置标志并调用回调函数。

- 签名：`Del(object: table, ...)`
- 参数
    - object：要删除的对象
    - ...：透传给回调方法的参数

### kill

通知杀死一个对象。将设置标志并调用回调函数。

- 签名：`Kill(object: table, ...)`
- 参数
    - object：要杀死的对象
    - ...：透传给回调方法的参数

### IsValid

检查对象是否有效。

- 签名：`IsValid(object: table): boolean`
- 返回值：对象是否有效

### GetV

获取对象的速度，依次返回速度大小和速度方向。

- 签名：`GetV(object: table): [number, number]`
- 参数
    - object：要获取的对象
- 返回值：速度大小, 速度方向

### SetV

设置对象的速度。

- 签名：`SetV(object: table, v: number, a: number, track?: boolean)`
- 参数
    - object：要设置的对象
    - v：速度大小
    - a：角度
    - track：是否设置方向跟随，若设置为 true，则同时设置 rot，默认为 false

### SetImgState

设置对象绑定的资产的状态。

::: warning
该方法会影响所有同名资源。
:::

- 签名：`SetImgState(object: table, blend: string, a: number, r: number, g: number, b: number)`
- 参数
    - object：要设置的对象
    - blend：混合模式，详见[混合选项](../../guide/Subsystem/RenderSystem.md#混合选项)
    - a：Alpha
    - r：Red
    - g：Green
    - b：Blue

### Angle

若 a, b 为对象，则求向量 < 对象b.中心 - 对象a.中心 > 相对 X 轴正方向的夹角。

否则，计算 atan2(y2 - b, x2 - a)。

- 签名：`Angle(a: table | number, b: table | number, x2?: number, y2?: number): number`
- 参数
    - a：要计算的对象或者横坐标 X
    - b：要计算的对象或者纵坐标 Y
    - x2：横坐标 X2
    - y2：纵坐标 Y2

### Dist

求距离，若 a, b 为对象，则计算 a 与 b 之间的距离。

否则，计算向量 < c, d > 与 < a, b > 之间的距离，

- 签名：`Dist(a: table | number, b: table | number, c?: number, d?: number): number`
- 参数
    - a：对象 A 或者坐标值
    - b：对象 B 或者坐标值
    - c：坐标值
    - d：坐标值

### BoxCheck

检查对象中心是否在所给范围内。

- 签名：`BoxCheck(object: table, left: number, right: number, top: number, bottom: number): boolean`
- 参数
    - object：被检测的对象
    - left：范围左侧的横坐标
    - right：范围右侧的横坐标
    - top：范围顶边的纵坐标
    - bottom：范围底边的纵坐标

### ResetPool

清空并回收所有对象。

- 签名：`ResetPool()`

### DefaultRenderFunc

在对象上调用默认渲染方法。

- 签名：`DefaultRenderFunc(object: table)`
- 参数
    - object：对象

### NextObject

获取碰撞组中的下一个元素。

若 groupId 为无效的碰撞组则返回所有对象。

返回的第一个参数为 id，第二个参数为对象。

- 签名：`NextObject(groupId: number, id: number): [number, table]`
- 参数
    - groupId：组 ID
    - id：对象 ID
- 返回值：id, object

### ObjList

产生组遍历迭代器。

- 签名：`ObjList(groupId: number): [NextObject, number, number]`
- 参数
    - groupId：组 ID
- 返回值：NextObject 方法, groupId, id

### ParticleFire

启动绑定在对象上的粒子发射器。

- 签名：`ParticleFire(object: table)`
- 参数
    - object：对象

### ParticleStop

停止绑定在对象上的粒子发射器。

- 签名：`ParticleStop(object: table)`
- 参数
    - object：对象

### ParticleGetn

返回绑定在对象上的粒子发射器的存活粒子数。

- 签名：`ParticleGetn(object: table)`
- 参数
    - object：对象

### ParticleGetEmission

获取绑定在对象上粒子发射器的发射密度。（个/秒）

- 签名：`ParticleGetEmission(object: table)`
- 参数
    - object：对象

### ParticleSetEmission

设置绑定在对象上粒子发射器的发射密度。（个/秒）

- 签名：`ParticleSetEmission(object: table, count: number)`
- 参数
    - object：对象
    - count：密度

## 杂项

### Registry <Badge type="warning" vertical="middle" text="deprecated" />

返回注册表。

LuaSTGPlus 不支持该方法。

::: warning
该方法已废弃，不再有任何效果。
:::

### Snapshot

记录截图并保存为 PNG 格式。

该方法会异步的完成。

- 签名：`Snapshot(path: string)`
- 参数
    - path：保存路径，以`/storage`为根路径

### Execute <Badge type="warning" vertical="middle" text="deprecated" />

执行外部程序。

- 签名：`Execute(path: string, arguments?: string, directory?: string, wait?: boolean): boolean`
- 参数
    - path：程序路径
    - arguments：参数
    - directory：工作目录
    - wait：是否等待，默认`true`

::: warning
该方法已废弃，不再有任何效果。
:::

### Rand

构造一个随机数生成器。

以当前系统时间作为种子。

- 签名：`Rand(): LSTGRandomizer`
- 返回值：随机数发生器

### Color

构造一个颜色。

- 签名：`Color(a: number, r?: number, g?: number, b?: number): LSTGColor`
- 参数
    - a：当指定四个参数时，表示 alpha。否则将解释为 ARGB 颜色值
    - r：Red 通道值，取 0-255
    - g：Green 通道值，取 0-255
    - b：Blue 通道值，取 0-255
- 返回值：颜色对象
 
### BentLaserData

构造一个曲线激光控制器。

- 签名：`BentLaserData(): LSTGBentLaserData`
- 返回值：曲线激光对象
