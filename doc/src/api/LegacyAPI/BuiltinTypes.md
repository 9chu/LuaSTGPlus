# 内建类型

内建类型的构造方法参考[杂项](./BuiltinMethods.md#杂项)。

## LSTGColor

`LSTGColor` 用于表示一个基于 a, r, g, b 四分量的 32 bit 颜色值。

### 成员方法

#### ARGB

返回颜色分量。

- 签名：`ARGB(self: LSTGColor): number, number, number, number`
- 返回值：a, r, g, b 分量

### 元方法

#### __eq

判断两个颜色值是否相等。

#### __add

将两个颜色值相加，超出部分置为255。

#### __sub

将两个颜色值相减，下溢部分置为0。

#### __mul

将两个颜色值的各个分量相乘或与一个数字相乘，超出部分置为255。

#### __tostring

打印类名，该值为`lstg.Color(a,r,g,b)`。其中 a, r, g, b 为颜色分量值。

## LSTGRandomizer

随机数发生器。

### 成员方法

#### Seed

设置随机数种子。

- 签名：`Seed(self: LSTGRandomizer, value: number)`
- 参数
    - value：种子

#### GetSeed

获取随机数种子。

- 签名：`GetSeed(self: LSTGRandomizer): number`
- 返回值：随机数种子

#### Int

返回 [lower, upper] 上的整数随机数。

- 签名：`Int(self: LSTGRandomizer, lower: number, upper: number): number`
- 参数
    - lower：下界
    - upper：上界
- 返回值：随机整数

#### Float

返回 [lower, upper) 上的浮点随机数。

- 签名：`Float(self: LSTGRandomizer, lower: number, upper: number): number`
- 参数
    - lower：下界
    - upper：上界
- 返回值：随机浮点数

#### Sign

返回 -1 或者 +1。

- 签名：`Sign(self: LSTGRandomizer): number`
- 返回值：正负 1。

### 元方法

#### __tostring

总是返回`lstgRandomizer`。

## LSTGBentLaserData

曲线激光。

### 成员方法

#### Update

更新对象。

- 签名：`Update(self: LSTGBentLaserData, length: number, width: number)`
- 参数
    - length：节点数
    - width：曲光宽度（影响碰撞）

#### Release <Badge type="warning" vertical="middle" text="deprecated" />

释放对象。

::: warning
该方法已废弃，不再有任何效果。
:::

#### Render

渲染对象。

- 签名：`Render(self: LSTGBentLaserData, texture: string, blend: string, color: LSTGColor, texLeft: number, texTop: number, texWidth: number, texHeight: number, scale?: number)`
- 参数
    - texture：用于填充曲光的纹理
    - blend：混合模式，详见[混合选项](../../guide/Subsystem/RenderSystem.md#混合选项)
    - color：混合颜色
    - texLeft：距离纹理左边的距离（受 PPU 影响）
    - texTop：距离纹理顶边的距离（受 PPU 影响）
    - texWidth：纹理宽度（受 PPU 影响）
    - texHeight：纹理高度（受 PPU 影响）
    - scale：缩放，默认为 1

#### CollisionCheck

执行碰撞检查。

- 签名：`CollisionCheck(self: LSTGBentLaserData, x: number, y: number, rot?: number, a?: number, b?: number, rect?: boolean): boolean`
- 参数
    - x：被检查的对象坐标 X
    - y：被检查的对象坐标 Y
    - rot：被检查的对象旋转
    - a：被检查的对象碰撞盒横向半长
    - b：被检查的对象碰撞盒纵向半长
    - rect：被检查对象的碰撞盒是否是矩形
- 返回值：是否发生碰撞

#### BoundCheck

检查曲线激光是否还在范围内。

- 签名：`BoundCheck(self: LSTGBentLaserData): boolean`
- 返回值：true 还在边界内，false 提示越界。

### 元方法

#### __tostring

总是返回`lstgBentLaserData`。
