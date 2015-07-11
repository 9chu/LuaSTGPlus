using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.ComponentModel;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using DbMon.NET;
using Bencode;

namespace PerformanceMonitor
{
    /// <summary>
    /// 目标被监控程序
    /// </summary>
    public class TargetProgram
    {
        Regex _Regex = new Regex("(\\[(INFO|ERRO|WARN)\\])?(.*)", RegexOptions.Compiled | RegexOptions.Multiline);

        UdpClient _UdpClient;
        ushort _Port;

        Process _Process;

        PerformanceCounter _WorkingSet;
        PerformanceCounter _PrivateWorkingSet;
        PerformanceCounter _CpuTime;

        float _FPS = 0;
        float _Objects = 0;
        float _FrameTime = 0;
        float _RenderTime = 0;

        /// <summary>
        /// udp数据类型
        /// </summary>
        private enum UdpMessageType
        {
            PerformanceUpdate = 1,
            ResourceLoaded = 2,
            ResourceRemoved = 3,
            ResourceCleared = 4
        }

        /// <summary>
        /// 日志类型
        /// </summary>
        public enum LogType
        {
            Information,
            Warning,
            Error
        }

        /// <summary>
        /// 资源类型
        /// </summary>
        public enum ResourceType
	    {
		    Texture = 1,
		    Sprite,
		    Animation,
		    Music,
		    SoundEffect,
		    Particle,
		    SpriteFont,
		    TrueTypeFont,
		    FX
        }

        /// <summary>
        /// 资源池类型
        /// </summary>
	    public enum ResourcePoolType
	    {
		    None = 0,
		    Global,
		    Stage
	    }

        /// <summary>
        /// 调试器端口
        /// </summary>
        public ushort DebuggerPort
        {
            get
            {
                return _Port;
            }
        }

        /// <summary>
        /// 异步回调接口
        /// </summary>
        public ISynchronizeInvoke SynchronizeInvoke { get; set; }

        /// <summary>
        /// 运行时间
        /// </summary>
        public TimeSpan Lifetime
        {
            get
            {
                if (_Process == null)
                    return new TimeSpan();
                if (_Process.HasExited)
                    return _Process.ExitTime - _Process.StartTime;
                return DateTime.Now - _Process.StartTime;
            }
        }

        /// <summary>
        /// 获取工作集大小
        /// </summary>
        public float WorkingSet
        {
            get
            {
                if (_WorkingSet == null)
                    return 0;
                return _WorkingSet.NextValue();
            }
        }

        /// <summary>
        /// 获取私有工作集大小
        /// </summary>
        public float PrivateWorkingSet
        {
            get
            {
                if (_PrivateWorkingSet == null)
                    return 0;
                return _PrivateWorkingSet.NextValue();
            }
        }

        /// <summary>
        /// 获取处理器时间
        /// </summary>
        public float CpuTime
        {
            get
            {
                if (_CpuTime == null)
                    return 0;
                return _CpuTime.NextValue();
            }
        }

        /// <summary>
        /// 获取FPS
        /// </summary>
        public float FPS
        {
            get
            {
                lock (this)
                {
                    return _FPS;
                }
            }
        }

        /// <summary>
        /// 获取对象数
        /// </summary>
        public float Objects
        {
            get
            {
                lock (this)
                {
                    return _Objects;
                }
            }
        }

        /// <summary>
        /// 获取逻辑时间
        /// </summary>
        public float FrameTime
        {
            get
            {
                lock (this)
                {
                    return _FrameTime;
                }
            }
        }

        /// <summary>
        /// 获取渲染时间
        /// </summary>
        public float RenderTime
        {
            get
            {
                lock (this)
                {
                    return _RenderTime;
                }
            }
        }

        // 事件回调
        public delegate void OnProcessExitHandler(int exitCode);
        public delegate void OnProcessTraceHandler(DateTime time, LogType type, string message);
        public delegate void OnResourceLoadedHandler(ResourceType type, ResourcePoolType pool, string name, string path, float time);
        public delegate void OnResourceRemovedHandler(ResourceType type, ResourcePoolType pool, string name);
        public delegate void OnResourceClearedHandler(ResourcePoolType pool);

        public event OnProcessExitHandler OnProcessExit;
        public event OnProcessTraceHandler OnProcessTrace;
        public event OnResourceLoadedHandler OnResourceLoaded;
        public event OnResourceRemovedHandler OnResourceRemoved;
        public event OnResourceClearedHandler OnResourceCleared;

        public void Kill()
        {
            if (_Process != null && !_Process.HasExited)
                _Process.Kill();
        }

        public bool IsRunning()
        {
            if (_Process == null)
                return false;
            return !_Process.HasExited;
        }
        
        public void Start(string path, string workDirectory)
        {
            if (_Process != null && !_Process.HasExited)
                throw new InvalidOperationException();

            ProcessStartInfo tInfo = new ProcessStartInfo(path);
            tInfo.Arguments = "/debugger:" + DebuggerPort.ToString();
            tInfo.WorkingDirectory = workDirectory;

            DebugMonitor.Start();

            Thread.Sleep(500);

            try
            {
                _Process = Process.Start(tInfo);
                _Process.EnableRaisingEvents = true;
                _Process.Exited += Process_Exited;
            }
            catch
            {
                DebugMonitor.Stop();
                throw;
            }

            _WorkingSet = new PerformanceCounter("Process", "Working Set", _Process.ProcessName);
            _PrivateWorkingSet = new PerformanceCounter("Process", "Working Set - Private", _Process.ProcessName);
            _CpuTime = new PerformanceCounter("Process", "% Processor Time", _Process.ProcessName);
        }

        public void CloseSocket()
        {
            _UdpClient.Close();
        }

