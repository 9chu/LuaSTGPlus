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

- BeginScene()

	通知渲染开始。该方法必须在RenderFunc中调用。所有渲染动作必须在BeginScene/EndScene中进行。

		不兼容性
			从luastg+开始，渲染操作将被移动到RenderFunc中进行。

- EndScene()

	通知渲染结束。该方法必须在RenderFunc中调用。

- RenderClear(lstgColor)

	使用指定颜色清空屏幕。在清除颜色的同时会清除深度缓冲区。

- SetViewport(left:number, right:number, bottom:number, top:number)

	设置视口，将影响裁剪和渲染。

- SetOrtho(left:number, right:number, bottom:number, top:number)

	设置正投影矩阵。left表示x轴最小值，right表示x轴最大值，bottom表示y轴最小值，top表示y轴最大值。

		细节
			创建的正投影矩阵将把z轴限制在[0,1]区间内。

- SetPerspective(eyeX:number, eyeY:number, eyeZ:number, atX:number, atY:number, atZ:number, upX:number, upY:number, upZ:number, fovy:number, aspect:number, zn:number, zf:number)

	设置透视投影矩阵和观察矩阵。(eyeX,eyeY,eyeZ)表示观察者位置，(atX,atY,atZ)表示观察目标，(upX,upY,upZ)用于表示观察者向上的正方向。fovy描述视角范围（弧度制），aspect描述宽高比，zn和zf描述z轴裁剪距离。

- Render(image_name:string, x:number, y:number, [rot:number=0, [hscale:number=1, [vscale:number=1, [z:number=0.5]]]])

	渲染图像。(x,y)指定中心点，rot指定旋转（弧度制），(hscale,vscale)XY轴缩放，z指定Z坐标。

	若指定了hscale而没有指定vscale则vscale=hscale。

	该函数受全局图像缩放系数影响。

- RenderRect(image_name:string, left:number, right:number, bottom:number, top:number)

	在一个矩阵范围渲染图像。此时z=0.5。

- Render4V(image_name:string, x1:number, y1:number, z1:number, x2:number, y2:number, z2:number, x3:number, y3:number, z3:number, x4:number, y4:number, z4:number)

	给出四个顶点渲染图像。此时z=0.5。

- SetFog([near:number, far:number, [color:lstgColor = 0x00FFFFFF]])

	若参数为空，将关闭雾效果。否则设置一个从near到far的雾。

- RenderText(name:string, text:string, x:number, y:number, [scale:number=1, align:integer=5])

	使用纹理字体渲染一段文字。参数name指定纹理名称，text指定字符串，x、y指定坐标，align指定对齐模式。

	该函数受全局图像缩放系数影响。

		细节
			对齐模式指定渲染中心，对齐模式可取值：
				左上  0 + 0  0
				左中  0 + 4  4
				左下  0 + 8  8
				中上  1 + 0  1
				中中  1 + 4  5
				中下  1 + 8  9
				右上  2 + 0  2
				右中  2 + 4  6
				右下  2 + 8  10
			由于使用了新的布局机制，在渲染HGE字体时在横向上会有少许误差，请手动调整。

- RenderTexture(tex\_name:string, blend:string, vertex1:table, vertex2:table, vertex3:table, vertex4:table)

	直接渲染纹理。

		细节
			vertex1~4指定各个顶点坐标，其中必须包含以下项：
				[1] = X坐标
				[2] = Y坐标
				[3] = Z坐标
				[4] = U坐标（以纹理大小为区间）
				[5] = V坐标（以纹理大小为区间）
				[6] = 顶点颜色
			注意该函数效率较低，若要使用请考虑缓存顶点所用table。

- RenderTTF(name:string, text:string, left:number, right:number, bottom:number, top:number, fmt:integer, blend:lstgColor)  **[不兼容]**

	渲染TTF字体。

	该函数受全局图像缩放系数影响。

		细节
			暂时不支持渲染格式设置。 
			接口已统一到屏幕坐标系，不需要在代码中进行转换。

- PushRenderTarget(name:string) **[新增]**

	将一个RenderTarget作为屏幕缓冲区，并推入栈。

	高级方法。

		细节
			lstg+使用栈来管理RenderTarget，这意味着可以嵌套使用RenderTarget。

- PopRenderTarget()  **[新增]**

	将当前使用的RenderTarget从堆栈中移除。

	高级方法。

