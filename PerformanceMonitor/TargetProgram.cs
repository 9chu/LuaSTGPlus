using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.ComponentModel;
using DbMon.NET;

namespace PerformanceMonitor
{
    /// <summary>
    /// 目标被监控程序
    /// </summary>
    public class TargetProgram
    {
        Regex _Regex = new Regex("(\\[(INFO|ERRO|WARN)\\])?(.*)", RegexOptions.Compiled);

        Process _Process;
        PerformanceCounter _WorkingSet;
        PerformanceCounter _PrivateWorkingSet;
        PerformanceCounter _CpuTime;

        float _FPS = 0;
        float _Objects = 0;
        float _FrameTime = 0;
        float _RenderTime = 0;

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
                return _FPS;
            }
        }

        /// <summary>
        /// 获取对象数
        /// </summary>
        public float Objects
        {
            get
            {
                return _Objects;
            }
        }

        /// <summary>
        /// 获取逻辑时间
        /// </summary>
        public float FrameTime
        {
            get
            {
                return _FrameTime;
            }
        }

        /// <summary>
        /// 获取渲染时间
        /// </summary>
        public float RenderTime
        {
            get
            {
                return _RenderTime;
            }
        }

        // 事件回调
        public delegate void OnProcessExitHandler(int exitCode);
        public delegate void OnProcessTraceHandler(DateTime time, LogType type, string message);

        public event OnProcessExitHandler OnProcessExit;
        public event OnProcessTraceHandler OnProcessTrace;

        public void Kill()
        {
            if (!_Process.HasExited)
                _Process.Kill();
        }

        public bool IsRunning()
        {
            return !_Process.HasExited;
        }

        void Process_Exited(object sender, EventArgs e)
        {
            DebugMonitor.Stop();
            DebugMonitor.OnOutputDebugString -= DebugMonitor_OnOutputDebugString;

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

        public TargetProgram(string path, string workDirectory, ISynchronizeInvoke invoker)
        {
            SynchronizeInvoke = invoker;

            ProcessStartInfo tInfo = new ProcessStartInfo(path);
            tInfo.WorkingDirectory = workDirectory;

            DebugMonitor.OnOutputDebugString += DebugMonitor_OnOutputDebugString;
            DebugMonitor.Start();

            try
            {
                _Process = Process.Start(tInfo);
                _Process.EnableRaisingEvents = true;
                _Process.Exited += Process_Exited;
            }
            catch
            {
                DebugMonitor.Stop();
                DebugMonitor.OnOutputDebugString -= DebugMonitor_OnOutputDebugString;
                throw;
            }

            _WorkingSet = new PerformanceCounter("Process", "Working Set", _Process.ProcessName);
            _PrivateWorkingSet = new PerformanceCounter("Process", "Working Set - Private", _Process.ProcessName);
            _CpuTime = new PerformanceCounter("Process", "% Processor Time", _Process.ProcessName);
        }
    }
}
