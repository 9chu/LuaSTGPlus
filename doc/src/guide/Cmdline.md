# 命令行

当前版本提供了下述命令行选项供在启动时对引擎行为进行调整。

需要注意，您必须确保编译过程中开启了`LSTG_PARSE_CMDLINE`选项，否则命令行参数不会生效。

## -preload-pack=string

该命令行参数可以在进入引擎循环前指定加载某个资源包（通常是`.zip`文件）。

## -enable-async-loading

当设置该选项时，将默认打开异步加载功能。

::: warning
当前版本下，异步加载尚未提供相关支撑的API，当强制打开功能时表现可能不符合预期，仅作实验用。
:::

## -graphics=string

设置第一优先图形API，可选值包括：d3d11/d3d12/vulkan/opengl。

若对应平台无该图形API支持，则不会有任何效果。

## -force-fullscreen

设置强制全屏，当打开强制全屏模式时，任何窗口模式切换API均不会有效果，将总是保持全屏无边框窗口大小。

## -render-frame-skip=integer

设置跳帧，指定一个渲染帧会跳过多少个逻辑帧。

例如，当`-render-frame-skip=1`时，逻辑将保持 60 FPS，而渲染会降低到 30 FPS。

## -controller-to-key-config=string

设置手柄到按键映射配置。当指定该选项时，引擎将自动完成手柄到键盘按键的映射。

配置文件使用`JSON`编写，参考配置如下：

```json
[
  {
    "guid": "",
    "buttonA": "z",
    "buttonB": "x",
    "buttonBack": "escape",
    "buttonX": {
      "mode": "switch",
      "mapping": "z"
    },
    "buttonY": {
      "mode": "switch",
      "mapping": "shift"
    },
    "buttonLeftShoulder": "shift",
    "buttonDPadUp": "up",
    "buttonDPadDown": "down",
    "buttonDPadLeft": "left",
    "buttonDPadRight": "right",
    "axis0": {
      "xNegativeMapping": "left",
      "xPositiveMapping": "right",
      "yNegativeMapping": "down",
      "yPositiveMapping": "up"
    }
  }
]
```

手柄按键类型会以`XBox`手柄为基准，在上述配置下，将会得到效果：
- 手柄A键映射到键盘`z`键
- 手柄B键映射到键盘`x`键
- 手柄Select键映射到键盘`ESC`键
- 手柄X键映射到键盘`z`键，且以开关模式运作，即按下X键会让`z`键保持长按状态
- 左肩键映射到键盘`shift`键
- 手柄Y键映射到键盘`shift`键，且以开关模式运作
- 上下左右按键映射到键盘方向键

您也可以在配置中设置模拟轴的死区等选项，其他不予赘述。

## -cwd-log-file

设置`log.txt`打印在当前执行路径下，而不是`AppData`中。

仅**开发模式**。
