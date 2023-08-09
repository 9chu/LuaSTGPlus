# 编译

## 前置依赖

前置依赖不区分具体平台，无论您希望在哪个平台构建**LuaSTGPlus**，都需安装下述依赖项。

- 支持 C++17 特性的编译器
- cmake >= 3.19
- git >= 2.25
- python >= 3.6

需要注意，构建过程中需要连接网络以安装依赖，请确保具有可以连通 Github 的网络环境。

## 构建选项

下述构建选项可以在编译时传递给 CMake，以开关某些功能，或配置交叉编译环境。

### LSTG_SHIPPING

- 可选值：ON(1)/OFF(0)
- 默认值：ON

当配置有`LSTG_SHIPPING`时，代码中供开发所用的功能会全数关闭，包括但不局限于：

- 脚本及资产热加载
- 性能剖析视图
- 调试控制台
- 部分代码逻辑检查和断言

建议仅当发布时开启该选项。

当`LSTG_SHIPPING=0`时，我们称之为**开发模式**。

### LSTG_ENABLE_ASSERTIONS

- 可选值：ON(1)/OFF(0)
- 默认值：ON(Debug)/OFF(其他)

该选项用于强制开启`-D_DEBUG`等调试宏，您通常无需关注该选项。

### LSTG_APP_NAME

- 可选值：字符串
- 默认值：`default`

该选项用于指定在用户存储路径（如`AppData`）中的子文件夹名称，以防止存储的分数信息、Replay数据冲突。

### LSTG_PARSE_CMDLINE

- 可选值：ON(1)/OFF(0)
- 默认值：ON

是否允许引擎解析命令行字符串，当开启该选项时，用户将可以通过命令行参数调整引擎的行为，例如：

```bash
./LuaSTGPlus2 -graphics=vulkan
```

命令行参数详见[命令行](./Cmdline.md)章节。

需要注意，当开启该功能时，所传递给Lua侧的参数必须附加在`--`之后，例如：

```bash
./LuaSTGPlus2 -graphics=vulkan -- launch
```

如果您希望还原之前的行为，可以关闭该选项，但同时将无法通过命令行参数调整引擎行为。

### LSTG_DISABLE_HOT_RELOAD

- 可选值：ON(1)/OFF(0)
- 默认值：OFF

是否关闭热加载功能（仅限**开发模式**）。

### LSTG_CROSSCOMPILING_EARLY_BUILD

- 可选值：ON(1)/OFF(0)
- 默认值：ON

在交叉编译时是否启用二阶段编译。

二阶段编译用于生成`Native`工具链，若关闭功能，需要手工指定编译时依赖的工具。

### LSTG_EARLY_BUILD_GENERATOR

- 默认值：`N/A`

用于指定在二阶段编译时使用的`Generator`，若不设置，将使用与交叉编译时相同的构建工具。

## 编译方式

::: warning
我们不提供安装脚本，请不要使用 `make install` 尝试进行安装。
:::

### Windows

在安装完前置依赖后，您可以通过`CMake`命令行或`CMake`图形化工具进行构建工作。

::: tip
我们推荐直接在`Visual Studio`中通过`打开文件夹`功能进行编译和开发工作。
:::

### MacOS

在安装完前置依赖后，您可以通过`CMake`命令行进行构建工作。

```bash
git clone https://github.com/9chu/LuaSTGPlus
cd LuaSTGPlus
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

::: tip
我们推荐直接在`CLion`中进行编译和开发工作。
:::

### Linux

由于Linux环境相对灵活，请参照SDL2的各类前置依赖按需构建。

此处给出在`Ubuntu 22.04 Arm64`下的构建流程供参考。

#### 安装构建工具

```bash
sudo apt install cmake git gcc g++ make python3
```

::: warning
您所用Linux发行版对应的`git`、`cmake`等工具可能不满足需要，若版本过低可能需要您手工编译符合前置要求的版本。
:::

#### 安装依赖

```bash
sudo apt install libx11-xcb-dev libpulse-dev libssl-dev libxext-dev libglx-dev libgl-dev
```

#### 编译

```bash
git clone http://github.com/9chu/LuaSTGPlus
cd LuaSTGPlus
mkdir build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

::: tip
若构建SDL2时出现找不到依赖的头文件的提示，但是确实已经安装了对应的包，则您可能需要删除`CMakeCache.txt`后重试。
:::

### Emscripten

**LuaSTGPlus**使用**emscripten**编译到**wasm**，使之可以在浏览器中执行。

当您使用**emscripten**构建**LuaSTGPlus**时，请确保**emsdk**已经升级到了最新版本，较老的版本可能会造成无法编译的情况。

**emscripten**的环境准备请参考[官方文档](https://emscripten.org/docs/getting_started/downloads.html)。

::: tip
我们推荐在Linux或者macOS环境下进行构建。
:::

#### 编译

```bash
mdkir cmake-build-release-emscripten
cd cmake-build-release-emscripten
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=~/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ..
make -j$(nproc)
```

#### 手工构建本机工具（不推荐）

下述内容仅在不启用`LSTG_CROSSCOMPILING_EARLY_BUILD`时参考。

##### 准备原生版本

由于交叉编译过程依赖一些本机可执行的二进制工具，您需要预先编译好原生版本供使用。

具体构建命令可参考上文，这里不再赘述。

##### 构建Emscripten版本

```bash
mdkir cmake-build-release-emscripten
cd cmake-build-release-emscripten
# 请根据实际情况调整工具链位置和原生版本的构建产物目录（需要绝对路径）
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=~/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DLSTG_CROSSCOMPILING_EARLY_BUILD=OFF -DIcuBuildTools_DIR=~/LuaSTGPlus/build ..
make -j$(nproc)
```
