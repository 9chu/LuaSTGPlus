# 脚本子系统

LuaSTGPlus 采用 luajit/lua51 作为脚本虚拟机，故语言/库标准基于 lua5.1。

## 标准库调整

出于跨平台考虑，为了保证各平台行为一致，相比较标准的 lua，我们对库进行了一定调整：

- 不允许启动外部进程
    - os.execute 被禁止
    - io.popen 被禁止
- 标准库函数的 I/O 操作将基于虚拟文件系统进行
    - os.remove
    - os.rename
    - os.tmpname
    - io.*
    - loadfile
    - dofile
    - require
- require 现在不允许动态加载外部二进制库
- print 除去输出到 stdout，还会转发到日志系统
- 拉平 luajit/lua51 的差异
    - luajit
        - 关闭 FFI 支持
    - lua
        - 引入 bit 库
- 默认引入下述第三方库
    - cjson
    - lfs（基于 VFS 的兼容实现）

## 沙箱与热更新机制

### 沙箱模式

为了方便快速开发迭代，我们在 LuaSTGPlus 中引入了沙箱和热更新机制。

在沙箱模式下，**脚本文件会形成独立的执行环境**，在脚本文件中定义的任意全局方法将仅存在于该文件的环境中，例如：

```lua
-- 1.lua
function foo() end
print(type(foo))  -- function

-- 2.lua
print(type(foo))  -- nil
```

如果你希望将方法定义到全局环境中，需要显式进行定义：

```lua
-- 1.lua
function _G.foo() end
print(type(foo))  -- function

-- 2.lua
print(type(foo))  -- function
```

使用`import`方法来以沙箱模式加载脚本文件。

```lua
a = import("1.lua")
```

此时，`import`的返回值即为脚本的执行环境。通过这一方法可以访问其他脚本中所定义的值。

需要注意，脚本文件会在第一次`import`时被执行，往后相同文件不会再被加载，而是直接返回之前创建的执行环境。

综上，相比较`require`加载，区别如下：

- 方法默认定义在脚本独立的环境中而不是全局环境中
- `require`返回脚本执行结果，`import`返回脚本执行环境

::: warning
`require`和`import`为两套独立机制，互相不干扰，但是不推荐混合使用。
:::

### 热更新机制

我们在沙箱模式基础上建立起热更新机制。

脚本子系统会在帧更新期间自动检查脚本文件是否发生过修改，如果发生过修改，脚本子系统会自动重新加载文件并执行。

通过这种机制，脚本中定义的方法将得以重新覆盖原始定义，从而达到热更新的效果。

需要注意的是，脚本在重新执行时，原有的执行环境不会被清空，因此可以通过判空的方式检查原有变量是否存在。

此外，热更新仅在**开发模式**下生效。
