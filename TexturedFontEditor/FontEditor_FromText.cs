using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace TexturedFontEditor
{
    public partial class FontEditor_FromText : Form
    {
        public static DialogResult OpenDialog(FontDef Def, IList<FontDef.CharDef> SelectedChars)
        {
            FontEditor_FromText tForm = new FontEditor_FromText(Def, SelectedChars);
            return tForm.ShowDialog();
        }

        FontDef _Def;
        IList<FontDef.CharDef> _SelectedChars;

        void CalcuCount()
        {
            HashSet<char> tList = new HashSet<char>();
            int tAdd = 0;

            // 统计文本
            foreach(char x in textBox1.Text)
            {
                if (Char.IsControl(x))
                    continue;

                if(!tList.Contains(x))
                {
                    tList.Add(x);

                    bool tAddable = true;
                    if (checkBox2.Checked)
                    {
                        foreach (FontDef.CharDef y in _Def.CharList)
                        {
                            if (y.Character == x)
                            {
                                tAddable = false;
                                break;
                            }
                        }
                    }

                    if(tAddable)
                        tAdd++;
                }
            }

            label2.Text = String.Format(
                "可添加 {0} 个字符，可用空位 {1} 个字符，实际添加 {2} 个字符。",
                tList.Count,
                _SelectedChars.Count,
                checkBox3.Checked ? tAdd : (!checkBox1.Checked ? (_SelectedChars.Count < tAdd ? _SelectedChars.Count : tAdd) : tAdd)
                );
        }

        public FontEditor_FromText(FontDef Def, IList<FontDef.CharDef> SelectedChars)
        {
            InitializeComponent();

            _Def = Def;
            _SelectedChars = SelectedChars;
        }

        private void FontEditor_FromText_Load(object sender, EventArgs e)
        {
            CalcuCount();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            fontDialog1.ShowDialog();
        }

        private void button4_Click(object sender, EventArgs e)
        {
            colorDialog1.ShowDialog();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            // 收集所有字符
            HashSet<char> tList = new HashSet<char>();
            
            // 统计文本
            foreach (char x in textBox1.Text)
            {
                if (Char.IsControl(x))
                    continue;

                if (!tList.Contains(x))
                {
                    bool tAddable = true;
                    if (checkBox2.Checked)
                    {
                        foreach (FontDef.CharDef y in _Def.CharList)
                        {
                            if (y.Character == x)
                            {
                                tAddable = false;
                                break;
                            }
                        }
                    }

                    if (tAddable)
                        tList.Add(x);
                }
            }

            // 填充字符
            foreach (char x in tList)
            {
                if (_SelectedChars.Count > 0 && !checkBox3.Checked)
                {
                    _SelectedChars[0].SrcFont = fontDialog1.Font;
                    _SelectedChars[0].FontColor = colorDialog1.Color;
                    _SelectedChars[0].Character = x;
                    if (_SelectedChars[0].Character == ' ')
                        _SelectedChars[0].Advance = (int)(fontDialog1.Font.SizeInPoints * 0.8);

                    _SelectedChars.RemoveAt(0);
                }
                else
                {
                    if (checkBox1.Checked || checkBox3.Checked)
                    {
                        FontDef.CharDef tDef = new FontDef.CharDef();
                        tDef.SrcFont = fontDialog1.Font;
                        tDef.FontColor = colorDialog1.Color;
                        tDef.Character = x;
                        if (tDef.Character == ' ')
                            tDef.Advance = (int)(fontDialog1.Font.SizeInPoints * 0.8);

                        _Def.CharList.Add(tDef);
                    }
                    else
                        break;
                }
            }

            DialogResult = DialogResult.OK;
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            CalcuCount();
        }

        private void checkBox1_CheckStateChanged(object sender, EventArgs e)
        {
            CalcuCount();
        }

        private void checkBox2_CheckStateChanged(object sender, EventArgs e)
        {
            CalcuCount();
        }

        private void checkBox3_CheckStateChanged(object sender, EventArgs e)
        {
            if (checkBox3.Checked)
            {
                checkBox1.Enabled = false;
            }
            else
            {
                checkBox1.Enabled = true;
            }

            CalcuCount();
        }
    }
}
