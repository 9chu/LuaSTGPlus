# luaSTG+ lua层API手册

## 文档约定

### 符号

- **[新]**： 表示该方法相对于基准代码增加了新的功能并保有兼容性
- **[新增]**： 表示该方法为新增的方法，不存在于基准代码中
- **[否决]**： 表示该方法可能在后续版本中移除，不再被维护
- **[移除]**： 表示该方法被移除，与基准代码不兼容
- **[改]**： 表示该方法相对于基准代码有所修改但保有兼容性
- **[不兼容]**： 表示该方法相对基准代码有所修改且不兼容
- **[补]**： 表示方法为基准代码中缺失的，不保证完全的兼容性

## 框架执行流程

- luaSTGPlus初始化
- 初始化lua引擎
- 装载`launch`文件执行初始化配置、装载资源包等
- 初始化fancy2d引擎框架
- 装载文件`core.lua`
- 执行lua全局函数`GameInit`
- 启动并执行游戏循环
- 销毁框架并退出

## 编码

- 程序将使用**UTF-8**作为lua代码的编码，如果lua端使用非UTF-8编码将在运行时导致乱码
- 程序将使用**UTF-8**作为资源包的编码，这意味着如果资源包中出现非UTF-8编码的字符将导致无法定位文件

## 内建变量

- arg:table

	保存命令行参数

## 内建类

### lstgColor

lstgColor用于表示一个基于a,r,g,b四分量的32位颜色

#### 方法

- ARGB(lstgColor):number, number, number, number

	返回颜色的a,r,g,b分量

#### 元方法

- __eq(lstgColor, lstgColor):boolean

	判断两个颜色值是否相等

- __add(lstgColor, lstgColor):lstgColor

	将两个颜色值相加，超出部分置为255

- __sub(lstgColor, lstgColor):lstgColor **[新增]**

	将两个颜色值相减，下溢部分置为0

- __mul(lstgColor|number, lstgColor|number):lstgColor **[新]**

	将两个颜色值的各个分量相乘或与一个数字相乘，超出部分置为255

- __tostring(lstgColor):string

	打印类名，该值为`lstg.Color(a,r,g,b)`其中argb为颜色分量值


### lstgRand

基于WELL512算法的随机数发生器，初始化时将使用系统tick计数作为种子 **[改]**

	细节
		luaSTG实现中采用线性同余(?)方法产生随机数，luaSTG+采用WELL512算法产生随机数（该算法摘自《游戏编程精粹 7》）

	潜在兼容性问题
		luaSTG实现中默认使用种子(0)初始化生成器

#### 方法

- Seed(lstgRand, number)

	设置随机数种子（注意：number值不应该超过32bit无符号整数范围）

- GetSeed(lstgRand) **[新增]**

	返回随机数种子

- Int(lstgRand, min:number, max:number):number

	产生一个在[min,max]区间内的整型随机数，请确保min<=max

- Float(lstgRand, min:number, max:number):number

	产生一个在[min,max]区间内的浮点型随机数，请确保min<=max

- Sign(lstgRand):number

	随机返回±1

#### 元方法

- __tostring(lstgRand):string

	打印类名，该值始终为`lstg.Rand object`

## 内建方法

所有内建方法归类于lstg全局表中。

### 框架控制方法

- SetWindowed(boolean) **[不兼容]**

	设置窗口化(true)/非窗口化(false)。默认为true。

	**仅限初始化中使用**，不允许在运行时切换窗口模式。

- SetFPS(number) **[不兼容]**

	设置FPS锁定值。默认为60FPS。

	**仅限初始化中使用**，不允许在运行时动态设置FPS。

- GetFPS():number

	获得当前的实时FPS。

- SetVsync(boolean) **[不兼容]**

	设置是否垂直同步。默认为true。

	**仅限初始化中使用**，不允许在运行时动态设置垂直同步。

- SetResolution(width:number, height:number)

	设置分辨率。默认为640x480。

	仅限初始化中使用，不允许在运行时动态设置分辨率。

- SetSplash(boolean)

	设置是否显示光标。默认为false。

- SetTitle(string)

	设置窗口标题。默认为"LuaSTGPlus"。

- SystemLog(string)

	写出日志。

- Print(...)

	将若干值写到日志。

- LoadPack(path:string, password:string) **[新]**

	加载指定位置的ZIP资源包，可选填密码。

	失败将导致错误。

		细节
			后加载的资源包有较高的查找优先级。这意味着可以通过该机制加载资源包来覆盖基础资源包中的文件。
			一旦zip文件被打开，将不能被访问。
			加载文件时将按照优先级依次搜索资源包，若资源包中不含文件则从当前目录加载。

- UnloadPack(path:string)

	卸载指定位置的资源包，要求路径名必须一致。
	
	若资源包不存在不发生错误。

- ExtractRes(path:string, target:string) **[补]**

	将资源包中的数据解压到本地。

	若失败将抛出错误。

- DoFile(path:string)

	执行指定路径的脚本。已执行过的脚本会再次执行。

	若文件不存在、编译失败、执行失败则抛出错误。

## 全局回调函数

下述回调函数必须定义在脚本的全局范围中。

- GameInit()

	在游戏框架初始化完毕后，游戏循环启动前调用。

- FocusLoseFunc()

	在渲染窗口失去焦点时调用。

- FocusGainFunc()

	在渲染窗口获得焦点时调用。

- FrameFunc()

	帧处理函数，每帧被调用来处理逻辑。

	若返回true将终止游戏循环，退出游戏。