- PostEffect(name:string, fx:string, blend:string, [args:table]) **[新增]**

	应用PostEffect。参数指定传递给FX的参数表，将会影响后续对该FX的使用。

	其中blend指定posteffect要以什么样的形式绘制到屏幕上，此时blend的第一分量无效。

	高级方法。
	
		细节
			对于PostEffect只会渲染第一个technique中的所有pass。
			可以在PostEffect中使用下列语义注释(不区分大小写)捕获对象：
				POSTEFFECTTEXTURE获取posteffect的捕获纹理(texture2d类型)。
				VIEWPORT获取视口大小(vector类型)。
				SCREENSIZE获取屏幕大小(vector类型)。

- PostEffectCapture() **[新增]**

	开始捕获绘制数据。

	从这一步开始，所有后续渲染操作都将在PostEffect缓冲区中进行。

	这一操作等价于`PushRenderTarget(InternalPostEffectBuffer)`。

	高级方法。

- PostEffectApply(fx_name:string, blend:string, [args:table]) **[新增]**

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

	高级方法。

## 音频方法

- PlaySound(name:string, vol:number, [pan:number=0.0])

	播放一个音效。vol为音量，取值范围[0~1]，pan为平衡，取值[-1~1]。

		细节
			luastg+每次只播放一个音效，如果一个音效已在播放中则会打断这个播放从头开始。

- StopSound(name:string) **[新增]**

	停止播放音效。name为资源名称。

- PauseSound(name:string) **[新增]**

	暂停播放音效。name为资源名称。

- ResumeSound(name:string) **[新增]**

	继续播放音效。name为资源名称。

- GetSoundState(name:string):string **[新增]**

	获取音效播放状态，将返回paused、playing、stopped。

- PlayMusic(name:string, [vol:number=1.0, position:number=0])

	播放音乐。name为资源名称，vol为音量，position为起始播放位置（秒）。

- StopMusic(name:string)

	停止播放音乐。该操作会使音乐播放位置回到开头。name为资源名称。

- PauseMusic(name:string)

	暂停播放音乐。name为资源名称。

- ResumeMusic(name:string)

	继续播放音乐。name为资源名称。

- GetMusicState(name:string):string

	获取音乐播放状态，将返回paused、playing、stopped。

- UpdateSound()  **[否决]**

	**该方法已不起任何作用，将于后续版本移除。**

- SetSEVolume(vol:number)

	设置全局音效音量，将影响后续播放音效的音量。

	音量值范围为[0,1]。

- SetBGMVolume([vol:number] | [name:string, vol:number]) **[新]**

	若参数个数为1，则设置全局音乐音量。该操作将影响后续播放音乐的音量。

	若参数个数为2，则设置指定音乐的播放音量。

	音量值范围为[0,1]。

## 输入

当前，手柄输入被映射到0x92~0xB1和0xDF~0xFE（共2个手柄、32个按键）的位置上。

其中，X轴Y轴的位置被映射到前4个按键上，对应上下左右。

- GetKeyState(vk\_code:integer):boolean

	给出虚拟键代码检测是否按下。

		细节
			VK_CODE的具体含义请查阅MSDN。

- GetLastKey():integer

	返回最后一次输入的按键的虚拟键代码。

- GetLastChar():string

	返回上一次输入的字符。

- GetMousePosition():number,number **[新增]**

	获取鼠标的位置，以窗口左下角为原点。

- GetMouseState(button:integer):boolean **[新增]**

	检查鼠标按键是否按下。button可取0、1、2，分别对应鼠标左键、中键、右键。

## 对象池管理方法

- GetnObj():number

	获取对象池中对象个数。

- UpdateObjList() **[否决]**

	更新对象池。此时将所有对象排序并归类。

	排序规则：uid越小越靠前

		细节
			luaSTG+中该函数不再起任何作用，对象表总是保持有序的。

- ObjFrame()

	更新对象列表中所有对象，并更新属性。

	**禁止在协程上调用该方法。**

		细节
			按照下列顺序更新这些属性：
				vx += ax
				vy += ay
				x += vx
				y += vy
				rot += omiga
				更新绑定的粒子系统（若有）

- ObjRender()

	渲染所有对象。此时将所有对象排序。

	**禁止在协程上调用该方法。**

	排序规则：layer小的先渲染，若layer相同则按照uid

		细节
			luaSTG+中渲染列表总是保持有序的，将不会每次排序。

- SetBound(left:number, right:number, bottom:number, top:number)

	设置舞台边界。

- BoundCheck()

	执行边界检查。注意BoundCheck只保证对象中心还在范围内，不进行碰撞盒检查。

	**禁止在协程上调用该方法。**

- CollisionCheck(A:groupid, B:groupid)

	对组A和B进行碰撞检测。如果组A中对象与组B中对象发生碰撞，将执行A中对象的碰撞回调函数。

	**禁止在协程上调用该方法。**

