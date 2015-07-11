using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Xml;

namespace TexturedFontEditor
{
    public partial class FontEditor : Form
    {
        private FontDef _Def = new FontDef();
        private string _FileName = String.Empty;

        public FontEditor()
        {
            InitializeComponent();
        }

        void RefreshAll()
        {
            ToolStripMenuItem_AA_Default.Checked = false;
            ToolStripMenuItem_AA_AA.Checked = false;
            ToolStripMenuItem_AA_AAGF.Checked = false;
            ToolStripMenuItem_AA_SBPP.Checked = false;
            ToolStripMenuItem_AA_SBPPGF.Checked = false;
            switch (_Def.AntiAlias)
            {
                case System.Drawing.Text.TextRenderingHint.SystemDefault:
                    ToolStripMenuItem_AA_Default.Checked = true;
                    break;
                case System.Drawing.Text.TextRenderingHint.AntiAlias:
                    ToolStripMenuItem_AA_AA.Checked = true;
                    break;
                case System.Drawing.Text.TextRenderingHint.AntiAliasGridFit:
                    ToolStripMenuItem_AA_AAGF.Checked = true;
                    break;
                case System.Drawing.Text.TextRenderingHint.SingleBitPerPixel:
                    ToolStripMenuItem_AA_SBPP.Checked = true;
                    break;
                case System.Drawing.Text.TextRenderingHint.SingleBitPerPixelGridFit:
                    ToolStripMenuItem_AA_SBPPGF.Checked = true;
                    break;
            }

            ToolStripMenuItem_256x256.Checked = false;
            ToolStripMenuItem_512x512.Checked = false;
            ToolStripMenuItem_1024x1024.Checked = false;
            ToolStripMenuItem_2048x2048.Checked = false;
            ToolStripMenuItem_usersize.Checked = false;
            if (_Def.TexWidth == 256 && _Def.TexHeight == 256)
                ToolStripMenuItem_256x256.Checked = true;
            else if (_Def.TexWidth == 512 && _Def.TexHeight == 512)
                ToolStripMenuItem_512x512.Checked = true;
            else if (_Def.TexWidth == 1024 && _Def.TexHeight == 1024)
                ToolStripMenuItem_1024x1024.Checked = true;
            else if (_Def.TexWidth == 2048 && _Def.TexHeight == 2048)
                ToolStripMenuItem_2048x2048.Checked = true;
            else
                ToolStripMenuItem_usersize.Checked = true;

            _Def.RenderTexture();

            listView_charmap.Items.Clear();
            propertyGrid_char.SelectedObject = null;
            pictureBox_tex.Image = _Def.Texture;
            pictureBox_tex.Width = _Def.Texture.Width;
            pictureBox_tex.Height = _Def.Texture.Height;

            int tID = 0;
            foreach (FontDef.CharDef x in _Def.CharList)
            {
                ListViewItem tItem = new ListViewItem();
                tItem.Text = tID.ToString();
                tItem.Tag = x;

                ListViewItem.ListViewSubItem tSubItem = new ListViewItem.ListViewSubItem();
                tSubItem.Text = x.Character.ToString();
                tItem.SubItems.Add(tSubItem);

                listView_charmap.Items.Add(tItem);

                tID++;
            }

            pictureBox_tex.Invalidate();
        }

        void Repaint()
        {
            _Def.RenderTexture();
            pictureBox_tex.Image = _Def.Texture;
            pictureBox_tex.Width = _Def.Texture.Width;
            pictureBox_tex.Height = _Def.Texture.Height;
            
            pictureBox_tex.Invalidate();
        }

        private Pen _M_ImageMargin = new Pen(Color.Blue);
        private Pen _M_RealMargin = new Pen(Color.LightBlue);
        private Pen _M_SelectedMargin = new Pen(Color.Red);
        void PaintLayout(Graphics g, bool bShowSelectedItem)
        {
            HashSet<FontDef.CharDef> tSet = null;
            if (bShowSelectedItem)
            {
                tSet = new HashSet<FontDef.CharDef>();

                if (listView_charmap.SelectedItems.Count > 0)
                {
                    foreach (ListViewItem y in listView_charmap.SelectedItems)
                    {
                        tSet.Add((FontDef.CharDef)y.Tag);
                    }
                }
            }

            if (ToolStripMenuItem_showmargin.Checked)
            {
                // 图像辅助线
                foreach (FontDef.CharDef x in _Def.CharList)
                {
                    g.DrawRectangle(_M_ImageMargin, new Rectangle(x.Location, x.CharSize));
                }
            }

            if (ToolStripMenuItem_showfont.Checked)
            {
                // 文字度量线
                foreach (FontDef.CharDef x in _Def.CharList)
                {
                    bool bSelected = false;

                    if (bShowSelectedItem)
                        if (tSet.Contains(x))
                            bSelected = true;

                    if (bSelected)
                    {
                        g.DrawRectangle(_M_SelectedMargin,
                            new Rectangle(x.Location.X + x.ImageMargin, x.Location.Y + x.ImageMargin, x.CharSize.Width - x.ImageMargin * 2, x.CharSize.Height - x.ImageMargin * 2));
                    }
                    else
                    {
                        g.DrawRectangle(_M_RealMargin,
                            new Rectangle(x.Location.X + x.ImageMargin, x.Location.Y + x.ImageMargin, x.CharSize.Width - x.ImageMargin * 2, x.CharSize.Height - x.ImageMargin * 2));
                    }
                }
            }
        }

