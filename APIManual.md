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

### 对象池管理方法

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
				ani
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

- Del(object, ...) **[新]**

	通知删除一个对象。将设置标志并调用回调函数。

	**若在object后传递多个参数，将被传递给回调函数。**

- Kill(object, ...) **[新]**

	通知杀死一个对象。将设置标志并调用回调函数。

	**若在object后传递多个参数，将被传递给回调函数。**

- IsValid(object)

	检查对象是否有效。

- SetV(object, v:number, a:number, track:boolean)

	以<速度大小,角度>设置对象的速度，若track为true将同时设置r。

- Angle(a:object, b:object):number

	求向量(对象b.中心 - 对象a.中心)相对x轴正方向的夹角。

- Dist(a:object|number, b:object|number, c:number, d:number):number

	求距离。若a与b为对象则计算a与b之间的距离。否则计算向量(c,d)与(a,b)之间的距离。

- BoxCheck(object, left:number, right:number, top:number, bottom:number):boolean

	检查对象中心是否在所给范围内。

- ResetPool()

	清空并回收所有对象。

	该方法必须在UpdateObjList后调用。

- DefaultRenderFunc(object)

	在对象上调用默认渲染方法。

- NextObject(groupid:number, id:number):number, object **[不兼容]**

	获取组中的下一个元素。若groupid为无效的碰撞组则返回所有对象。

		细节
			luastg中NextObject接受的第二个参数为组中的元素索引而非id。
			出于效率考虑，luastg+中接受id查询下一个元素并返回下一个元素的id。

- ObjList(groupid:number):NextObject, number, number **[不兼容]**

	产生组遍历迭代器。

		细节
			由于NextObject行为发生变更，ObjList只在for循环中使用时可以获得兼容性。

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
