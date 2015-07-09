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
            this.toolStrip_main = new System.Windows.Forms.ToolStrip();
            this.toolStripLabel1 = new System.Windows.Forms.ToolStripLabel();
            this.toolStripTextBox_path = new System.Windows.Forms.ToolStripTextBox();
            this.statusStrip_main = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel_status = new System.Windows.Forms.ToolStripStatusLabel();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.tabControl2 = new System.Windows.Forms.TabControl();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.splitContainer2 = new System.Windows.Forms.SplitContainer();
            this.listView_log = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.imageList_main = new System.Windows.Forms.ImageList(this.components);
            this.textBox_log = new System.Windows.Forms.TextBox();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.openFileDialog_main = new System.Windows.Forms.OpenFileDialog();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.label_cpu = new System.Windows.Forms.Label();
            this.label_memory = new System.Windows.Forms.Label();
            this.label_fps = new System.Windows.Forms.Label();
            this.label_objects = new System.Windows.Forms.Label();
            this.label_frame = new System.Windows.Forms.Label();
            this.label_render = new System.Windows.Forms.Label();
            this.timer_main = new System.Windows.Forms.Timer(this.components);
            this.toolStripButton_selectApp = new System.Windows.Forms.ToolStripButton();
            this.toolStripButton_launch = new System.Windows.Forms.ToolStripButton();
            this.performanceChart_main = new PerformanceMonitor.PerformanceChart();
            this.toolStrip_main.SuspendLayout();
            this.statusStrip_main.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabControl2.SuspendLayout();
            this.tabPage3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
            this.splitContainer2.Panel1.SuspendLayout();
            this.splitContainer2.Panel2.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
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
            // tabPage2
            // 
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(698, 210);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "资源监控";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // tabControl2
            // 
            this.tabControl2.Controls.Add(this.tabPage3);
            this.tabControl2.Controls.Add(this.tabPage4);
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
            // imageList_main
            // 
            this.imageList_main.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList_main.ImageStream")));
            this.imageList_main.TransparentColor = System.Drawing.Color.Transparent;
            this.imageList_main.Images.SetKeyName(0, "error");
            this.imageList_main.Images.SetKeyName(1, "info");
            this.imageList_main.Images.SetKeyName(2, "warning");
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
            // tabPage4
            // 
            this.tabPage4.Location = new System.Drawing.Point(4, 22);
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage4.Size = new System.Drawing.Size(698, 133);
            this.tabPage4.TabIndex = 1;
            this.tabPage4.Text = "远程控制台";
            this.tabPage4.UseVisualStyleBackColor = true;
            // 
            // openFileDialog_main
            // 
            this.openFileDialog_main.DefaultExt = "exe";
            this.openFileDialog_main.FileName = "LuaSTGPlus.dev.exe";
            this.openFileDialog_main.Filter = "可执行程序(*.exe)|*.exe";
            this.openFileDialog_main.Title = "选择LuaSTGPlus主程序";
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
            // timer_main
            // 
            this.timer_main.Interval = 500;
            this.timer_main.Tick += new System.EventHandler(this.timer_main_Tick);
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
            // performanceChart_main
            // 
            this.performanceChart_main.BackColor = System.Drawing.Color.White;
            this.performanceChart_main.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
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
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "LuaSTGPlus 性能分析工具";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
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
            this.tabControl2.ResumeLayout(false);
            this.tabPage3.ResumeLayout(false);
            this.splitContainer2.Panel1.ResumeLayout(false);
            this.splitContainer2.Panel2.ResumeLayout(false);
            this.splitContainer2.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).EndInit();
            this.splitContainer2.ResumeLayout(false);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
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
        private System.Windows.Forms.TabPage tabPage4;
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
    }
}