        private void FontEditor_Load(object sender, EventArgs e)
        {
            RefreshAll();
        }

        private void toolStripButton_add_Click(object sender, EventArgs e)
        {
            string tOut;
            if(DialogResult.OK == Dialog_InputBox.OpenDialog("要添加字符的个数:", "添加字符", "1", out tOut))
            {
                int tCount = 0;

                try
                {
                    tCount = Convert.ToInt32(tOut);
                }
                catch (Exception Expt)
                {
                    Helper.ShowErr("无效值：" + Expt.Message);
                    return;
                }

                for(int i = 0; i<tCount; ++i)
                {
                    _Def.CharList.Add(new FontDef.CharDef());
                }

                RefreshAll();
            }
        }

        private void toolStripButton_remove_Click(object sender, EventArgs e)
        {
            if (listView_charmap.SelectedItems.Count > 0)
            {
                if (DialogResult.Yes == MessageBox.Show("是否删除选中项目？", "删除字符", MessageBoxButtons.YesNo, MessageBoxIcon.Question))
                {
                    foreach (ListViewItem x in listView_charmap.SelectedItems)
                    {
                        _Def.CharList.Remove((FontDef.CharDef)x.Tag);
                    }

                    RefreshAll();
                }
            }
        }

        private void toolStripButton_up_Click(object sender, EventArgs e)
        {
            if (listView_charmap.SelectedItems.Count > 0)
            {
                int tObjIndex = listView_charmap.SelectedIndices[0];
                ListViewItem tItem = listView_charmap.SelectedItems[0];

                if (tObjIndex > 0)
                    tObjIndex--;

                listView_charmap.Items.Remove(tItem);
                listView_charmap.Items.Insert(tObjIndex, tItem);

                listView_charmap.SelectedIndices.Clear();
                listView_charmap.SelectedIndices.Add(tItem.Index);

                _Def.CharList.Remove((FontDef.CharDef)tItem.Tag);
                _Def.CharList.Insert(tObjIndex, (FontDef.CharDef)tItem.Tag);

                // 刷新序号
                foreach (ListViewItem x in listView_charmap.Items)
                {
                    x.Text = _Def.CharList.IndexOf(((FontDef.CharDef)x.Tag)).ToString();
                }

                Repaint();
            }
        }

        private void toolStripButton_down_Click(object sender, EventArgs e)
        {
            if (listView_charmap.SelectedItems.Count > 0)
            {
                int tObjIndex = listView_charmap.SelectedIndices[0];
                ListViewItem tItem = listView_charmap.SelectedItems[0];

                if (tObjIndex < listView_charmap.Items.Count)
                    tObjIndex++;

                listView_charmap.Items.Remove(tItem);

                if (tObjIndex > listView_charmap.Items.Count)
                    tObjIndex = listView_charmap.Items.Count;

                listView_charmap.Items.Insert(tObjIndex, tItem);

                listView_charmap.SelectedIndices.Clear();
                listView_charmap.SelectedIndices.Add(tItem.Index);

                _Def.CharList.Remove((FontDef.CharDef)tItem.Tag);
                _Def.CharList.Insert(tObjIndex, (FontDef.CharDef)tItem.Tag);

                // 刷新序号
                foreach (ListViewItem x in listView_charmap.Items)
                {
                    x.Text = _Def.CharList.IndexOf(((FontDef.CharDef)x.Tag)).ToString();
                }

                Repaint();
            }
        }

        private void ToolStripMenuItem_new_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("你确定要新建字模？\n\n所有未保存的修改将会丢失。", "新建字模", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
            {
                _FileName = String.Empty;
                _Def = new FontDef();

                RefreshAll();
            }
        }

        private void ToolStripMenuItem_open_Click(object sender, EventArgs e)
        {
            if (DialogResult.OK == openFileDialog_def.ShowDialog())
            {
                FontDef tDef = null;

                try
                {
                    tDef = new FontDef(openFileDialog_def.FileName);
                }
                catch (Exception Expt)
                {
                    Helper.ShowErr("打开字模失败！\n" + Expt.Message);
                    return;
                }

                _Def = tDef;
                _FileName = openFileDialog_def.FileName;

                RefreshAll();
            }
        }