- UpdateXY()

	刷新对象的dx,dy,lastx,lasty,rot（若navi=true）值。

	**禁止在协程上调用该方法。**

- AfterFrame()

	刷新对象的timer和ani_timer，若对象被标记为del或kill将删除对象并回收资源。

	**禁止在协程上调用该方法。**

		细节
			对象只有在AfterFrame调用后才会被清理，在此之前可以通过设置对象的status字段取消删除标记。

- New(class)

	创建新对象。将累加uid值。

		细节
			该方法使用class创建一个对象，并在构造对象后调用class的构造方法构造对象。
			被创建的对象具有如下属性：
				x, y             坐标
				dx, dy           (只读)距离上一次更新的坐标增量
				rot              角度
				omiga            角度增量
				timer            计数器
				vx, vy           速度
				ax, ay           加速度
				layer            渲染层级
				group            碰撞组
				hide             是否隐藏
				bound            是否越界销毁
				navi             是否自动更新朝向
				colli            是否允许碰撞
				status           对象状态，返回del kill normal
				hscale, vscale   横向、纵向的缩放程度
				class            对象的父类
				a, b             碰撞盒大小
				rect             是否为矩形碰撞盒
				img              
				ani              (只读)动画计数器
			被创建对象的索引1和2被用于存放类和id【请勿修改】

			其中父类class需满足如下形式：
				is_class = true
				[1] = 初始化函数 (object, ...)
				[2] = 删除函数(DEL) (object, ...) [新]
				[3] = 帧函数 (object)
				[4] = 渲染函数 (object)
				[5] = 碰撞函数 (object, object)
				[6] = 消亡函数(KILL) (object, ...) [新]
			上述回调函数将在对象触发相应事件时被调用
				
			luastg+提供了至多32768个空间共object使用。超过这个大小后将报错。

- Del(object, [...]) **[新]**

	通知删除一个对象。将设置标志并调用回调函数。

	**若在object后传递多个参数，将被传递给回调函数。**

- Kill(object, [...]) **[新]**

	通知杀死一个对象。将设置标志并调用回调函数。

	**若在object后传递多个参数，将被传递给回调函数。**

- IsValid(object)

	检查对象是否有效。

- GetV(object):number, number **[新增]**

	获取对象的速度，依次返回速度大小和速度方向。

- SetV(object, v:number, a:number, track:boolean)

	以<速度大小,角度>设置对象的速度，若track为true将同时设置r。

- SetImgState(object, blend:string, a:number, r:number, g:number, b:number)

	设置资源状态。blend指示混合模式（含义见后文）a,r,g,b指定颜色。

	该函数将会设置和对象绑定的精灵、动画资源的混合模式，该设置对所有同名资源都有效果。 

- Angle(a:object | x1:number, b:object | y1:number, [x2:number, y2:number]):number

	若a,b为对象，则求向量(对象b.中心 - 对象a.中心)相对x轴正方向的夹角。否则计算tan2(y2-y1, x2-x1)。

- Dist(a:object|number, b:object|number, [c:number, d:number]):number

	求距离。若a与b为对象则计算a与b之间的距离。否则计算向量(c,d)与(a,b)之间的距离。

- BoxCheck(object, left:number, right:number, top:number, bottom:number):boolean

	检查对象中心是否在所给范围内。

- ResetPool()

	清空并回收所有对象。

- DefaultRenderFunc(object)

	在对象上调用默认渲染方法。

- NextObject(groupid:number, id:number):number, object **[不兼容]**

	获取组中的下一个元素。若groupid为无效的碰撞组则返回所有对象。

	返回的第一个参数为id（luastg中为idx），第二个参数为对象

		细节
			luastg中NextObject接受的第二个参数为组中的元素索引而非id。
			出于效率考虑，luastg+中接受id查询下一个元素并返回下一个元素的id。

- ObjList(groupid:number):NextObject, number, number **[不兼容]**

	产生组遍历迭代器。

		细节
			由于NextObject行为发生变更，ObjList只在for循环中使用时可以获得兼容性。

- ParticleFire(object)
	
	启动绑定在对象上的粒子发射器。

- ParticleStop(object)

	停止绑定在对象上的粒子发射器。

- ParticleGetn(object)

	返回绑定在对象上的粒子发射器的存活粒子数。

- ParticleGetEmission(object)

	获取绑定在对象上粒子发射器的发射密度。（个/秒）

		细节
			luastg/luastg+更新粒子发射器的时钟始终为1/60s。

- ParticleSetEmission(object, count)

	设置绑定在对象上粒子发射器的发射密度。（个/秒）

- ObjTable():table

	该方法可以获得对象池所在的table。慎用。

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
