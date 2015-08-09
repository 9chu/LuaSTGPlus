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
        class ResourceListViewSorter : System.Collections.IComparer
        {
            public bool Descending { get; set; }

            public int Column { get; set; }
            
            public int Compare(object x, object y)
            {
                int tempInt;

                ListViewItem lx = (ListViewItem)x;
                ListViewItem ly = (ListViewItem)y;

                if (Column == 3)
                {
                    double dx = Convert.ToDouble(lx.SubItems[Column].Text);
                    double dy = Convert.ToDouble(ly.SubItems[Column].Text);
                    if (dx < dy)
                        tempInt = -1;
                    else if (dx == dy)
                        tempInt = 0;
                    else
                        tempInt = 1;
                }
                else
                    tempInt = String.Compare(lx.SubItems[Column].Text, ly.SubItems[Column].Text);

                if (Descending)
                    return -tempInt;
                else
                    return tempInt;
            }

            public ResourceListViewSorter()
            {
                Column = 0;
            }

            public ResourceListViewSorter(int column)
            {
                Column = column;
                Descending = true;
            }
        }
        
        struct PerformanceData
        {
            public float CPUTime;
            public float WorkingSet;
            public float FPS;
            public float Objects;
            public float FrameTime;
            public float RenderTime;
        }

        TargetProgram _Target;
        bool _CloseAfterExited = false;

        ListView[] _ResourceStatusListView;

        List<PerformanceData> _PerformanceDataRecorder = new List<PerformanceData>();

        public MainForm()
        {
            InitializeComponent();

            _ResourceStatusListView = new ListView[] {
                listView_texture,
                listView_image,
                listView_animation,
                listView_music,
                listView_soundeffect,
                listView_particle,
                listView_texturedfont,
                listView_ttffont,
                listView_shader
            };

            try
            {
                _Target = new TargetProgram(this);
            }
            catch (Exception ex)
            {
                MessageBox.Show("无法创建性能监测。\n\n错误：" + ex.Message, "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Application.Exit();
            }
            
            // 绑定事件
            _Target.OnProcessTrace += Target_OnProcessTrace;
            _Target.OnProcessExit += Target_OnProcessExit;
            _Target.OnResourceLoaded += Target_OnResourceLoaded;
            _Target.OnResourceRemoved += Target_OnResourceRemoved;
            _Target.OnResourceCleared += Target_OnResourceCleared;
        }
        
        void clearListView()
        {
            foreach (ListViewItem item in listView_resourceCounter.Items)
            {
                item.SubItems[1].Text = "0";
                item.SubItems[2].Text = "0";
            }

            foreach (ListView view in _ResourceStatusListView)
            {
                view.Items.Clear();
            }
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
            clearListView();
            _PerformanceDataRecorder.Clear();

            try
            {
                _Target.Start(toolStripTextBox_path.Text, Path.GetDirectoryName(toolStripTextBox_path.Text));
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
            
            // 启动计时器
            timer_main.Enabled = true;
        }

        void Target_OnResourceCleared(TargetProgram.ResourcePoolType pool)
        {
            string tPoolName = pool == TargetProgram.ResourcePoolType.Global ? "全局" : "关卡";
            
            for (int i = 0; i < _ResourceStatusListView.Count(); ++i)
            {
                List<ListViewItem> tItemsToRemove = new List<ListViewItem>();
                foreach (ListViewItem item in _ResourceStatusListView[i].Items)
                {
                    if (item.SubItems[0].Text == tPoolName)
                        tItemsToRemove.Add(item);
                }

                ListViewItem tCounter = listView_resourceCounter.Items[i];
                double tTotalTime = 0;
                int tTotalCount = 0;
                foreach (ListViewItem item in tItemsToRemove)
                {
                    tTotalTime += Convert.ToDouble(item.SubItems[3].Text);
                    ++tTotalCount;
                    _ResourceStatusListView[i].Items.Remove(item);
                }
                double tNewTime = Math.Max(0, Convert.ToDouble(tCounter.SubItems[1].Text) - tTotalTime);
                if (tNewTime < 0.00001)
                    tNewTime = 0;
                tCounter.SubItems[1].Text = tNewTime.ToString();
                tCounter.SubItems[2].Text = (Convert.ToInt32(tCounter.SubItems[2].Text) - tTotalCount).ToString();
            }
        }

        void Target_OnResourceRemoved(TargetProgram.ResourceType type, TargetProgram.ResourcePoolType pool, string name)
        {
            if ((int)type < 1 || (int)type > _ResourceStatusListView.Count())
                return;

            string tPoolName = pool == TargetProgram.ResourcePoolType.Global ? "全局" : "关卡";
            List<ListViewItem> tItemsToRemove = new List<ListViewItem>();
            foreach (ListViewItem item in _ResourceStatusListView[(int)type - 1].Items)
            {
                if (item.SubItems[0].Text == tPoolName && item.SubItems[1].Text == name)
                    tItemsToRemove.Add(item);
            }

            ListViewItem tCounter = listView_resourceCounter.Items[(int)type - 1];
            double tTotalTime = 0;
            int tTotalCount = 0;
            foreach (ListViewItem item in tItemsToRemove)
            {
                tTotalTime += Convert.ToDouble(item.SubItems[3].Text);
                ++tTotalCount;
                _ResourceStatusListView[(int)type - 1].Items.Remove(item);
            }
            double tNewTime = Math.Max(0, Convert.ToDouble(tCounter.SubItems[1].Text) - tTotalTime);
            if (tNewTime < 0.00001)
                tNewTime = 0;
            tCounter.SubItems[1].Text = tNewTime.ToString();
            tCounter.SubItems[2].Text = (Convert.ToInt32(tCounter.SubItems[2].Text) - tTotalCount).ToString();
        }

        void Target_OnResourceLoaded(TargetProgram.ResourceType type, TargetProgram.ResourcePoolType pool, string name, string path, float time)
        {
            if ((int)type < 1 || (int)type > _ResourceStatusListView.Count())
                return;

            ListViewItem tNewItem = new ListViewItem(pool == TargetProgram.ResourcePoolType.Global ? "全局" : "关卡");
            tNewItem.SubItems.Add(new ListViewItem.ListViewSubItem(tNewItem, name));
            tNewItem.SubItems.Add(new ListViewItem.ListViewSubItem(tNewItem, path));
            tNewItem.SubItems.Add(new ListViewItem.ListViewSubItem(tNewItem, time.ToString()));
            _ResourceStatusListView[(int)type - 1].Items.Add(tNewItem);

            ListViewItem tCounter = listView_resourceCounter.Items[(int)type - 1];
            tCounter.SubItems[1].Text = (Convert.ToDouble(tCounter.SubItems[1].Text) + time).ToString();
            tCounter.SubItems[2].Text = (Convert.ToInt32(tCounter.SubItems[2].Text) + 1).ToString();
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

        private void listView_resources_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            ListView listView = (ListView)sender;
            
            if (listView.ListViewItemSorter == null)
                listView.ListViewItemSorter = new ResourceListViewSorter(e.Column);
            else
            {
                ((ResourceListViewSorter)listView.ListViewItemSorter).Column = e.Column;
                ((ResourceListViewSorter)listView.ListViewItemSorter).Descending = !((ResourceListViewSorter)listView.ListViewItemSorter).Descending;
            }   

            listView.Sort();
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
                label_fps.Text = String.Format("FPS {0:N2}", tFPS);
                label_objects.Text = String.Format("对象 {0:N2}个", tObjects);
                label_frame.Text = String.Format("逻辑 {0:N2}ms", tFrameTime * 1000);
                label_render.Text = String.Format("渲染 {0:N2}ms", tRenderTime * 1000);

                performanceChart_main.AddPerformance(PerformanceChart.PerformanceArg.CPU, tCPUTime);
                performanceChart_main.AddPerformance(PerformanceChart.PerformanceArg.Memory, tWorkingSet);
                performanceChart_main.AddPerformance(PerformanceChart.PerformanceArg.FPS, tFPS);
                performanceChart_main.AddPerformance(PerformanceChart.PerformanceArg.Objects, tObjects);
                performanceChart_main.AddPerformance(PerformanceChart.PerformanceArg.FrameTime, tFrameTime);
                performanceChart_main.AddPerformance(PerformanceChart.PerformanceArg.RenderTime, tRenderTime);

                _PerformanceDataRecorder.Add(new PerformanceData
                {
                    CPUTime = tCPUTime,
                    WorkingSet = tWorkingSet / 1024 / 1024,
                    FPS = tFPS,
                    Objects = tObjects,
                    FrameTime = tFrameTime * 1000,
                    RenderTime = tRenderTime * 1000
                });

                toolStripStatusLabel_status.Text = String.Format("运行中 - {0}", _Target.Lifetime.ToString("g"));
            }
        }

        private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            _Target.CloseSocket();
        }

        private void ToolStripMenuItem_exportPerformanceData_Click(object sender, EventArgs e)
        {
            if (saveFileDialog_main.ShowDialog() != System.Windows.Forms.DialogResult.Cancel)
            {
                try
                {
                    using (FileStream fs = new FileStream(saveFileDialog_main.FileName, FileMode.Create, FileAccess.Write))
                    {
                        StringBuilder sb = new StringBuilder();
                        BinaryWriter wr = new BinaryWriter(fs, Encoding.Default);

                        sb.Append("CPU,内存,FPS,对象数,逻辑时间,渲染时间\n");
                        for (int i = 0; i < _PerformanceDataRecorder.Count; ++i)
                        {
                            sb.Append(_PerformanceDataRecorder[i].CPUTime);
                            sb.Append(",");
                            sb.Append(_PerformanceDataRecorder[i].WorkingSet);
                            sb.Append(",");
                            sb.Append(_PerformanceDataRecorder[i].FPS);
                            sb.Append(",");
                            sb.Append(_PerformanceDataRecorder[i].Objects);
                            sb.Append(",");
                            sb.Append(_PerformanceDataRecorder[i].FrameTime);
                            sb.Append(",");
                            sb.Append(_PerformanceDataRecorder[i].RenderTime);
                            sb.Append("\n");
                        }

                        wr.Write(sb.ToString());
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show("保存性能分析结果失败。\n\n错误：" + ex.Message, "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }
    }
}