        void Process_Exited(object sender, EventArgs e)
        {
            DebugMonitor.Stop();

            if (OnProcessExit != null)
            {
                if (SynchronizeInvoke != null)
                {
                    SynchronizeInvoke.Invoke((Action)(() =>
                    {
                        OnProcessExit(_Process.ExitCode);
                    }), null);
                }
                else
                    OnProcessExit(_Process.ExitCode);
            }
        }

        void DebugMonitor_OnOutputDebugString(int pid, string text)
        {
            if (_Process != null && pid == _Process.Id)
            {
                Match tMatchResult = _Regex.Match(text);
                DateTime tTime = DateTime.Now;
                LogType tLogType = LogType.Information;
                string tDetail =  tMatchResult.Groups[3].Value.ToString().Trim();

                if (tMatchResult.Groups[2].Length > 0)
                {
                    switch (tMatchResult.Groups[2].Value)
                    {
                        case "INFO":
                            tLogType = LogType.Information;
                            break;
                        case "WARN":
                            tLogType = LogType.Warning;
                            break;
                        case "ERRO":
                            tLogType = LogType.Error;
                            break;
                    }
                }

                if (OnProcessTrace != null)
                {
                    if (SynchronizeInvoke != null)
                    {
                        SynchronizeInvoke.Invoke((Action)(() =>
                        {
                            OnProcessTrace(tTime, tLogType, tDetail);
                        }), null);
                    }
                    else
                        OnProcessTrace(tTime, tLogType, tDetail);
                }
            }
        }
        
        void UDP_OnDataReceived(IAsyncResult ar)
        {
            try
            {
                IPEndPoint tEndPoint = new IPEndPoint(IPAddress.Any, 0);
                Byte[] tBytesReceived = _UdpClient.EndReceive(ar, ref tEndPoint);
                Dictionary<string, object> tData = Bencode.BencodeUtility.DecodeDictionary(tBytesReceived);
                if (_Process != null && (long)tData["processId"] == _Process.Id)
                {
                    Dictionary<string, object> tArgs = (Dictionary<string, object>)tData["args"];
                    
                    ResourceType tResourceType;
                    ResourcePoolType tResourcePoolType;
                    string tResourceName;
                    string tResourcePath;
                    float tResourceTime;

                    switch ((UdpMessageType)(long)tData["msgType"])
                    {
                        case UdpMessageType.PerformanceUpdate:
                            lock (this)
                            {
                                _FPS = ((long)tArgs["fps"]) / 1000.0f;
                                _Objects = ((long)tArgs["objects"]) / 1000.0f;
                                _FrameTime = ((long)tArgs["frametime"]) / 1000.0f / 1000.0f;
                                _RenderTime = ((long)tArgs["rendertime"]) / 1000.0f / 1000.0f;
                            }
                            break;
                        case UdpMessageType.ResourceLoaded:
                            tResourceType = (ResourceType)(long)tArgs["type"];
                            tResourcePoolType = (ResourcePoolType)(long)tArgs["pool"];
                            tResourceName = Encoding.UTF8.GetString((Byte[])tArgs["name"]);
                            tResourcePath = Encoding.UTF8.GetString((Byte[])tArgs["path"]);
                            tResourceTime = ((long)tArgs["time"] / 1000.0f);
                            
                            if (OnResourceLoaded != null)
                            {
                                if (SynchronizeInvoke != null)
                                {
                                    SynchronizeInvoke.Invoke((Action)(() =>
                                    {
                                        OnResourceLoaded(tResourceType, tResourcePoolType, tResourceName, tResourcePath, tResourceTime);
                                    }), null);
                                }
                                else
                                    OnResourceLoaded(tResourceType, tResourcePoolType, tResourceName, tResourcePath, tResourceTime);
                            }
                            break;
                        case UdpMessageType.ResourceRemoved:
                            tResourceType = (ResourceType)(long)tArgs["type"];
                            tResourcePoolType = (ResourcePoolType)(long)tArgs["pool"];
                            tResourceName = Encoding.UTF8.GetString((Byte[])tArgs["name"]);

                            if (OnResourceRemoved != null)
                            {
                                if (SynchronizeInvoke != null)
                                {
                                    SynchronizeInvoke.Invoke((Action)(() =>
                                    {
                                        OnResourceRemoved(tResourceType, tResourcePoolType, tResourceName);
                                    }), null);
                                }
                                else
                                    OnResourceRemoved(tResourceType, tResourcePoolType, tResourceName);
                            }
                            break;
                        case UdpMessageType.ResourceCleared:
                            tResourcePoolType = (ResourcePoolType)(long)tArgs["pool"];

                            if (OnResourceCleared != null)
                            {
                                if (SynchronizeInvoke != null)
                                {
                                    SynchronizeInvoke.Invoke((Action)(() =>
                                    {
                                        OnResourceCleared(tResourcePoolType);
                                    }), null);
                                }
                                else
                                    OnResourceCleared(tResourcePoolType);
                            }
                            break;
                    }
                }
            }
            catch (ObjectDisposedException)
            {
                // 操作被取消
                return;
            }
            catch (Exception)
            {
            }

            // 接收下一个数据包
            _UdpClient.BeginReceive(new AsyncCallback(UDP_OnDataReceived), null);
        }

        public TargetProgram(ISynchronizeInvoke invoker, ushort port=3459)
        {
            _Port = port;
            SynchronizeInvoke = invoker;

            DebugMonitor.OnOutputDebugString += DebugMonitor_OnOutputDebugString;

            // 启动监听服务
            _UdpClient = new UdpClient(new IPEndPoint(IPAddress.Any, port));

            // 启动UDP
            _UdpClient.BeginReceive(new AsyncCallback(UDP_OnDataReceived), null);
        }
    }
}