        private void ToolStripMenuItem_save_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(_FileName))
            {
                if (DialogResult.OK == saveFileDialog_def.ShowDialog())
                {
                    _FileName = saveFileDialog_def.FileName;
                }
                else
                    return;
            }

            try
            {
                _Def.SaveTo(_FileName);
            }
            catch (Exception Expt)
            {
                Helper.ShowErr("保存失败！\n" + Expt.Message);
                _FileName = String.Empty;
            }
        }

        private void ToolStripMenuItem_saveas_Click(object sender, EventArgs e)
        {
            if (DialogResult.OK == saveFileDialog_def.ShowDialog())
            {
                _FileName = saveFileDialog_def.FileName;

                try
                {
                    _Def.SaveTo(_FileName);
                }
                catch (Exception Expt)
                {
                    Helper.ShowErr("保存失败！\n" + Expt.Message);
                    _FileName = String.Empty;
                }
            }
        }

        private void ToolStripMenuItem_export_Click(object sender, EventArgs e)
        {            
            // 检查字符表
            HashSet<char> tCharset = new HashSet<char>();
            foreach(FontDef.CharDef x in _Def.CharList)
            {
                if(tCharset.Contains(x.Character))
                {
                    Helper.ShowErr("字符表中存在重复字符: " + x.Character.ToString());
                    return;
                }
                else if(x.Character == '\0')
                {
                    Helper.ShowErr("字符表中存在无效字符。");
                    return;
                }
                else
                    tCharset.Add(x.Character);
            }

            // 决定保存路径
            string tPathToSave;
            if (DialogResult.Cancel == saveFileDialog_savexml.ShowDialog())
                return;
            tPathToSave = saveFileDialog_savexml.FileName;

            string tOut;
            if (DialogResult.Cancel == Dialog_InputBox.OpenDialog("请输入行间距:", "设置行间距", "0", out tOut))
                return;

            int tLineGap;
            try
            {
                tLineGap = Convert.ToInt32(tOut);
            }
            catch (Exception Expt)
            {
                Helper.ShowErr("格式不正确: " + Expt.Message);
                return;
            }

            XmlDocument tDoc = new XmlDocument();

            XmlElement tRoot = tDoc.CreateElement("f2dTexturedFont");

            XmlElement tMeasure = tDoc.CreateElement("Measure");
            tMeasure.SetAttribute("LineHeight", (_Def.MaxLineHeight + tLineGap).ToString());
            tMeasure.SetAttribute("Ascender", _Def.MaxLineHeight.ToString());
            tMeasure.SetAttribute("Descender", "0");
            tRoot.AppendChild(tMeasure);

            // 追加字符表
            XmlElement tCharTable = tDoc.CreateElement("CharList");
            foreach (FontDef.CharDef x in _Def.CharList)
            {
                XmlElement tItem = tDoc.CreateElement("Item");

                tItem.SetAttribute("Char", x.Character.ToString());
                tItem.SetAttribute("Pos", String.Format("{0},{1}", x.Location.X, x.Location.Y));
                tItem.SetAttribute("Size", String.Format("{0},{1}", x.CharSize.Width, x.CharSize.Height));
                tItem.SetAttribute("BrushPos", String.Format("{0},{1}", x.ImageMargin, x.CharSize.Height - x.ImageMargin + x.Baseline));
                tItem.SetAttribute("Advance", String.Format("{0},{1}", x.CharSize.Width - x.ImageMargin * 2 + x.Advance, 0));

                tCharTable.AppendChild(tItem);
            }
            tRoot.AppendChild(tCharTable);

            tDoc.AppendChild(tRoot);

            try
            {
                Helper.WriteFSTGXmlType(tDoc, tPathToSave);
            }
            catch (Exception Expt)
            {
                Helper.ShowErr("保存失败！\n" + Expt.Message);
            }
        }

        private void ToolStripMenuItem_fromText_Click(object sender, EventArgs e)
        {
            List<FontDef.CharDef> tList = new List<FontDef.CharDef>();

            foreach(ListViewItem x in listView_charmap.SelectedItems)
            {
                tList.Add((FontDef.CharDef)x.Tag);
            }

            if (DialogResult.OK == FontEditor_FromText.OpenDialog(_Def, tList))
                RefreshAll();
        }

        private void ToolStripMenuItem_bk_white_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_bk_black.Checked = false;
            ToolStripMenuItem_bk_white.Checked = true;

            splitContainer2.Panel1.BackColor = Color.White;
        }

        private void ToolStripMenuItem_bk_black_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_bk_black.Checked = true;
            ToolStripMenuItem_bk_white.Checked = false;

            splitContainer2.Panel1.BackColor = Color.Black;
        }

        private void ToolStripMenuItem_256x256_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_256x256.Checked = true;
            ToolStripMenuItem_512x512.Checked = false;
            ToolStripMenuItem_1024x1024.Checked = false;
            ToolStripMenuItem_2048x2048.Checked = false;
            ToolStripMenuItem_usersize.Checked = false;

            _Def.TexWidth = 256;
            _Def.TexHeight = 256;
            RefreshAll();
        }

        private void ToolStripMenuItem_512x512_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_256x256.Checked = false;
            ToolStripMenuItem_512x512.Checked = true;
            ToolStripMenuItem_1024x1024.Checked = false;
            ToolStripMenuItem_2048x2048.Checked = false;
            ToolStripMenuItem_usersize.Checked = false;

            _Def.TexWidth = 512;
            _Def.TexHeight = 512;
            RefreshAll();
        }

        private void ToolStripMenuItem_1024x1024_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_256x256.Checked = false;
            ToolStripMenuItem_512x512.Checked = false;
            ToolStripMenuItem_1024x1024.Checked = true;
            ToolStripMenuItem_2048x2048.Checked = false;
            ToolStripMenuItem_usersize.Checked = false;

            _Def.TexWidth = 1024;
            _Def.TexHeight = 1024;
            RefreshAll();
        }

        private void ToolStripMenuItem_2048x2048_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_256x256.Checked = false;
            ToolStripMenuItem_512x512.Checked = false;
            ToolStripMenuItem_1024x1024.Checked = false;
            ToolStripMenuItem_2048x2048.Checked = true;
            ToolStripMenuItem_usersize.Checked = false;

            _Def.TexWidth = 2048;
            _Def.TexHeight = 2048;
            RefreshAll();
        }

        private void ToolStripMenuItem_usersize_Click(object sender, EventArgs e)
        {
            string tOutWidth;
            string tOutHeight;
            int tWidth;
            int tHeight;

            if (DialogResult.OK == Dialog_InputBox.OpenDialog("请输入纹理宽度", "纹理宽度", _Def.TexWidth.ToString(), out tOutWidth))
            {
                try
                {
                    tWidth = Convert.ToInt32(tOutWidth);
                }
                catch (Exception Expt)
                {
                    Helper.ShowErr("格式不正确: " + Expt.Message);
                    return;
                }

                if (DialogResult.OK == Dialog_InputBox.OpenDialog("请输入纹理高度", "纹理高度", _Def.TexHeight.ToString(), out tOutHeight))
                {
                    try
                    {
                        tHeight = Convert.ToInt32(tOutHeight);
                    }
                    catch (Exception Expt)
                    {
                        Helper.ShowErr("格式不正确: " + Expt.Message);
                        return;
                    }
                }
                else
                    return;
            }
            else
                return;

            ToolStripMenuItem_256x256.Checked = false;
            ToolStripMenuItem_512x512.Checked = false;
            ToolStripMenuItem_1024x1024.Checked = false;
            ToolStripMenuItem_2048x2048.Checked = false;
            ToolStripMenuItem_usersize.Checked = true;

            _Def.TexWidth = tWidth;
            _Def.TexHeight = tHeight;
            RefreshAll();
        }

        private void listView_charmap_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (listView_charmap.SelectedItems.Count > 0)
            {
                ListViewItem tItem = listView_charmap.SelectedItems[0];

                propertyGrid_char.SelectedObject = (FontDef.CharDef)tItem.Tag;
            }
            else
                propertyGrid_char.SelectedObject = null;

            pictureBox_tex.Invalidate();
        }

        private void propertyGrid_char_PropertyValueChanged(object s, PropertyValueChangedEventArgs e)
        {
            // 刷新数据
            foreach (ListViewItem x in listView_charmap.Items)
            {
                x.SubItems[1].Text = ((FontDef.CharDef)x.Tag).Character.ToString();
            }

            Repaint();
        }

        private void pictureBox_tex_Paint(object sender, PaintEventArgs e)
        {
            PaintLayout(e.Graphics, true);
        }

        private void ToolStripMenuItem_AA_Default_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_AA_Default.Checked = true;
            ToolStripMenuItem_AA_AA.Checked = false;
            ToolStripMenuItem_AA_AAGF.Checked = false;
            ToolStripMenuItem_AA_SBPP.Checked = false;
            ToolStripMenuItem_AA_SBPPGF.Checked = false;

            _Def.AntiAlias = System.Drawing.Text.TextRenderingHint.SystemDefault;
            Repaint();
        }

        private void ToolStripMenuItem_AA_AA_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_AA_Default.Checked = false;
            ToolStripMenuItem_AA_AA.Checked = true;
            ToolStripMenuItem_AA_AAGF.Checked = false;
            ToolStripMenuItem_AA_SBPP.Checked = false;
            ToolStripMenuItem_AA_SBPPGF.Checked = false;

            _Def.AntiAlias = System.Drawing.Text.TextRenderingHint.AntiAlias;
            Repaint();
        }

        private void ToolStripMenuItem_AA_AAGF_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_AA_Default.Checked = false;
            ToolStripMenuItem_AA_AA.Checked = false;
            ToolStripMenuItem_AA_AAGF.Checked = true;
            ToolStripMenuItem_AA_SBPP.Checked = false;
            ToolStripMenuItem_AA_SBPPGF.Checked = false;

            _Def.AntiAlias = System.Drawing.Text.TextRenderingHint.AntiAliasGridFit;
            Repaint();
        }

        private void ToolStripMenuItem_AA_SBPP_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_AA_Default.Checked = false;
            ToolStripMenuItem_AA_AA.Checked = false;
            ToolStripMenuItem_AA_AAGF.Checked = false;
            ToolStripMenuItem_AA_SBPP.Checked = true;
            ToolStripMenuItem_AA_SBPPGF.Checked = false;

            _Def.AntiAlias = System.Drawing.Text.TextRenderingHint.SingleBitPerPixel;
            Repaint();
        }

        private void ToolStripMenuItem_AA_SBPPGF_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_AA_Default.Checked = false;
            ToolStripMenuItem_AA_AA.Checked = false;
            ToolStripMenuItem_AA_AAGF.Checked = false;
            ToolStripMenuItem_AA_SBPP.Checked = false;
            ToolStripMenuItem_AA_SBPPGF.Checked = true;

            _Def.AntiAlias = System.Drawing.Text.TextRenderingHint.SingleBitPerPixelGridFit;
            Repaint();
        }

        private void ToolStripMenuItem_setfont_Click(object sender, EventArgs e)
        {
            if (listView_charmap.SelectedItems.Count > 0)
            {
                if (DialogResult.OK == fontDialog_main.ShowDialog())
                {
                    foreach (ListViewItem x in listView_charmap.SelectedItems)
                    {
                        ((FontDef.CharDef)x.Tag).SrcFont = fontDialog_main.Font;
                    }
                }
            }

            Repaint();
        }

        private void ToolStripMenuItem_setcolor_Click(object sender, EventArgs e)
        {
            if (listView_charmap.SelectedItems.Count > 0)
            {
                if (DialogResult.OK == colorDialog_main.ShowDialog())
                {
                    foreach (ListViewItem x in listView_charmap.SelectedItems)
                    {
                        ((FontDef.CharDef)x.Tag).FontColor = colorDialog_main.Color;
                    }
                }
            }

            Repaint();
        }

        private void ToolStripMenuItem_setmargin_Click(object sender, EventArgs e)
        {
            if (listView_charmap.SelectedItems.Count > 0)
            {
                string tOut;
                if (DialogResult.OK == Dialog_InputBox.OpenDialog("请输入图像边距, 前缀@表示相对量:", "设置图像边距", "1", out tOut))
                {
                    bool tOffset = false;
                    if (tOut.Length > 0 && tOut[0] == '@')
                    {
                        tOffset = true;
                        tOut = tOut.Remove(0, 1);
                    }

                    int tValue;
                    try
                    {
                        tValue = Convert.ToInt32(tOut);
                    }
                    catch (Exception Expt)
                    {
                        Helper.ShowErr("格式不正确: " + Expt.Message);
                        return;
                    }

                    foreach (ListViewItem x in listView_charmap.SelectedItems)
                    {
                        FontDef.CharDef tDef = (FontDef.CharDef)x.Tag;

                        if (tOffset)
                        {
                            tDef.ImageMargin += tValue;
                        }
                        else
                        {
                            tDef.ImageMargin = tValue;
                        }
                    }
                }
            }

            Repaint();
        }

        private void ToolStripMenuItem_setadv_Click(object sender, EventArgs e)
        {
            if (listView_charmap.SelectedItems.Count > 0)
            {
                string tOut;
                if (DialogResult.OK == Dialog_InputBox.OpenDialog("请输入前进量, 前缀@表示相对量:", "设置字体额外前进量", "1", out tOut))
                {
                    bool tOffset = false;
                    if (tOut.Length > 0 && tOut[0] == '@')
                    {
                        tOffset = true;
                        tOut = tOut.Remove(0, 1);
                    }

                    int tValue;
                    try
                    {
                        tValue = Convert.ToInt32(tOut);
                    }
                    catch (Exception Expt)
                    {
                        Helper.ShowErr("格式不正确: " + Expt.Message);
                        return;
                    }

                    foreach (ListViewItem x in listView_charmap.SelectedItems)
                    {
                        FontDef.CharDef tDef = (FontDef.CharDef)x.Tag;

                        if (tOffset)
                        {
                            tDef.Advance += tValue;
                        }
                        else
                        {
                            tDef.Advance = tValue;
                        }
                    }
                }
            }

            Repaint();
        }

        private void ToolStripMenuItem_showfont_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_showfont.Checked = !ToolStripMenuItem_showfont.Checked;

            Repaint();
        }

        private void ToolStripMenuItem_showmargin_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem_showmargin.Checked = !ToolStripMenuItem_showmargin.Checked;

            Repaint();
        }

        private void ToolStripMenuItem_setbaseline_Click(object sender, EventArgs e)
        {
            if (listView_charmap.SelectedItems.Count > 0)
            {
                string tOut;
                if (DialogResult.OK == Dialog_InputBox.OpenDialog("请输入基线值, 前缀@表示相对量:", "设置字体基线", "0", out tOut))
                {
                    bool tOffset = false;
                    if (tOut.Length > 0 && tOut[0] == '@')
                    {
                        tOffset = true;
                        tOut = tOut.Remove(0, 1);
                    }

                    int tValue;
                    try
                    {
                        tValue = Convert.ToInt32(tOut);
                    }
                    catch (Exception Expt)
                    {
                        Helper.ShowErr("格式不正确: " + Expt.Message);
                        return;
                    }

                    foreach (ListViewItem x in listView_charmap.SelectedItems)
                    {
                        FontDef.CharDef tDef = (FontDef.CharDef)x.Tag;

                        if (tOffset)
                        {
                            tDef.Baseline += tValue;
                        }
                        else
                        {
                            tDef.Baseline = tValue;
                        }
                    }
                }
            }

            Repaint();
        }

        private void FontEditor_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (DialogResult.Yes != MessageBox.Show("你确定要关闭窗口么？\n\n所有未保存的修改将丢失！", "关闭纹理字体编辑器", MessageBoxButtons.YesNo, MessageBoxIcon.Warning))
                e.Cancel = true;
        }

        private void ToolStripMenuItem_exporttex_Click(object sender, EventArgs e)
        {
            if (DialogResult.OK == saveFileDialog_savepng.ShowDialog())
            {
                _Def.RenderTexture();

                try
                {
                    _Def.Texture.Save(saveFileDialog_savepng.FileName, System.Drawing.Imaging.ImageFormat.Png);
                }
                catch (Exception Expt)
                {
                    Helper.ShowErr("保存失败！\n" + Expt.Message);
                }
            }
        }

        private void ToolStripMenuItem_exportLayout_Click(object sender, EventArgs e)
        {
            if (DialogResult.OK == saveFileDialog_savepng.ShowDialog())
            {
                Bitmap tData = new Bitmap(_Def.TexWidth, _Def.TexHeight);
                Graphics tGraph = Graphics.FromImage(tData);
                tGraph.Clear(Color.Transparent);

                PaintLayout(tGraph, false);

                try
                {
                    tData.Save(saveFileDialog_savepng.FileName, System.Drawing.Imaging.ImageFormat.Png);
                }
                catch (Exception Expt)
                {
                    Helper.ShowErr("保存失败！\n" + Expt.Message);
                }
            }
        }

        private void ToolStripMenuItem_paste_Click(object sender, EventArgs e)
        {
            string tText = String.Empty;

            foreach (FontDef.CharDef x in _Def.CharList)
            {
                tText += x.Character;
            }

            bool tOK = false;
            while (!tOK)
            {
                try
                {
                    Clipboard.SetText(tText);
                    tOK = true;
                }
                catch (Exception)
                {
                    if (DialogResult.Cancel == MessageBox.Show("复制到剪辑版失败，可能剪辑版正在被使用。", "错误", MessageBoxButtons.RetryCancel, MessageBoxIcon.Error))
                        break;
                }
            }
        }
    }

    public class FontDef
    {
        public class CharDef
        {
            private char _Character;        // 字符
            private Font _SrcFont;          // 渲染所用字体
            private Color _FontColor = Color.Black; // 渲染所用颜色
            private int _Advance = 1;        // 额外前进量
            private int _Baseline = 0;       // 额外基线
            private int _ImageMargin = 1;    // 图像保留边界

            private Size _CharSize = new Size();  // 字符大小
            private Point _Location = new Point();// 位置

            [Category("文字")]
            [DisplayName("字符")]
            [Description("指定字符。")]
            public char Character
            {
                get
                {
                    return _Character;
                }
                set
                {
                    _Character = value;
                }
            }

            [Category("文字")]
            [DisplayName("字体")]
            [Description("指定字体。")]
            public Font SrcFont
            {
                get
                {
                    return _SrcFont;
                }
                set
                {
                    _SrcFont = value;
                }
            }

            [Category("文字")]
            [DisplayName("颜色")]
            [Description("指定字体颜色。")]
            public Color FontColor
            {
                get
                {
                    return _FontColor;
                }
                set
                {
                    _FontColor = value;
                }
            }

            [Category("定位")]
            [DisplayName("额外前进量")]
            [Description("指定笔触的额外前进量。")]
            public int Advance
            {
                get
                {
                    return _Advance;
                }
                set
                {
                    _Advance = value;
                }
            }

            [Category("定位")]
            [DisplayName("基线偏移")]
            [Description("指定基线距离底部的偏移，负值向上移动基线，正值向下移动基线。")]
            public int Baseline
            {
                get
                {
                    return _Baseline;
                }
                set
                {
                    _Baseline = value;
                }
            }

            [Category("定位")]
            [DisplayName("图像边距")]
            [Description("指定字符的有效图像大小。")]
            public int ImageMargin
            {
                get
                {
                    return _ImageMargin;
                }
                set
                {
                    _ImageMargin = value;
                }
            }

            [Browsable(false)]
            public Size CharSize
            {
                get
                {
                    return _CharSize;
                }
            }

            [Browsable(false)]
            public Point Location
            {
                get
                {
                    return _Location;
                }
                set
                {
                    _Location = value;
                }
            }

            public Size GetCharSize(Graphics g)
            {
                if (g == null)
                    return _CharSize;

                if (_Character == '\0' || _SrcFont == null)
                {
                    _CharSize = new Size(_ImageMargin * 2, _ImageMargin * 2);
                    return _CharSize;
                }
                else
                {
                    SizeF tSize = g.MeasureString(String.Format("{0}", _Character), _SrcFont, new PointF(0, 0), StringFormat.GenericTypographic);
                    _CharSize = new Size((int)Math.Round(tSize.Width), (int)Math.Round(tSize.Height));
                    _CharSize.Width += _ImageMargin * 2;
                    _CharSize.Height += _ImageMargin * 2;
                    return _CharSize;
                }
            }

            public CharDef()
            {
            }
        }

        private List<CharDef> _CharList = new List<CharDef>();
        private int _TexWidth = 512;
        private int _TexHeight = 512;

        private int _MaxLineHeight = 0;
        private System.Drawing.Text.TextRenderingHint _AntiAlias = System.Drawing.Text.TextRenderingHint.AntiAlias;

        private Bitmap _FontTexture = null;

        public System.Drawing.Text.TextRenderingHint AntiAlias
        {
            get
            {
                return _AntiAlias;
            }
            set
            {
                _AntiAlias = value;
            }
        }

        public IList<CharDef> CharList
        {
            get
            {
                return _CharList;
            }
        }

        public int TexWidth
        {
            get
            {
                return _TexWidth;
            }
            set
            {
                _TexWidth = value;
                if (_TexWidth <= 0)
                    _TexWidth = 1;
            }
        }

        public int TexHeight
        {
            get
            {
                return _TexHeight;
            }
            set
            {
                _TexHeight = value;
                if (_TexHeight <= 0)
                    _TexHeight = 1;
            }
        }

        public Bitmap Texture
        {
            get
            {
                return _FontTexture;
            }
        }

        public int MaxLineHeight
        {
            get
            {
                return _MaxLineHeight;
            }
        }

        public void RenderTexture()
        {
            _MaxLineHeight = 0;

            Bitmap tTexture = new Bitmap(_TexWidth, _TexHeight);
            using (Graphics tGraph = Graphics.FromImage(tTexture))
            {
                tGraph.Clear(Color.Transparent);
                tGraph.TextRenderingHint = AntiAlias;

                // 第一次遍历 计算出字形大小和Location.x以及行索引
                Dictionary<CharDef, int> tLineDict = new Dictionary<CharDef, int>();
                SortedDictionary<int, int> tLineHeight = new SortedDictionary<int, int>();
                int tLeft = 0;
                int tLine = 0;
                foreach (CharDef x in _CharList)
                {
                    Size tCharSize = x.GetCharSize(tGraph);

                    // 检查字体能不能容纳到纹理中
                    if (tLeft + tCharSize.Width >= TexWidth - 1)
                    {
                        tLine++;
                        tLeft = 0;

                        x.Location = new Point(tLeft, x.Location.Y);
                        tLineDict.Add(x, tLine);

                        tLeft += tCharSize.Width;
                    }
                    else
                    {
                        x.Location = new Point(tLeft, x.Location.Y);
                        tLineDict.Add(x, tLine);

                        tLeft += tCharSize.Width;
                    }

                    // 处理行高
                    int tCurLineHeight = tCharSize.Height;
                    if (!tLineHeight.ContainsKey(tLine))
                        tLineHeight.Add(tLine, tCurLineHeight);
                    else
                    {
                        if (tCurLineHeight > tLineHeight[tLine])
                            tLineHeight[tLine] = tCurLineHeight;
                    }
                }

                // 计算行基线
                int[] tBaseLine = new int[tLineHeight.Count];
                foreach (KeyValuePair<int, int> y in tLineHeight)
                {
                    if (y.Key == 0)
                        tBaseLine[0] = y.Value;
                    else
                        tBaseLine[y.Key] = tBaseLine[y.Key - 1] + y.Value;

                    if (_MaxLineHeight < y.Value)
                        _MaxLineHeight = y.Value;
                }

                // 第二次遍历，计算Location.y并绘图
                foreach (CharDef x in _CharList)
                {
                    x.Location = new Point(x.Location.X, tBaseLine[tLineDict[x]] - x.CharSize.Height);

                    if (x.Character != '\0' && x.SrcFont != null)
                        tGraph.DrawString(String.Format("{0}", x.Character), x.SrcFont, new SolidBrush(x.FontColor), new PointF((float)(x.Location.X + x.ImageMargin), (float)(x.Location.Y + x.ImageMargin)), StringFormat.GenericTypographic);
                }
            }

            _FontTexture = tTexture; 
        }

        public void SaveTo(string FilePath)
        {
            XmlDocument tDoc = new XmlDocument();

            XmlElement tRoot = tDoc.CreateElement("FontDef");
            tRoot.SetAttribute("TexWidth", TexWidth.ToString());
            tRoot.SetAttribute("TexHeight", TexHeight.ToString());
            tRoot.SetAttribute("AntiAlias", ((int)AntiAlias).ToString());

            foreach (FontDef.CharDef x in _CharList)
            {
                XmlElement tElement = tDoc.CreateElement("Char");

                tElement.SetAttribute("Advance", x.Advance.ToString());
                tElement.SetAttribute("Baseline", x.Baseline.ToString());
                tElement.SetAttribute("Character", x.Character.ToString());
                tElement.SetAttribute("ImageMargin", x.ImageMargin.ToString());
                tElement.SetAttribute("FontColor", x.FontColor.ToArgb().ToString());

                if (x.SrcFont != null)
                {
                    tElement.SetAttribute("Bold", x.SrcFont.Bold.ToString());
                    tElement.SetAttribute("Italic", x.SrcFont.Italic.ToString());
                    tElement.SetAttribute("Strikeout", x.SrcFont.Strikeout.ToString());
                    tElement.SetAttribute("Underline", x.SrcFont.Underline.ToString());
                    tElement.SetAttribute("FontName", x.SrcFont.FontFamily.Name);
                    tElement.SetAttribute("FontSize", x.SrcFont.Size.ToString());
                }
                else
                {
                    tElement.SetAttribute("Bold", "False");
                    tElement.SetAttribute("Italic", "False");
                    tElement.SetAttribute("Strikeout", "False");
                    tElement.SetAttribute("Underline", "False");
                    tElement.SetAttribute("FontName", "宋体");
                    tElement.SetAttribute("FontSize", "12");
                }

                tRoot.AppendChild(tElement);
            }

            tDoc.AppendChild(tRoot);

            tDoc.Save(FilePath);
        }

        public FontDef()
        {
            RenderTexture();
        }

        public FontDef(string FilePath)
        {
            XmlDocument tDoc = new XmlDocument();
            tDoc.Load(FilePath);

            XmlElement tRoot = tDoc.DocumentElement;
            _TexWidth = Convert.ToInt32(tRoot.GetAttribute("TexWidth"));
            _TexHeight = Convert.ToInt32(tRoot.GetAttribute("TexHeight"));
            _AntiAlias = (System.Drawing.Text.TextRenderingHint)Convert.ToInt32(tRoot.GetAttribute("AntiAlias"));

            foreach (XmlNode x in tRoot)
            {
                XmlElement tObj = x as XmlElement;
                if (tObj != null)
                {
                    CharDef tChar = new CharDef();
                    tChar.Advance = Convert.ToInt32(tObj.GetAttribute("Advance"));
                    tChar.Baseline = Convert.ToInt32(tObj.GetAttribute("Baseline"));
                    tChar.Character = Convert.ToChar(tObj.GetAttribute("Character"));
                    tChar.ImageMargin = Convert.ToInt32(tObj.GetAttribute("ImageMargin"));
                    tChar.FontColor = Color.FromArgb(Convert.ToInt32(tObj.GetAttribute("FontColor")));
                    
                    FontStyle tStyle = FontStyle.Regular;
                    if(Convert.ToBoolean(tObj.GetAttribute("Bold")))
                        tStyle = FontStyle.Bold;
                    if(Convert.ToBoolean(tObj.GetAttribute("Italic")))
                        tStyle |= FontStyle.Italic;
                    if (Convert.ToBoolean(tObj.GetAttribute("Strikeout")))
                        tStyle |= FontStyle.Strikeout;
                    if (Convert.ToBoolean(tObj.GetAttribute("Underline")))
                        tStyle |= FontStyle.Underline;
                    tChar.SrcFont = new Font(tObj.GetAttribute("FontName"), Convert.ToSingle(tObj.GetAttribute("FontSize")), tStyle);

                    _CharList.Add(tChar);
                }
            }

            RenderTexture();
        }
    }
}
