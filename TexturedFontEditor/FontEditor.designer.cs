namespace TexturedFontEditor
{
    partial class FontEditor
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FontEditor));
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.文件FToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_new = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_open = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_save = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_saveas = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
            this.ToolStripMenuItem_export = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_exporttex = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
            this.ToolStripMenuItem_exportLayout = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_paste = new System.Windows.Forms.ToolStripMenuItem();
            this.填充FToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_fromText = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripSeparator();
            this.ToolStripMenuItem_setfont = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_setcolor = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_setmargin = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_setadv = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_setbaseline = new System.Windows.Forms.ToolStripMenuItem();
            this.选项OToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.背景颜色BToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_bk_white = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_bk_black = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_showfont = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_showmargin = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.设置纹理大小SToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_256x256 = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_512x512 = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_1024x1024 = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_2048x2048 = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_usersize = new System.Windows.Forms.ToolStripMenuItem();
            this.抗锯齿AToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_AA_Default = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_AA_AA = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_AA_AAGF = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_AA_SBPP = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_AA_SBPPGF = new System.Windows.Forms.ToolStripMenuItem();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.listView_charmap = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.toolStripLabel1 = new System.Windows.Forms.ToolStripLabel();
            this.splitContainer2 = new System.Windows.Forms.SplitContainer();
            this.propertyGrid_char = new System.Windows.Forms.PropertyGrid();
            this.saveFileDialog_def = new System.Windows.Forms.SaveFileDialog();
            this.openFileDialog_def = new System.Windows.Forms.OpenFileDialog();
            this.saveFileDialog_savepng = new System.Windows.Forms.SaveFileDialog();
            this.fontDialog_main = new System.Windows.Forms.FontDialog();
            this.colorDialog_main = new System.Windows.Forms.ColorDialog();
            this.saveFileDialog_savexml = new System.Windows.Forms.SaveFileDialog();
            this.toolStripButton_add = new System.Windows.Forms.ToolStripButton();
            this.toolStripButton_remove = new System.Windows.Forms.ToolStripButton();
            this.toolStripButton_up = new System.Windows.Forms.ToolStripButton();
            this.toolStripButton_down = new System.Windows.Forms.ToolStripButton();
            this.pictureBox_tex = new System.Windows.Forms.PictureBox();
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
            this.splitContainer2.Panel1.SuspendLayout();
            this.splitContainer2.Panel2.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox_tex)).BeginInit();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.文件FToolStripMenuItem,
            this.填充FToolStripMenuItem,
            this.选项OToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(771, 25);
            this.menuStrip1.TabIndex = 0;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // 文件FToolStripMenuItem
            // 
            this.文件FToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolStripMenuItem_new,
            this.ToolStripMenuItem_open,
            this.ToolStripMenuItem_save,
            this.ToolStripMenuItem_saveas,
            this.toolStripMenuItem3,
            this.ToolStripMenuItem_export,
            this.ToolStripMenuItem_exporttex,
            this.toolStripMenuItem2,
            this.ToolStripMenuItem_exportLayout,
            this.ToolStripMenuItem_paste});
            this.文件FToolStripMenuItem.Name = "文件FToolStripMenuItem";
            this.文件FToolStripMenuItem.Size = new System.Drawing.Size(64, 21);
            this.文件FToolStripMenuItem.Text = "文件(&F)";
            // 
            // ToolStripMenuItem_new
            // 
            this.ToolStripMenuItem_new.Name = "ToolStripMenuItem_new";
            this.ToolStripMenuItem_new.Size = new System.Drawing.Size(205, 22);
            this.ToolStripMenuItem_new.Text = "新建字体模板(&N)";
            this.ToolStripMenuItem_new.Click += new System.EventHandler(this.ToolStripMenuItem_new_Click);
            // 
            // ToolStripMenuItem_open
            // 
            this.ToolStripMenuItem_open.Name = "ToolStripMenuItem_open";
            this.ToolStripMenuItem_open.Size = new System.Drawing.Size(205, 22);
            this.ToolStripMenuItem_open.Text = "打开字体模板(&O)";
            this.ToolStripMenuItem_open.Click += new System.EventHandler(this.ToolStripMenuItem_open_Click);
            // 
            // ToolStripMenuItem_save
            // 
            this.ToolStripMenuItem_save.Name = "ToolStripMenuItem_save";
            this.ToolStripMenuItem_save.Size = new System.Drawing.Size(205, 22);
            this.ToolStripMenuItem_save.Text = "保存字体模板(&S)";
            this.ToolStripMenuItem_save.Click += new System.EventHandler(this.ToolStripMenuItem_save_Click);
            // 
            // ToolStripMenuItem_saveas
            // 
            this.ToolStripMenuItem_saveas.Name = "ToolStripMenuItem_saveas";
            this.ToolStripMenuItem_saveas.Size = new System.Drawing.Size(205, 22);
            this.ToolStripMenuItem_saveas.Text = "另存为...";
            this.ToolStripMenuItem_saveas.Click += new System.EventHandler(this.ToolStripMenuItem_saveas_Click);
            // 
            // toolStripMenuItem3
            // 
            this.toolStripMenuItem3.Name = "toolStripMenuItem3";
            this.toolStripMenuItem3.Size = new System.Drawing.Size(202, 6);
            // 
            // ToolStripMenuItem_export
            // 
            this.ToolStripMenuItem_export.Name = "ToolStripMenuItem_export";
            this.ToolStripMenuItem_export.Size = new System.Drawing.Size(205, 22);
            this.ToolStripMenuItem_export.Text = "导出字体布局...";
            this.ToolStripMenuItem_export.Click += new System.EventHandler(this.ToolStripMenuItem_export_Click);
            // 
            // ToolStripMenuItem_exporttex
            // 
            this.ToolStripMenuItem_exporttex.Name = "ToolStripMenuItem_exporttex";
            this.ToolStripMenuItem_exporttex.Size = new System.Drawing.Size(205, 22);
            this.ToolStripMenuItem_exporttex.Text = "导出字体纹理...";
            this.ToolStripMenuItem_exporttex.Click += new System.EventHandler(this.ToolStripMenuItem_exporttex_Click);
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(202, 6);
            // 
            // ToolStripMenuItem_exportLayout
            // 
            this.ToolStripMenuItem_exportLayout.Name = "ToolStripMenuItem_exportLayout";
            this.ToolStripMenuItem_exportLayout.Size = new System.Drawing.Size(205, 22);
            this.ToolStripMenuItem_exportLayout.Text = "导出布局图...";
            this.ToolStripMenuItem_exportLayout.Click += new System.EventHandler(this.ToolStripMenuItem_exportLayout_Click);
            // 
            // ToolStripMenuItem_paste
            // 
            this.ToolStripMenuItem_paste.Name = "ToolStripMenuItem_paste";
            this.ToolStripMenuItem_paste.Size = new System.Drawing.Size(205, 22);
            this.ToolStripMenuItem_paste.Text = "导出文本到剪贴板...";
            this.ToolStripMenuItem_paste.Click += new System.EventHandler(this.ToolStripMenuItem_paste_Click);
            // 
            // 填充FToolStripMenuItem
            // 
            this.填充FToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolStripMenuItem_fromText,
            this.toolStripMenuItem4,
            this.ToolStripMenuItem_setfont,
            this.ToolStripMenuItem_setcolor,
            this.ToolStripMenuItem_setmargin,
            this.ToolStripMenuItem_setadv,
            this.ToolStripMenuItem_setbaseline});
            this.填充FToolStripMenuItem.Name = "填充FToolStripMenuItem";
            this.填充FToolStripMenuItem.Size = new System.Drawing.Size(65, 21);
            this.填充FToolStripMenuItem.Text = "批量(&B)";
            // 
            // ToolStripMenuItem_fromText
            // 
            this.ToolStripMenuItem_fromText.Name = "ToolStripMenuItem_fromText";
            this.ToolStripMenuItem_fromText.Size = new System.Drawing.Size(256, 22);
            this.ToolStripMenuItem_fromText.Text = "从文本添加/填充文字(&F)";
            this.ToolStripMenuItem_fromText.Click += new System.EventHandler(this.ToolStripMenuItem_fromText_Click);
            // 
            // toolStripMenuItem4
            // 
            this.toolStripMenuItem4.Name = "toolStripMenuItem4";
            this.toolStripMenuItem4.Size = new System.Drawing.Size(253, 6);
            // 
            // ToolStripMenuItem_setfont
            // 
            this.ToolStripMenuItem_setfont.Name = "ToolStripMenuItem_setfont";
            this.ToolStripMenuItem_setfont.Size = new System.Drawing.Size(256, 22);
            this.ToolStripMenuItem_setfont.Text = "设置选中字体(&O)";
            this.ToolStripMenuItem_setfont.Click += new System.EventHandler(this.ToolStripMenuItem_setfont_Click);
            // 
            // ToolStripMenuItem_setcolor
            // 
            this.ToolStripMenuItem_setcolor.Name = "ToolStripMenuItem_setcolor";
            this.ToolStripMenuItem_setcolor.Size = new System.Drawing.Size(256, 22);
            this.ToolStripMenuItem_setcolor.Text = "设置选中颜色(&C)";
            this.ToolStripMenuItem_setcolor.Click += new System.EventHandler(this.ToolStripMenuItem_setcolor_Click);
            // 
            // ToolStripMenuItem_setmargin
            // 
            this.ToolStripMenuItem_setmargin.Name = "ToolStripMenuItem_setmargin";
            this.ToolStripMenuItem_setmargin.Size = new System.Drawing.Size(256, 22);
            this.ToolStripMenuItem_setmargin.Text = "设置选中字符边界(&M)";
            this.ToolStripMenuItem_setmargin.Click += new System.EventHandler(this.ToolStripMenuItem_setmargin_Click);
            // 
            // ToolStripMenuItem_setadv
            // 
            this.ToolStripMenuItem_setadv.Name = "ToolStripMenuItem_setadv";
            this.ToolStripMenuItem_setadv.Size = new System.Drawing.Size(256, 22);
            this.ToolStripMenuItem_setadv.Text = "设置选中字符前进量(&A)";
            this.ToolStripMenuItem_setadv.Click += new System.EventHandler(this.ToolStripMenuItem_setadv_Click);
            // 
            // ToolStripMenuItem_setbaseline
            // 
            this.ToolStripMenuItem_setbaseline.Name = "ToolStripMenuItem_setbaseline";
            this.ToolStripMenuItem_setbaseline.Size = new System.Drawing.Size(256, 22);
            this.ToolStripMenuItem_setbaseline.Text = "设置选中字符的基线高度(S)";
            this.ToolStripMenuItem_setbaseline.Click += new System.EventHandler(this.ToolStripMenuItem_setbaseline_Click);
            // 
            // 选项OToolStripMenuItem
            // 
            this.选项OToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.背景颜色BToolStripMenuItem,
            this.ToolStripMenuItem_showfont,
            this.ToolStripMenuItem_showmargin,
            this.toolStripMenuItem1,
            this.设置纹理大小SToolStripMenuItem,
            this.抗锯齿AToolStripMenuItem});
            this.选项OToolStripMenuItem.Name = "选项OToolStripMenuItem";
            this.选项OToolStripMenuItem.Size = new System.Drawing.Size(68, 21);
            this.选项OToolStripMenuItem.Text = "选项(&O)";
            // 
            // 背景颜色BToolStripMenuItem
            // 
            this.背景颜色BToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolStripMenuItem_bk_white,
            this.ToolStripMenuItem_bk_black});
            this.背景颜色BToolStripMenuItem.Name = "背景颜色BToolStripMenuItem";
            this.背景颜色BToolStripMenuItem.Size = new System.Drawing.Size(196, 22);
            this.背景颜色BToolStripMenuItem.Text = "编辑区背景颜色(&B)";
            // 
            // ToolStripMenuItem_bk_white
            // 
            this.ToolStripMenuItem_bk_white.Checked = true;
            this.ToolStripMenuItem_bk_white.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ToolStripMenuItem_bk_white.Name = "ToolStripMenuItem_bk_white";
            this.ToolStripMenuItem_bk_white.Size = new System.Drawing.Size(126, 22);
            this.ToolStripMenuItem_bk_white.Text = "白色(&W)";
            this.ToolStripMenuItem_bk_white.Click += new System.EventHandler(this.ToolStripMenuItem_bk_white_Click);
            // 
            // ToolStripMenuItem_bk_black
            // 
            this.ToolStripMenuItem_bk_black.Name = "ToolStripMenuItem_bk_black";
            this.ToolStripMenuItem_bk_black.Size = new System.Drawing.Size(126, 22);
            this.ToolStripMenuItem_bk_black.Text = "黑色(&B)";
            this.ToolStripMenuItem_bk_black.Click += new System.EventHandler(this.ToolStripMenuItem_bk_black_Click);
            // 
            // ToolStripMenuItem_showfont
            // 
            this.ToolStripMenuItem_showfont.Checked = true;
            this.ToolStripMenuItem_showfont.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ToolStripMenuItem_showfont.Name = "ToolStripMenuItem_showfont";
            this.ToolStripMenuItem_showfont.Size = new System.Drawing.Size(196, 22);
            this.ToolStripMenuItem_showfont.Text = "显示字形矩形";
            this.ToolStripMenuItem_showfont.Click += new System.EventHandler(this.ToolStripMenuItem_showfont_Click);
            // 
            // ToolStripMenuItem_showmargin
            // 
            this.ToolStripMenuItem_showmargin.Checked = true;
            this.ToolStripMenuItem_showmargin.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ToolStripMenuItem_showmargin.Name = "ToolStripMenuItem_showmargin";
            this.ToolStripMenuItem_showmargin.Size = new System.Drawing.Size(196, 22);
            this.ToolStripMenuItem_showmargin.Text = "显示边界矩形";
            this.ToolStripMenuItem_showmargin.Click += new System.EventHandler(this.ToolStripMenuItem_showmargin_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(193, 6);
            // 
            // 设置纹理大小SToolStripMenuItem
            // 
            this.设置纹理大小SToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolStripMenuItem_256x256,
            this.ToolStripMenuItem_512x512,
            this.ToolStripMenuItem_1024x1024,
            this.ToolStripMenuItem_2048x2048,
            this.ToolStripMenuItem_usersize});
            this.设置纹理大小SToolStripMenuItem.Name = "设置纹理大小SToolStripMenuItem";
            this.设置纹理大小SToolStripMenuItem.Size = new System.Drawing.Size(196, 22);
            this.设置纹理大小SToolStripMenuItem.Text = "设置纹理大小(&S)";
            // 
            // ToolStripMenuItem_256x256
            // 
            this.ToolStripMenuItem_256x256.Name = "ToolStripMenuItem_256x256";
            this.ToolStripMenuItem_256x256.Size = new System.Drawing.Size(138, 22);
            this.ToolStripMenuItem_256x256.Text = "256x256";
            this.ToolStripMenuItem_256x256.Click += new System.EventHandler(this.ToolStripMenuItem_256x256_Click);
            // 
            // ToolStripMenuItem_512x512
            // 
            this.ToolStripMenuItem_512x512.Checked = true;
            this.ToolStripMenuItem_512x512.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ToolStripMenuItem_512x512.Name = "ToolStripMenuItem_512x512";
            this.ToolStripMenuItem_512x512.Size = new System.Drawing.Size(138, 22);
            this.ToolStripMenuItem_512x512.Text = "512x512";
            this.ToolStripMenuItem_512x512.Click += new System.EventHandler(this.ToolStripMenuItem_512x512_Click);
            // 
            // ToolStripMenuItem_1024x1024
            // 
            this.ToolStripMenuItem_1024x1024.Name = "ToolStripMenuItem_1024x1024";
            this.ToolStripMenuItem_1024x1024.Size = new System.Drawing.Size(138, 22);
            this.ToolStripMenuItem_1024x1024.Text = "1024x1024";
            this.ToolStripMenuItem_1024x1024.Click += new System.EventHandler(this.ToolStripMenuItem_1024x1024_Click);
            // 
            // ToolStripMenuItem_2048x2048
            // 
            this.ToolStripMenuItem_2048x2048.Name = "ToolStripMenuItem_2048x2048";
            this.ToolStripMenuItem_2048x2048.Size = new System.Drawing.Size(138, 22);
            this.ToolStripMenuItem_2048x2048.Text = "2048x2048";
            this.ToolStripMenuItem_2048x2048.Click += new System.EventHandler(this.ToolStripMenuItem_2048x2048_Click);
            // 
            // ToolStripMenuItem_usersize
            // 
            this.ToolStripMenuItem_usersize.Name = "ToolStripMenuItem_usersize";
            this.ToolStripMenuItem_usersize.Size = new System.Drawing.Size(138, 22);
            this.ToolStripMenuItem_usersize.Text = "自定义...";
            this.ToolStripMenuItem_usersize.Click += new System.EventHandler(this.ToolStripMenuItem_usersize_Click);
            // 
            // 抗锯齿AToolStripMenuItem
            // 
            this.抗锯齿AToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolStripMenuItem_AA_Default,
            this.ToolStripMenuItem_AA_AA,
            this.ToolStripMenuItem_AA_AAGF,
            this.ToolStripMenuItem_AA_SBPP,
            this.ToolStripMenuItem_AA_SBPPGF});
            this.抗锯齿AToolStripMenuItem.Name = "抗锯齿AToolStripMenuItem";
            this.抗锯齿AToolStripMenuItem.Size = new System.Drawing.Size(196, 22);
            this.抗锯齿AToolStripMenuItem.Text = "抗锯齿(&A)";
            // 
            // ToolStripMenuItem_AA_Default
            // 
            this.ToolStripMenuItem_AA_Default.Name = "ToolStripMenuItem_AA_Default";
            this.ToolStripMenuItem_AA_Default.Size = new System.Drawing.Size(208, 22);
            this.ToolStripMenuItem_AA_Default.Text = "系统默认(&D)";
            this.ToolStripMenuItem_AA_Default.Click += new System.EventHandler(this.ToolStripMenuItem_AA_Default_Click);
            // 
            // ToolStripMenuItem_AA_AA
            // 
            this.ToolStripMenuItem_AA_AA.Checked = true;
            this.ToolStripMenuItem_AA_AA.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ToolStripMenuItem_AA_AA.Name = "ToolStripMenuItem_AA_AA";
            this.ToolStripMenuItem_AA_AA.Size = new System.Drawing.Size(208, 22);
            this.ToolStripMenuItem_AA_AA.Text = "AntiAlias";
            this.ToolStripMenuItem_AA_AA.Click += new System.EventHandler(this.ToolStripMenuItem_AA_AA_Click);
            // 
            // ToolStripMenuItem_AA_AAGF
            // 
            this.ToolStripMenuItem_AA_AAGF.Name = "ToolStripMenuItem_AA_AAGF";
            this.ToolStripMenuItem_AA_AAGF.Size = new System.Drawing.Size(208, 22);
            this.ToolStripMenuItem_AA_AAGF.Text = "AntiAliasGridFit";
            this.ToolStripMenuItem_AA_AAGF.Click += new System.EventHandler(this.ToolStripMenuItem_AA_AAGF_Click);
            // 
            // ToolStripMenuItem_AA_SBPP
            // 
            this.ToolStripMenuItem_AA_SBPP.Name = "ToolStripMenuItem_AA_SBPP";
            this.ToolStripMenuItem_AA_SBPP.Size = new System.Drawing.Size(208, 22);
            this.ToolStripMenuItem_AA_SBPP.Text = "SingleBitPerPixel";
            this.ToolStripMenuItem_AA_SBPP.Click += new System.EventHandler(this.ToolStripMenuItem_AA_SBPP_Click);
            // 
            // ToolStripMenuItem_AA_SBPPGF
            // 
            this.ToolStripMenuItem_AA_SBPPGF.Name = "ToolStripMenuItem_AA_SBPPGF";
            this.ToolStripMenuItem_AA_SBPPGF.Size = new System.Drawing.Size(208, 22);
            this.ToolStripMenuItem_AA_SBPPGF.Text = "SingleBitPerPixelGridFit";
            this.ToolStripMenuItem_AA_SBPPGF.Click += new System.EventHandler(this.ToolStripMenuItem_AA_SBPPGF_Click);
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.splitContainer1.Location = new System.Drawing.Point(0, 25);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.listView_charmap);
            this.splitContainer1.Panel1.Controls.Add(this.toolStrip1);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.splitContainer2);
            this.splitContainer1.Size = new System.Drawing.Size(771, 386);
            this.splitContainer1.SplitterDistance = 169;
            this.splitContainer1.TabIndex = 1;
            // 
            // listView_charmap
            // 
            this.listView_charmap.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.listView_charmap.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_charmap.FullRowSelect = true;
            this.listView_charmap.GridLines = true;
            this.listView_charmap.Location = new System.Drawing.Point(0, 25);
            this.listView_charmap.Name = "listView_charmap";
            this.listView_charmap.Size = new System.Drawing.Size(169, 361);
            this.listView_charmap.TabIndex = 1;
            this.listView_charmap.UseCompatibleStateImageBehavior = false;
            this.listView_charmap.View = System.Windows.Forms.View.Details;
            this.listView_charmap.SelectedIndexChanged += new System.EventHandler(this.listView_charmap_SelectedIndexChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "序";
            this.columnHeader1.Width = 50;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "字符";
            this.columnHeader2.Width = 100;
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripLabel1,
            this.toolStripButton_add,
            this.toolStripButton_remove,
            this.toolStripButton_up,
            this.toolStripButton_down});
            this.toolStrip1.Location = new System.Drawing.Point(0, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(169, 25);
            this.toolStrip1.TabIndex = 0;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // toolStripLabel1
            // 
            this.toolStripLabel1.Name = "toolStripLabel1";
            this.toolStripLabel1.Size = new System.Drawing.Size(53, 22);
            this.toolStripLabel1.Text = "字符表";
            // 
            // splitContainer2
            // 
            this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer2.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.splitContainer2.Location = new System.Drawing.Point(0, 0);
            this.splitContainer2.Name = "splitContainer2";
            // 
            // splitContainer2.Panel1
            // 
            this.splitContainer2.Panel1.AutoScroll = true;
            this.splitContainer2.Panel1.BackColor = System.Drawing.Color.White;
            this.splitContainer2.Panel1.Controls.Add(this.pictureBox_tex);
            // 
            // splitContainer2.Panel2
            // 
            this.splitContainer2.Panel2.Controls.Add(this.propertyGrid_char);
            this.splitContainer2.Size = new System.Drawing.Size(598, 386);
            this.splitContainer2.SplitterDistance = 429;
            this.splitContainer2.TabIndex = 0;
            // 
            // propertyGrid_char
            // 
            this.propertyGrid_char.Dock = System.Windows.Forms.DockStyle.Fill;
            this.propertyGrid_char.Location = new System.Drawing.Point(0, 0);
            this.propertyGrid_char.Name = "propertyGrid_char";
            this.propertyGrid_char.Size = new System.Drawing.Size(165, 386);
            this.propertyGrid_char.TabIndex = 0;
            this.propertyGrid_char.PropertyValueChanged += new System.Windows.Forms.PropertyValueChangedEventHandler(this.propertyGrid_char_PropertyValueChanged);
            // 
            // saveFileDialog_def
            // 
            this.saveFileDialog_def.DefaultExt = "xml";
            this.saveFileDialog_def.Filter = "XML文件|*.xml";
            this.saveFileDialog_def.Title = "保存字体定义文件";
            // 
            // openFileDialog_def
            // 
            this.openFileDialog_def.Filter = "XML文件|*.xml";
            this.openFileDialog_def.Title = "打开字体定义文件";
            // 
            // saveFileDialog_savepng
            // 
            this.saveFileDialog_savepng.DefaultExt = "png";
            this.saveFileDialog_savepng.Filter = "PNG图片(*.png)|*.png";
            this.saveFileDialog_savepng.Title = "保存PNG图片";
            // 
            // fontDialog_main
            // 
            this.fontDialog_main.Color = System.Drawing.SystemColors.ControlText;
            // 
            // saveFileDialog_savexml
            // 
            this.saveFileDialog_savexml.DefaultExt = "xml";
            this.saveFileDialog_savexml.Filter = "F2D Xml文件(*.xml)|*.xml";
            this.saveFileDialog_savexml.Title = "保存XML文件";
            // 
            // toolStripButton_add
            // 
            this.toolStripButton_add.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButton_add.Image = global::TexturedFontEditor.Properties.Resources.Add;
            this.toolStripButton_add.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButton_add.Name = "toolStripButton_add";
            this.toolStripButton_add.Size = new System.Drawing.Size(23, 22);
            this.toolStripButton_add.Text = "增加字符";
            this.toolStripButton_add.Click += new System.EventHandler(this.toolStripButton_add_Click);
            // 
            // toolStripButton_remove
            // 
            this.toolStripButton_remove.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButton_remove.Image = global::TexturedFontEditor.Properties.Resources.Remove;
            this.toolStripButton_remove.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButton_remove.Name = "toolStripButton_remove";
            this.toolStripButton_remove.Size = new System.Drawing.Size(23, 22);
            this.toolStripButton_remove.Text = "删除字符";
            this.toolStripButton_remove.Click += new System.EventHandler(this.toolStripButton_remove_Click);
            // 
            // toolStripButton_up
            // 
            this.toolStripButton_up.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButton_up.Image = global::TexturedFontEditor.Properties.Resources.MoveUp;
            this.toolStripButton_up.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButton_up.Name = "toolStripButton_up";
            this.toolStripButton_up.Size = new System.Drawing.Size(23, 22);
            this.toolStripButton_up.Text = "上移";
            this.toolStripButton_up.Click += new System.EventHandler(this.toolStripButton_up_Click);
            // 
            // toolStripButton_down
            // 
            this.toolStripButton_down.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButton_down.Image = global::TexturedFontEditor.Properties.Resources.MoveDown;
            this.toolStripButton_down.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButton_down.Name = "toolStripButton_down";
            this.toolStripButton_down.Size = new System.Drawing.Size(23, 22);
            this.toolStripButton_down.Text = "下移";
            this.toolStripButton_down.Click += new System.EventHandler(this.toolStripButton_down_Click);
            // 
            // pictureBox_tex
            // 
            this.pictureBox_tex.BackColor = System.Drawing.Color.Transparent;
            this.pictureBox_tex.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pictureBox_tex.Location = new System.Drawing.Point(0, 0);
            this.pictureBox_tex.Name = "pictureBox_tex";
            this.pictureBox_tex.Size = new System.Drawing.Size(139, 193);
            this.pictureBox_tex.TabIndex = 0;
            this.pictureBox_tex.TabStop = false;
            this.pictureBox_tex.Paint += new System.Windows.Forms.PaintEventHandler(this.pictureBox_tex_Paint);
            // 
            // FontEditor
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(771, 411);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.menuStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "FontEditor";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "纹理化字体编辑器";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FontEditor_FormClosing);
            this.Load += new System.EventHandler(this.FontEditor_Load);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel1.PerformLayout();
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.splitContainer2.Panel1.ResumeLayout(false);
            this.splitContainer2.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).EndInit();
            this.splitContainer2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox_tex)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem 文件FToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_new;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_open;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_save;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_saveas;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.ListView listView_charmap;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripLabel toolStripLabel1;
        private System.Windows.Forms.ToolStripButton toolStripButton_remove;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private System.Windows.Forms.PropertyGrid propertyGrid_char;
        private System.Windows.Forms.PictureBox pictureBox_tex;
        private System.Windows.Forms.ToolStripMenuItem 选项OToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 背景颜色BToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_bk_white;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_bk_black;
        private System.Windows.Forms.SaveFileDialog saveFileDialog_def;
        private System.Windows.Forms.OpenFileDialog openFileDialog_def;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_export;
        private System.Windows.Forms.ToolStripMenuItem 填充FToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_fromText;
        private System.Windows.Forms.ToolStripButton toolStripButton_up;
        private System.Windows.Forms.ToolStripButton toolStripButton_down;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem 设置纹理大小SToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_256x256;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_512x512;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_1024x1024;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_2048x2048;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_usersize;
        private System.Windows.Forms.ToolStripButton toolStripButton_add;
        private System.Windows.Forms.ToolStripMenuItem 抗锯齿AToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_AA_Default;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_AA_AA;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_AA_AAGF;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_AA_SBPP;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_AA_SBPPGF;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem4;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_setfont;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_setcolor;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_setmargin;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_setadv;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_exportLayout;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_paste;
        private System.Windows.Forms.SaveFileDialog saveFileDialog_savepng;
        private System.Windows.Forms.FontDialog fontDialog_main;
        private System.Windows.Forms.ColorDialog colorDialog_main;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_showfont;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_showmargin;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_setbaseline;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_exporttex;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
        private System.Windows.Forms.SaveFileDialog saveFileDialog_savexml;
    }
}