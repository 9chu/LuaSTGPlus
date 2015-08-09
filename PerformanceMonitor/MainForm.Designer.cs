namespace PerformanceMonitor
{
    partial class MainForm
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            System.Windows.Forms.ListViewItem listViewItem1 = new System.Windows.Forms.ListViewItem(new string[] {
            "纹理",
            "0",
            "0"}, -1);
            System.Windows.Forms.ListViewItem listViewItem2 = new System.Windows.Forms.ListViewItem(new string[] {
            "图像",
            "0",
            "0"}, -1);
            System.Windows.Forms.ListViewItem listViewItem3 = new System.Windows.Forms.ListViewItem(new string[] {
            "动画",
            "0",
            "0"}, -1);
            System.Windows.Forms.ListViewItem listViewItem4 = new System.Windows.Forms.ListViewItem(new string[] {
            "音乐",
            "0",
            "0"}, -1);
            System.Windows.Forms.ListViewItem listViewItem5 = new System.Windows.Forms.ListViewItem(new string[] {
            "音效",
            "0",
            "0"}, -1);
            System.Windows.Forms.ListViewItem listViewItem6 = new System.Windows.Forms.ListViewItem(new string[] {
            "粒子",
            "0",
            "0"}, -1);
            System.Windows.Forms.ListViewItem listViewItem7 = new System.Windows.Forms.ListViewItem(new string[] {
            "纹理化字体",
            "0",
            "0"}, -1);
            System.Windows.Forms.ListViewItem listViewItem8 = new System.Windows.Forms.ListViewItem(new string[] {
            "TTF字体",
            "0",
            "0"}, -1);
            System.Windows.Forms.ListViewItem listViewItem9 = new System.Windows.Forms.ListViewItem(new string[] {
            "Shader",
            "0",
            "0"}, -1);
            this.toolStrip_main = new System.Windows.Forms.ToolStrip();
            this.toolStripLabel1 = new System.Windows.Forms.ToolStripLabel();
            this.toolStripTextBox_path = new System.Windows.Forms.ToolStripTextBox();
            this.toolStripButton_selectApp = new System.Windows.Forms.ToolStripButton();
            this.toolStripButton_launch = new System.Windows.Forms.ToolStripButton();
            this.statusStrip_main = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel_status = new System.Windows.Forms.ToolStripStatusLabel();
            this.imageList_main = new System.Windows.Forms.ImageList(this.components);
            this.openFileDialog_main = new System.Windows.Forms.OpenFileDialog();
            this.timer_main = new System.Windows.Forms.Timer(this.components);
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.label_cpu = new System.Windows.Forms.Label();
            this.label_memory = new System.Windows.Forms.Label();
            this.label_fps = new System.Windows.Forms.Label();
            this.label_objects = new System.Windows.Forms.Label();
            this.label_frame = new System.Windows.Forms.Label();
            this.label_render = new System.Windows.Forms.Label();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.splitContainer3 = new System.Windows.Forms.SplitContainer();
            this.listView_resourceCounter = new System.Windows.Forms.ListView();
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabControl_res = new System.Windows.Forms.TabControl();
            this.tabPage5 = new System.Windows.Forms.TabPage();
            this.listView_texture = new System.Windows.Forms.ListView();
            this.columnHeader34 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader7 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader8 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader9 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPage6 = new System.Windows.Forms.TabPage();
            this.listView_image = new System.Windows.Forms.ListView();
            this.columnHeader10 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader11 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader12 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader13 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPage7 = new System.Windows.Forms.TabPage();
            this.listView_animation = new System.Windows.Forms.ListView();
            this.columnHeader14 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader15 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader16 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader17 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPage8 = new System.Windows.Forms.TabPage();
            this.listView_music = new System.Windows.Forms.ListView();
            this.columnHeader18 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader19 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader20 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader21 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPage9 = new System.Windows.Forms.TabPage();
            this.listView_soundeffect = new System.Windows.Forms.ListView();
            this.columnHeader22 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader23 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader24 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader25 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPage10 = new System.Windows.Forms.TabPage();
            this.listView_particle = new System.Windows.Forms.ListView();
            this.columnHeader26 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader27 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader28 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader29 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPage11 = new System.Windows.Forms.TabPage();
            this.listView_texturedfont = new System.Windows.Forms.ListView();
            this.columnHeader30 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader31 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader32 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader33 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPage12 = new System.Windows.Forms.TabPage();
            this.listView_ttffont = new System.Windows.Forms.ListView();
            this.columnHeader35 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader36 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader37 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader38 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabPage13 = new System.Windows.Forms.TabPage();
            this.listView_shader = new System.Windows.Forms.ListView();
            this.columnHeader39 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader40 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader41 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader42 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tabControl2 = new System.Windows.Forms.TabControl();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.splitContainer2 = new System.Windows.Forms.SplitContainer();
            this.listView_log = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.textBox_log = new System.Windows.Forms.TextBox();
            this.contextMenuStrip_performance = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.ToolStripMenuItem_exportPerformanceData = new System.Windows.Forms.ToolStripMenuItem();
            this.saveFileDialog_main = new System.Windows.Forms.SaveFileDialog();
            this.performanceChart_main = new PerformanceMonitor.PerformanceChart();
            this.toolStrip_main.SuspendLayout();
            this.statusStrip_main.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer3)).BeginInit();
            this.splitContainer3.Panel1.SuspendLayout();
            this.splitContainer3.Panel2.SuspendLayout();
            this.splitContainer3.SuspendLayout();
            this.tabControl_res.SuspendLayout();
            this.tabPage5.SuspendLayout();
            this.tabPage6.SuspendLayout();
            this.tabPage7.SuspendLayout();
            this.tabPage8.SuspendLayout();
            this.tabPage9.SuspendLayout();
            this.tabPage10.SuspendLayout();
            this.tabPage11.SuspendLayout();
            this.tabPage12.SuspendLayout();
            this.tabPage13.SuspendLayout();
            this.tabControl2.SuspendLayout();
            this.tabPage3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
            this.splitContainer2.Panel1.SuspendLayout();
            this.splitContainer2.Panel2.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            this.contextMenuStrip_performance.SuspendLayout();
            this.SuspendLayout();
            // 
            // toolStrip_main
            // 
            this.toolStrip_main.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripLabel1,
            this.toolStripTextBox_path,
            this.toolStripButton_selectApp,
            this.toolStripButton_launch});
            this.toolStrip_main.Location = new System.Drawing.Point(0, 0);
            this.toolStrip_main.Name = "toolStrip_main";
            this.toolStrip_main.Size = new System.Drawing.Size(706, 25);
            this.toolStrip_main.TabIndex = 0;
            this.toolStrip_main.Text = "toolStrip1";
            // 
            // toolStripLabel1
            // 
            this.toolStripLabel1.Name = "toolStripLabel1";
            this.toolStripLabel1.Size = new System.Drawing.Size(38, 22);
            this.toolStripLabel1.Text = "目标";
            // 
            // toolStripTextBox_path
            // 
            this.toolStripTextBox_path.Name = "toolStripTextBox_path";
            this.toolStripTextBox_path.Size = new System.Drawing.Size(250, 25);
            // 
            // toolStripButton_selectApp
            // 
            this.toolStripButton_selectApp.Image = global::PerformanceMonitor.Properties.Resources.search;
            this.toolStripButton_selectApp.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButton_selectApp.Name = "toolStripButton_selectApp";
            this.toolStripButton_selectApp.Size = new System.Drawing.Size(67, 22);
            this.toolStripButton_selectApp.Text = "浏览...";
            this.toolStripButton_selectApp.Click += new System.EventHandler(this.toolStripButton_selectApp_Click);
            // 
            // toolStripButton_launch
            // 
            this.toolStripButton_launch.Image = global::PerformanceMonitor.Properties.Resources.start;
            this.toolStripButton_launch.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButton_launch.Name = "toolStripButton_launch";
            this.toolStripButton_launch.Size = new System.Drawing.Size(58, 22);
            this.toolStripButton_launch.Text = "启动";
            this.toolStripButton_launch.Click += new System.EventHandler(this.toolStripButton_launch_Click);
            // 
            // statusStrip_main
            // 
            this.statusStrip_main.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel_status});
            this.statusStrip_main.Location = new System.Drawing.Point(0, 424);
            this.statusStrip_main.Name = "statusStrip_main";
            this.statusStrip_main.Size = new System.Drawing.Size(706, 22);
            this.statusStrip_main.TabIndex = 1;
            this.statusStrip_main.Text = "statusStrip1";
            // 
            // toolStripStatusLabel_status
            // 
            this.toolStripStatusLabel_status.Name = "toolStripStatusLabel_status";
            this.toolStripStatusLabel_status.Size = new System.Drawing.Size(38, 17);
            this.toolStripStatusLabel_status.Text = "就绪";
            // 
            // imageList_main
            // 
            this.imageList_main.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList_main.ImageStream")));
            this.imageList_main.TransparentColor = System.Drawing.Color.Transparent;
            this.imageList_main.Images.SetKeyName(0, "error");
            this.imageList_main.Images.SetKeyName(1, "info");
            this.imageList_main.Images.SetKeyName(2, "warning");
            // 
            // openFileDialog_main
            // 
            this.openFileDialog_main.DefaultExt = "exe";
            this.openFileDialog_main.FileName = "LuaSTGPlus.dev.exe";
            this.openFileDialog_main.Filter = "可执行程序(*.exe)|*.exe";
            this.openFileDialog_main.Title = "选择LuaSTGPlus主程序";
            // 
            // timer_main
            // 
            this.timer_main.Interval = 500;
            this.timer_main.Tick += new System.EventHandler(this.timer_main_Tick);
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 25);
            this.splitContainer1.Name = "splitContainer1";
            this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.tabControl1);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.tabControl2);
            this.splitContainer1.Size = new System.Drawing.Size(706, 399);
            this.splitContainer1.SplitterDistance = 236;
            this.splitContainer1.TabIndex = 2;
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(706, 236);
            this.tabControl1.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.performanceChart_main);
            this.tabPage1.Controls.Add(this.tableLayoutPanel1);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(698, 210);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "性能监控";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 6;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 16.66667F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 16.66667F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 16.66667F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 16.66667F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 16.66667F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 16.66667F));
            this.tableLayoutPanel1.Controls.Add(this.label_cpu, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.label_memory, 1, 0);
            this.tableLayoutPanel1.Controls.Add(this.label_fps, 2, 0);
            this.tableLayoutPanel1.Controls.Add(this.label_objects, 3, 0);
            this.tableLayoutPanel1.Controls.Add(this.label_frame, 4, 0);
            this.tableLayoutPanel1.Controls.Add(this.label_render, 5, 0);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 187);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 1;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(692, 20);
            this.tableLayoutPanel1.TabIndex = 1;
            // 
            // label_cpu
            // 
            this.label_cpu.AutoSize = true;
            this.label_cpu.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label_cpu.Font = new System.Drawing.Font("微软雅黑", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_cpu.ForeColor = System.Drawing.Color.LightSkyBlue;
            this.label_cpu.Location = new System.Drawing.Point(3, 0);
            this.label_cpu.Name = "label_cpu";
            this.label_cpu.Size = new System.Drawing.Size(109, 20);
            this.label_cpu.TabIndex = 0;
            this.label_cpu.Text = "CPU";
            // 
            // label_memory
            // 
            this.label_memory.AutoSize = true;
            this.label_memory.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label_memory.Font = new System.Drawing.Font("微软雅黑", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_memory.ForeColor = System.Drawing.Color.BlueViolet;
            this.label_memory.Location = new System.Drawing.Point(118, 0);
            this.label_memory.Name = "label_memory";
            this.label_memory.Size = new System.Drawing.Size(109, 20);
            this.label_memory.TabIndex = 1;
            this.label_memory.Text = "内存";
            // 
            // label_fps
            // 
            this.label_fps.AutoSize = true;
            this.label_fps.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label_fps.Font = new System.Drawing.Font("微软雅黑", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_fps.ForeColor = System.Drawing.Color.ForestGreen;
            this.label_fps.Location = new System.Drawing.Point(233, 0);
            this.label_fps.Name = "label_fps";
            this.label_fps.Size = new System.Drawing.Size(109, 20);
            this.label_fps.TabIndex = 2;
            this.label_fps.Text = "FPS";
            // 
            // label_objects
            // 
            this.label_objects.AutoSize = true;
            this.label_objects.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label_objects.Font = new System.Drawing.Font("微软雅黑", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_objects.ForeColor = System.Drawing.Color.Maroon;
            this.label_objects.Location = new System.Drawing.Point(348, 0);
            this.label_objects.Name = "label_objects";
            this.label_objects.Size = new System.Drawing.Size(109, 20);
            this.label_objects.TabIndex = 3;
            this.label_objects.Text = "对象";
            // 
            // label_frame
            // 
            this.label_frame.AutoSize = true;
            this.label_frame.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label_frame.Font = new System.Drawing.Font("微软雅黑", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_frame.ForeColor = System.Drawing.Color.LightSalmon;
            this.label_frame.Location = new System.Drawing.Point(463, 0);
            this.label_frame.Name = "label_frame";
            this.label_frame.Size = new System.Drawing.Size(109, 20);
            this.label_frame.TabIndex = 4;
            this.label_frame.Text = "逻辑";
            // 
            // label_render
            // 
            this.label_render.AutoSize = true;
            this.label_render.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label_render.Font = new System.Drawing.Font("微软雅黑", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_render.ForeColor = System.Drawing.Color.Peru;
            this.label_render.Location = new System.Drawing.Point(578, 0);
            this.label_render.Name = "label_render";
            this.label_render.Size = new System.Drawing.Size(111, 20);
            this.label_render.TabIndex = 5;
            this.label_render.Text = "渲染";
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.splitContainer3);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(698, 210);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "资源监控";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // splitContainer3
            // 
            this.splitContainer3.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer3.Location = new System.Drawing.Point(3, 3);
            this.splitContainer3.Name = "splitContainer3";
            // 
            // splitContainer3.Panel1
            // 
            this.splitContainer3.Panel1.Controls.Add(this.listView_resourceCounter);
            // 
            // splitContainer3.Panel2
            // 
            this.splitContainer3.Panel2.Controls.Add(this.tabControl_res);
            this.splitContainer3.Size = new System.Drawing.Size(692, 204);
            this.splitContainer3.SplitterDistance = 222;
            this.splitContainer3.TabIndex = 0;
            // 
            // listView_resourceCounter
            // 
            this.listView_resourceCounter.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader3,
            this.columnHeader5,
            this.columnHeader6});
            this.listView_resourceCounter.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_resourceCounter.FullRowSelect = true;
            this.listView_resourceCounter.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            listViewItem1,
            listViewItem2,
            listViewItem3,
            listViewItem4,
            listViewItem5,
            listViewItem6,
            listViewItem7,
            listViewItem8,
            listViewItem9});
            this.listView_resourceCounter.Location = new System.Drawing.Point(0, 0);
            this.listView_resourceCounter.MultiSelect = false;
            this.listView_resourceCounter.Name = "listView_resourceCounter";
            this.listView_resourceCounter.Size = new System.Drawing.Size(222, 204);
            this.listView_resourceCounter.TabIndex = 0;
            this.listView_resourceCounter.UseCompatibleStateImageBehavior = false;
            this.listView_resourceCounter.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "资源类型";
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "加载总计时";
            this.columnHeader5.Width = 80;
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "资源数量";
            // 
            // tabControl_res
            // 
            this.tabControl_res.Controls.Add(this.tabPage5);
            this.tabControl_res.Controls.Add(this.tabPage6);
            this.tabControl_res.Controls.Add(this.tabPage7);
            this.tabControl_res.Controls.Add(this.tabPage8);
            this.tabControl_res.Controls.Add(this.tabPage9);
            this.tabControl_res.Controls.Add(this.tabPage10);
            this.tabControl_res.Controls.Add(this.tabPage11);
            this.tabControl_res.Controls.Add(this.tabPage12);
            this.tabControl_res.Controls.Add(this.tabPage13);
            this.tabControl_res.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl_res.Location = new System.Drawing.Point(0, 0);
            this.tabControl_res.Name = "tabControl_res";
            this.tabControl_res.SelectedIndex = 0;
            this.tabControl_res.Size = new System.Drawing.Size(466, 204);
            this.tabControl_res.TabIndex = 0;
            // 
            // tabPage5
            // 
            this.tabPage5.Controls.Add(this.listView_texture);
            this.tabPage5.Location = new System.Drawing.Point(4, 22);
            this.tabPage5.Name = "tabPage5";
            this.tabPage5.Size = new System.Drawing.Size(458, 178);
            this.tabPage5.TabIndex = 0;
            this.tabPage5.Text = "纹理";
            this.tabPage5.UseVisualStyleBackColor = true;
            // 
            // listView_texture
            // 
            this.listView_texture.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader34,
            this.columnHeader7,
            this.columnHeader8,
            this.columnHeader9});
            this.listView_texture.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_texture.FullRowSelect = true;
            this.listView_texture.Location = new System.Drawing.Point(0, 0);
            this.listView_texture.Name = "listView_texture";
            this.listView_texture.Size = new System.Drawing.Size(458, 178);
            this.listView_texture.TabIndex = 1;
            this.listView_texture.UseCompatibleStateImageBehavior = false;
            this.listView_texture.View = System.Windows.Forms.View.Details;
            this.listView_texture.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.listView_resources_ColumnClick);
            // 
            // columnHeader34
            // 
            this.columnHeader34.Text = "池";
            this.columnHeader34.Width = 40;
            // 
            // columnHeader7
            // 
            this.columnHeader7.Text = "名称";
            this.columnHeader7.Width = 125;
            // 
            // columnHeader8
            // 
            this.columnHeader8.Text = "路径";
            this.columnHeader8.Width = 200;
            // 
            // columnHeader9
            // 
            this.columnHeader9.Text = "装载耗时";
            this.columnHeader9.Width = 80;
            // 
            // tabPage6
            // 
            this.tabPage6.Controls.Add(this.listView_image);
            this.tabPage6.Location = new System.Drawing.Point(4, 22);
            this.tabPage6.Name = "tabPage6";
            this.tabPage6.Size = new System.Drawing.Size(458, 178);
            this.tabPage6.TabIndex = 1;
            this.tabPage6.Text = "图像";
            this.tabPage6.UseVisualStyleBackColor = true;
            // 
            // listView_image
            // 
            this.listView_image.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader10,
            this.columnHeader11,
            this.columnHeader12,
            this.columnHeader13});
            this.listView_image.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_image.FullRowSelect = true;
            this.listView_image.Location = new System.Drawing.Point(0, 0);
            this.listView_image.Name = "listView_image";
            this.listView_image.Size = new System.Drawing.Size(458, 178);
            this.listView_image.TabIndex = 1;
            this.listView_image.UseCompatibleStateImageBehavior = false;
            this.listView_image.View = System.Windows.Forms.View.Details;
            this.listView_image.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.listView_resources_ColumnClick);
            // 
            // columnHeader10
            // 
            this.columnHeader10.Text = "池";
            this.columnHeader10.Width = 40;
            // 
            // columnHeader11
            // 
            this.columnHeader11.Text = "名称";
            this.columnHeader11.Width = 125;
            // 
            // columnHeader12
            // 
            this.columnHeader12.Text = "路径";
            this.columnHeader12.Width = 200;
            // 
            // columnHeader13
            // 
            this.columnHeader13.Text = "装载耗时";
            this.columnHeader13.Width = 80;
            // 
            // tabPage7
            // 
            this.tabPage7.Controls.Add(this.listView_animation);
            this.tabPage7.Location = new System.Drawing.Point(4, 22);
            this.tabPage7.Name = "tabPage7";
            this.tabPage7.Size = new System.Drawing.Size(458, 178);
            this.tabPage7.TabIndex = 2;
            this.tabPage7.Text = "动画";
            this.tabPage7.UseVisualStyleBackColor = true;
            // 
            // listView_animation
            // 
            this.listView_animation.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader14,
            this.columnHeader15,
            this.columnHeader16,
            this.columnHeader17});
            this.listView_animation.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_animation.FullRowSelect = true;
            this.listView_animation.Location = new System.Drawing.Point(0, 0);
            this.listView_animation.Name = "listView_animation";
            this.listView_animation.Size = new System.Drawing.Size(458, 178);
            this.listView_animation.TabIndex = 1;
            this.listView_animation.UseCompatibleStateImageBehavior = false;
            this.listView_animation.View = System.Windows.Forms.View.Details;
            this.listView_animation.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.listView_resources_ColumnClick);
            // 
            // columnHeader14
            // 
            this.columnHeader14.Text = "池";
            this.columnHeader14.Width = 40;
            // 
            // columnHeader15
            // 
            this.columnHeader15.Text = "名称";
            this.columnHeader15.Width = 125;
            // 
            // columnHeader16
            // 
            this.columnHeader16.Text = "路径";
            this.columnHeader16.Width = 200;
            // 
            // columnHeader17
            // 
            this.columnHeader17.Text = "装载耗时";
            this.columnHeader17.Width = 80;
            // 
            // tabPage8
            // 
            this.tabPage8.Controls.Add(this.listView_music);
            this.tabPage8.Location = new System.Drawing.Point(4, 22);
            this.tabPage8.Name = "tabPage8";
            this.tabPage8.Size = new System.Drawing.Size(458, 178);
            this.tabPage8.TabIndex = 3;
            this.tabPage8.Text = "音乐";
            this.tabPage8.UseVisualStyleBackColor = true;
            // 
            // listView_music
            // 
            this.listView_music.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader18,
            this.columnHeader19,
            this.columnHeader20,
            this.columnHeader21});
            this.listView_music.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_music.FullRowSelect = true;
            this.listView_music.Location = new System.Drawing.Point(0, 0);
            this.listView_music.Name = "listView_music";
            this.listView_music.Size = new System.Drawing.Size(458, 178);
            this.listView_music.TabIndex = 1;
            this.listView_music.UseCompatibleStateImageBehavior = false;
            this.listView_music.View = System.Windows.Forms.View.Details;
            this.listView_music.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.listView_resources_ColumnClick);
            // 
            // columnHeader18
            // 
            this.columnHeader18.Text = "池";
            this.columnHeader18.Width = 40;
            // 
            // columnHeader19
            // 
            this.columnHeader19.Text = "名称";
            this.columnHeader19.Width = 125;
            // 
            // columnHeader20
            // 
            this.columnHeader20.Text = "路径";
            this.columnHeader20.Width = 200;
            // 
            // columnHeader21
            // 
            this.columnHeader21.Text = "装载耗时";
            this.columnHeader21.Width = 80;
            // 
            // tabPage9
            // 
            this.tabPage9.Controls.Add(this.listView_soundeffect);
            this.tabPage9.Location = new System.Drawing.Point(4, 22);
            this.tabPage9.Name = "tabPage9";
            this.tabPage9.Size = new System.Drawing.Size(458, 178);
            this.tabPage9.TabIndex = 4;
            this.tabPage9.Text = "音效";
            this.tabPage9.UseVisualStyleBackColor = true;
            // 
            // listView_soundeffect
            // 
            this.listView_soundeffect.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader22,
            this.columnHeader23,
            this.columnHeader24,
            this.columnHeader25});
            this.listView_soundeffect.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_soundeffect.FullRowSelect = true;
            this.listView_soundeffect.Location = new System.Drawing.Point(0, 0);
            this.listView_soundeffect.Name = "listView_soundeffect";
            this.listView_soundeffect.Size = new System.Drawing.Size(458, 178);
            this.listView_soundeffect.TabIndex = 1;
            this.listView_soundeffect.UseCompatibleStateImageBehavior = false;
            this.listView_soundeffect.View = System.Windows.Forms.View.Details;
            this.listView_soundeffect.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.listView_resources_ColumnClick);
            // 
            // columnHeader22
            // 
            this.columnHeader22.Text = "池";
            this.columnHeader22.Width = 40;
            // 
            // columnHeader23
            // 
            this.columnHeader23.Text = "名称";
            this.columnHeader23.Width = 125;
            // 
            // columnHeader24
            // 
            this.columnHeader24.Text = "路径";
            this.columnHeader24.Width = 200;
            // 
            // columnHeader25
            // 
            this.columnHeader25.Text = "装载耗时";
            this.columnHeader25.Width = 80;
            // 
            // tabPage10
            // 
            this.tabPage10.Controls.Add(this.listView_particle);
            this.tabPage10.Location = new System.Drawing.Point(4, 22);
            this.tabPage10.Name = "tabPage10";
            this.tabPage10.Size = new System.Drawing.Size(458, 178);
            this.tabPage10.TabIndex = 5;
            this.tabPage10.Text = "粒子";
            this.tabPage10.UseVisualStyleBackColor = true;
            // 
            // listView_particle
            // 
            this.listView_particle.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader26,
            this.columnHeader27,
            this.columnHeader28,
            this.columnHeader29});
            this.listView_particle.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_particle.FullRowSelect = true;
            this.listView_particle.Location = new System.Drawing.Point(0, 0);
            this.listView_particle.Name = "listView_particle";
            this.listView_particle.Size = new System.Drawing.Size(458, 178);
            this.listView_particle.TabIndex = 1;
            this.listView_particle.UseCompatibleStateImageBehavior = false;
            this.listView_particle.View = System.Windows.Forms.View.Details;
            this.listView_particle.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.listView_resources_ColumnClick);
            // 
            // columnHeader26
            // 
            this.columnHeader26.Text = "池";
            this.columnHeader26.Width = 40;
            // 
            // columnHeader27
            // 
            this.columnHeader27.Text = "名称";
            this.columnHeader27.Width = 125;
            // 
            // columnHeader28
            // 
            this.columnHeader28.Text = "路径";
            this.columnHeader28.Width = 200;
            // 
            // columnHeader29
            // 
            this.columnHeader29.Text = "装载耗时";
            this.columnHeader29.Width = 80;
            // 
            // tabPage11
            // 
            this.tabPage11.Controls.Add(this.listView_texturedfont);
            this.tabPage11.Location = new System.Drawing.Point(4, 22);
            this.tabPage11.Name = "tabPage11";
            this.tabPage11.Size = new System.Drawing.Size(458, 178);
            this.tabPage11.TabIndex = 6;
            this.tabPage11.Text = "纹理化字体";
            this.tabPage11.UseVisualStyleBackColor = true;
            // 
            // listView_texturedfont
            // 
            this.listView_texturedfont.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader30,
            this.columnHeader31,
            this.columnHeader32,
            this.columnHeader33});
            this.listView_texturedfont.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_texturedfont.FullRowSelect = true;
            this.listView_texturedfont.Location = new System.Drawing.Point(0, 0);
            this.listView_texturedfont.Name = "listView_texturedfont";
            this.listView_texturedfont.Size = new System.Drawing.Size(458, 178);
            this.listView_texturedfont.TabIndex = 1;
            this.listView_texturedfont.UseCompatibleStateImageBehavior = false;
            this.listView_texturedfont.View = System.Windows.Forms.View.Details;
            this.listView_texturedfont.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.listView_resources_ColumnClick);
            // 
            // columnHeader30
            // 
            this.columnHeader30.Text = "池";
            this.columnHeader30.Width = 40;
            // 
            // columnHeader31
            // 
            this.columnHeader31.Text = "名称";
            this.columnHeader31.Width = 125;
            // 
            // columnHeader32
            // 
            this.columnHeader32.Text = "路径";
            this.columnHeader32.Width = 200;
            // 
            // columnHeader33
            // 
            this.columnHeader33.Text = "装载耗时";
            this.columnHeader33.Width = 80;
            // 
            // tabPage12
            // 
            this.tabPage12.Controls.Add(this.listView_ttffont);
            this.tabPage12.Location = new System.Drawing.Point(4, 22);
            this.tabPage12.Name = "tabPage12";
            this.tabPage12.Size = new System.Drawing.Size(458, 178);
            this.tabPage12.TabIndex = 7;
            this.tabPage12.Text = "TTF字体";
            this.tabPage12.UseVisualStyleBackColor = true;
            // 
            // listView_ttffont
            // 
            this.listView_ttffont.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader35,
            this.columnHeader36,
            this.columnHeader37,
            this.columnHeader38});
            this.listView_ttffont.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_ttffont.FullRowSelect = true;
            this.listView_ttffont.Location = new System.Drawing.Point(0, 0);
            this.listView_ttffont.Name = "listView_ttffont";
            this.listView_ttffont.Size = new System.Drawing.Size(458, 178);
            this.listView_ttffont.TabIndex = 1;
            this.listView_ttffont.UseCompatibleStateImageBehavior = false;
            this.listView_ttffont.View = System.Windows.Forms.View.Details;
            this.listView_ttffont.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.listView_resources_ColumnClick);
            // 
            // columnHeader35
            // 
            this.columnHeader35.Text = "池";
            this.columnHeader35.Width = 40;
            // 
            // columnHeader36
            // 
            this.columnHeader36.Text = "名称";
            this.columnHeader36.Width = 125;
            // 
            // columnHeader37
            // 
            this.columnHeader37.Text = "路径";
            this.columnHeader37.Width = 200;
            // 
            // columnHeader38
            // 
            this.columnHeader38.Text = "装载耗时";
            this.columnHeader38.Width = 80;
            // 
            // tabPage13
            // 
            this.tabPage13.Controls.Add(this.listView_shader);
            this.tabPage13.Location = new System.Drawing.Point(4, 22);
            this.tabPage13.Name = "tabPage13";
            this.tabPage13.Size = new System.Drawing.Size(458, 178);
            this.tabPage13.TabIndex = 8;
            this.tabPage13.Text = "Shader";
            this.tabPage13.UseVisualStyleBackColor = true;
            // 
            // listView_shader
            // 
            this.listView_shader.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader39,
            this.columnHeader40,
            this.columnHeader41,
            this.columnHeader42});
            this.listView_shader.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_shader.FullRowSelect = true;
            this.listView_shader.Location = new System.Drawing.Point(0, 0);
            this.listView_shader.Name = "listView_shader";
            this.listView_shader.Size = new System.Drawing.Size(458, 178);
            this.listView_shader.TabIndex = 1;
            this.listView_shader.UseCompatibleStateImageBehavior = false;
            this.listView_shader.View = System.Windows.Forms.View.Details;
            this.listView_shader.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.listView_resources_ColumnClick);
            // 
            // columnHeader39
            // 
            this.columnHeader39.Text = "池";
            this.columnHeader39.Width = 40;
            // 
            // columnHeader40
            // 
            this.columnHeader40.Text = "名称";
            this.columnHeader40.Width = 125;
            // 
            // columnHeader41
            // 
            this.columnHeader41.Text = "路径";
            this.columnHeader41.Width = 200;
            // 
            // columnHeader42
            // 
            this.columnHeader42.Text = "装载耗时";
            this.columnHeader42.Width = 80;
            // 
            // tabControl2
            // 
            this.tabControl2.Controls.Add(this.tabPage3);
            this.tabControl2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl2.Location = new System.Drawing.Point(0, 0);
            this.tabControl2.Name = "tabControl2";
            this.tabControl2.SelectedIndex = 0;
            this.tabControl2.Size = new System.Drawing.Size(706, 159);
            this.tabControl2.TabIndex = 0;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.splitContainer2);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(698, 133);
            this.tabPage3.TabIndex = 0;
            this.tabPage3.Text = "调试输出";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // splitContainer2
            // 
            this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer2.Location = new System.Drawing.Point(3, 3);
            this.splitContainer2.Name = "splitContainer2";
            this.splitContainer2.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer2.Panel1
            // 
            this.splitContainer2.Panel1.Controls.Add(this.listView_log);
            // 
            // splitContainer2.Panel2
            // 
            this.splitContainer2.Panel2.Controls.Add(this.textBox_log);
            this.splitContainer2.Size = new System.Drawing.Size(692, 127);
            this.splitContainer2.SplitterDistance = 94;
            this.splitContainer2.TabIndex = 0;
            // 
            // listView_log
            // 
            this.listView_log.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader4});
            this.listView_log.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_log.FullRowSelect = true;
            this.listView_log.GridLines = true;
            this.listView_log.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.listView_log.Location = new System.Drawing.Point(0, 0);
            this.listView_log.Name = "listView_log";
            this.listView_log.Size = new System.Drawing.Size(692, 94);
            this.listView_log.SmallImageList = this.imageList_main;
            this.listView_log.TabIndex = 0;
            this.listView_log.UseCompatibleStateImageBehavior = false;
            this.listView_log.View = System.Windows.Forms.View.Details;
            this.listView_log.SelectedIndexChanged += new System.EventHandler(this.listView_log_SelectedIndexChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "";
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "时间";
            this.columnHeader2.Width = 100;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "内容";
            this.columnHeader4.Width = 420;
            // 
            // textBox_log
            // 
            this.textBox_log.Dock = System.Windows.Forms.DockStyle.Fill;
            this.textBox_log.Location = new System.Drawing.Point(0, 0);
            this.textBox_log.Multiline = true;
            this.textBox_log.Name = "textBox_log";
            this.textBox_log.ReadOnly = true;
            this.textBox_log.Size = new System.Drawing.Size(692, 29);
            this.textBox_log.TabIndex = 0;
            // 
            // contextMenuStrip_performance
            // 
            this.contextMenuStrip_performance.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolStripMenuItem_exportPerformanceData});
            this.contextMenuStrip_performance.Name = "contextMenuStrip_performance";
            this.contextMenuStrip_performance.Size = new System.Drawing.Size(167, 26);
            // 
            // ToolStripMenuItem_exportPerformanceData
            // 
            this.ToolStripMenuItem_exportPerformanceData.Name = "ToolStripMenuItem_exportPerformanceData";
            this.ToolStripMenuItem_exportPerformanceData.Size = new System.Drawing.Size(166, 22);
            this.ToolStripMenuItem_exportPerformanceData.Text = "导出分析数据";
            this.ToolStripMenuItem_exportPerformanceData.Click += new System.EventHandler(this.ToolStripMenuItem_exportPerformanceData_Click);
            // 
            // saveFileDialog_main
            // 
            this.saveFileDialog_main.DefaultExt = "csv";
            this.saveFileDialog_main.Filter = "逗号分割文件(*.csv)|*.csv";
            this.saveFileDialog_main.Title = "保存性能数据";
            // 
            // performanceChart_main
            // 
            this.performanceChart_main.BackColor = System.Drawing.Color.White;
            this.performanceChart_main.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.performanceChart_main.ContextMenuStrip = this.contextMenuStrip_performance;
            this.performanceChart_main.Dock = System.Windows.Forms.DockStyle.Fill;
            this.performanceChart_main.Location = new System.Drawing.Point(3, 3);
            this.performanceChart_main.Name = "performanceChart_main";
            this.performanceChart_main.Size = new System.Drawing.Size(692, 184);
            this.performanceChart_main.TabIndex = 0;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(706, 446);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.statusStrip_main);
            this.Controls.Add(this.toolStrip_main);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "LuaSTGPlus 性能分析工具";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MainForm_FormClosed);
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.toolStrip_main.ResumeLayout(false);
            this.toolStrip_main.PerformLayout();
            this.statusStrip_main.ResumeLayout(false);
            this.statusStrip_main.PerformLayout();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            this.splitContainer3.Panel1.ResumeLayout(false);
            this.splitContainer3.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer3)).EndInit();
            this.splitContainer3.ResumeLayout(false);
            this.tabControl_res.ResumeLayout(false);
            this.tabPage5.ResumeLayout(false);
            this.tabPage6.ResumeLayout(false);
            this.tabPage7.ResumeLayout(false);
            this.tabPage8.ResumeLayout(false);
            this.tabPage9.ResumeLayout(false);
            this.tabPage10.ResumeLayout(false);
            this.tabPage11.ResumeLayout(false);
            this.tabPage12.ResumeLayout(false);
            this.tabPage13.ResumeLayout(false);
            this.tabControl2.ResumeLayout(false);
            this.tabPage3.ResumeLayout(false);
            this.splitContainer2.Panel1.ResumeLayout(false);
            this.splitContainer2.Panel2.ResumeLayout(false);
            this.splitContainer2.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).EndInit();
            this.splitContainer2.ResumeLayout(false);
            this.contextMenuStrip_performance.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ToolStrip toolStrip_main;
        private System.Windows.Forms.ToolStripLabel toolStripLabel1;
        private System.Windows.Forms.ToolStripTextBox toolStripTextBox_path;
        private System.Windows.Forms.ToolStripButton toolStripButton_selectApp;
        private System.Windows.Forms.ToolStripButton toolStripButton_launch;
        private System.Windows.Forms.StatusStrip statusStrip_main;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel_status;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.TabControl tabControl2;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.OpenFileDialog openFileDialog_main;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private System.Windows.Forms.ListView listView_log;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.TextBox textBox_log;
        private System.Windows.Forms.ImageList imageList_main;
        private PerformanceChart performanceChart_main;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.Label label_cpu;
        private System.Windows.Forms.Label label_memory;
        private System.Windows.Forms.Label label_fps;
        private System.Windows.Forms.Label label_objects;
        private System.Windows.Forms.Label label_frame;
        private System.Windows.Forms.Label label_render;
        private System.Windows.Forms.Timer timer_main;
        private System.Windows.Forms.SplitContainer splitContainer3;
        private System.Windows.Forms.ListView listView_resourceCounter;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.ColumnHeader columnHeader6;
        private System.Windows.Forms.TabControl tabControl_res;
        private System.Windows.Forms.TabPage tabPage6;
        private System.Windows.Forms.TabPage tabPage7;
        private System.Windows.Forms.TabPage tabPage8;
        private System.Windows.Forms.TabPage tabPage9;
        private System.Windows.Forms.TabPage tabPage10;
        private System.Windows.Forms.TabPage tabPage11;
        private System.Windows.Forms.TabPage tabPage12;
        private System.Windows.Forms.TabPage tabPage13;
        private System.Windows.Forms.ListView listView_image;
        private System.Windows.Forms.ColumnHeader columnHeader10;
        private System.Windows.Forms.ColumnHeader columnHeader11;
        private System.Windows.Forms.ColumnHeader columnHeader12;
        private System.Windows.Forms.ColumnHeader columnHeader13;
        private System.Windows.Forms.ListView listView_animation;
        private System.Windows.Forms.ColumnHeader columnHeader14;
        private System.Windows.Forms.ColumnHeader columnHeader15;
        private System.Windows.Forms.ColumnHeader columnHeader16;
        private System.Windows.Forms.ColumnHeader columnHeader17;
        private System.Windows.Forms.ListView listView_music;
        private System.Windows.Forms.ColumnHeader columnHeader18;
        private System.Windows.Forms.ColumnHeader columnHeader19;
        private System.Windows.Forms.ColumnHeader columnHeader20;
        private System.Windows.Forms.ColumnHeader columnHeader21;
        private System.Windows.Forms.ListView listView_soundeffect;
        private System.Windows.Forms.ColumnHeader columnHeader22;
        private System.Windows.Forms.ColumnHeader columnHeader23;
        private System.Windows.Forms.ColumnHeader columnHeader24;
        private System.Windows.Forms.ColumnHeader columnHeader25;
        private System.Windows.Forms.ListView listView_particle;
        private System.Windows.Forms.ColumnHeader columnHeader26;
        private System.Windows.Forms.ColumnHeader columnHeader27;
        private System.Windows.Forms.ColumnHeader columnHeader28;
        private System.Windows.Forms.ColumnHeader columnHeader29;
        private System.Windows.Forms.ListView listView_texturedfont;
        private System.Windows.Forms.ColumnHeader columnHeader30;
        private System.Windows.Forms.ColumnHeader columnHeader31;
        private System.Windows.Forms.ColumnHeader columnHeader32;
        private System.Windows.Forms.ColumnHeader columnHeader33;
        private System.Windows.Forms.ListView listView_ttffont;
        private System.Windows.Forms.ColumnHeader columnHeader35;
        private System.Windows.Forms.ColumnHeader columnHeader36;
        private System.Windows.Forms.ColumnHeader columnHeader37;
        private System.Windows.Forms.ColumnHeader columnHeader38;
        private System.Windows.Forms.ListView listView_shader;
        private System.Windows.Forms.ColumnHeader columnHeader39;
        private System.Windows.Forms.ColumnHeader columnHeader40;
        private System.Windows.Forms.ColumnHeader columnHeader41;
        private System.Windows.Forms.ColumnHeader columnHeader42;
        private System.Windows.Forms.TabPage tabPage5;
        private System.Windows.Forms.ListView listView_texture;
        private System.Windows.Forms.ColumnHeader columnHeader34;
        private System.Windows.Forms.ColumnHeader columnHeader7;
        private System.Windows.Forms.ColumnHeader columnHeader8;
        private System.Windows.Forms.ColumnHeader columnHeader9;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip_performance;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_exportPerformanceData;
        private System.Windows.Forms.SaveFileDialog saveFileDialog_main;
    }
}

