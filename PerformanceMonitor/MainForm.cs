using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.IO;
using System.Windows.Forms;

namespace PerformanceMonitor
{
    public partial class MainForm : Form
    {
        TargetProgram _Target;
        bool _CloseAfterExited = false;

        public MainForm()
        {
            InitializeComponent();
        }

        void Target_OnProcessTrace(DateTime t, TargetProgram.LogType type, string message)
        {
            ListViewItem tItem = new ListViewItem();
            switch (type)
            {
                case TargetProgram.LogType.Information:
                    tItem.Text = "信息";
                    tItem.ImageIndex = imageList_main.Images.IndexOfKey("info");
                    break;
                case TargetProgram.LogType.Error:
                    tItem.Text = "错误";
                    tItem.ImageIndex = imageList_main.Images.IndexOfKey("error");
                    break;
                case TargetProgram.LogType.Warning:
                    tItem.Text = "警告";
                    tItem.ImageIndex = imageList_main.Images.IndexOfKey("warning");
                    break;
            }
            tItem.SubItems.Add(new ListViewItem.ListViewSubItem(tItem, t.ToString("hh:mm:ss.fff")));
            tItem.SubItems.Add(new ListViewItem.ListViewSubItem(tItem, message));
            listView_log.Items.Add(tItem);

            if (listView_log.Items.Count > 5000)
                listView_log.Items.RemoveAt(0);

            listView_log.Items[listView_log.Items.Count - 1].EnsureVisible();
        }

        void Target_OnProcessExit(int exitCode)
        {
            toolStripStatusLabel_status.Text = String.Format("已终止 - {0}", _Target.Lifetime.ToString("g"));

            // 关闭计时器
            timer_main.Enabled = false;

            toolStripButton_selectApp.Enabled = true;
            toolStripTextBox_path.Enabled = true;
            toolStripButton_launch.Enabled = true;

            if (_CloseAfterExited)
                Close();
        }

        private void toolStripButton_selectApp_Click(object sender, EventArgs e)
        {
            if (openFileDialog_main.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                toolStripTextBox_path.Text = openFileDialog_main.FileName;
        }

        private void toolStripButton_launch_Click(object sender, EventArgs e)
        {
            toolStripButton_selectApp.Enabled = false;
            toolStripTextBox_path.Enabled = false;
            toolStripButton_launch.Enabled = false;

            listView_log.Items.Clear();
            textBox_log.Text = String.Empty;

            try
            {
                _Target = new TargetProgram(toolStripTextBox_path.Text, Path.GetDirectoryName(toolStripTextBox_path.Text), this);
                toolStripStatusLabel_status.Text = String.Format("运行中 - {0}", _Target.Lifetime.ToString("g"));
            }
            catch (Exception ex)
            {
                MessageBox.Show("无法启动性能监测。\n\n错误：" + ex.Message, "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);

                toolStripButton_selectApp.Enabled = true;
                toolStripTextBox_path.Enabled = true;
                toolStripButton_launch.Enabled = true;
                return;
            }

            performanceChart_main.Reset();

            // 绑定事件
            _Target.OnProcessTrace += Target_OnProcessTrace;
            _Target.OnProcessExit += Target_OnProcessExit;

            // 启动计时器
            timer_main.Enabled = true;
        }

        private void listView_log_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (listView_log.SelectedItems.Count != 0)
            {
                ListViewItem tItem = listView_log.SelectedItems[0];
                textBox_log.Text = String.Format("{1} [{0}]：{2}",
                    tItem.SubItems[0].Text,
                    tItem.SubItems[1].Text,
                    tItem.SubItems[2].Text
                    ).Replace("\r\n", "\n").Replace("\n", "\r\n");
            }
        }

        private void MainForm_Load(object sender, EventArgs e)
        {

        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (_Target != null && _Target.IsRunning())
            {
                if (MessageBox.Show(
                    "程序正在运行，是否退出？", "提示",
                    MessageBoxButtons.YesNo,
                    MessageBoxIcon.Question) == System.Windows.Forms.DialogResult.Yes)
                {
                    _Target.Kill();
                    _CloseAfterExited = true;
                }
                e.Cancel = true;
            }
        }

        private void timer_main_Tick(object sender, EventArgs e)
        {
            if (_Target.IsRunning())
            {
                float tCPUTime = _Target.CpuTime;
                float tWorkingSet = _Target.WorkingSet;
                float tFPS = _Target.FPS;
                float tObjects = _Target.Objects;
                float tFrameTime = _Target.FrameTime;
                float tRenderTime = _Target.RenderTime;

                label_cpu.Text = String.Format("CPU {0:N2}%", tCPUTime);
                label_memory.Text = String.Format("内存 {0:N2}MB", tWorkingSet / 1024 / 1024);
                label_fps.Text = String.Format("FPS {0}", tFPS);
                label_objects.Text = String.Format("对象 {0}", tObjects);
                label_frame.Text = String.Format("逻辑 {0}", tFrameTime);
                label_render.Text = String.Format("渲染 {0}", tRenderTime);

                performanceChart_main.AddPerformance(PerformanceChart.PerformanceArg.CPU, tCPUTime);
                performanceChart_main.AddPerformance(PerformanceChart.PerformanceArg.Memory, tWorkingSet);
                performanceChart_main.AddPerformance(PerformanceChart.PerformanceArg.FPS, tFPS);
                performanceChart_main.AddPerformance(PerformanceChart.PerformanceArg.Objects, tObjects);
                performanceChart_main.AddPerformance(PerformanceChart.PerformanceArg.FrameTime, tFrameTime);
                performanceChart_main.AddPerformance(PerformanceChart.PerformanceArg.RenderTime, tRenderTime);

                toolStripStatusLabel_status.Text = String.Format("运行中 - {0}", _Target.Lifetime.ToString("g"));
            }
        }
    }
}
