using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace TexturedFontEditor
{
    public partial class Dialog_InputBox : Form
    {
        public static DialogResult OpenDialog(string TipText, string Caption, string Default, out string InputText)
        {
            Dialog_InputBox tForm = new Dialog_InputBox(TipText, Caption, Default);
            DialogResult tRet = tForm.ShowDialog();
            InputText = tForm.InputText;

            return tRet;
        }

        string _TipText;
        string _Caption;
        string _Default;

        string _InputText = String.Empty;

        public string InputText
        {
            get
            {
                return _InputText;
            }
        }

        public Dialog_InputBox(string TipText, string Caption, string Default)
        {
            InitializeComponent();

            _TipText = TipText;
            _Caption = Caption;
            _Default = Default;
        }

        private void Dialog_InputBox_Load(object sender, EventArgs e)
        {
            this.Text = _Caption;
            label1.Text = _TipText;
            textBox1.Text = _Default;
            textBox1.SelectionStart = 0;
            textBox1.SelectionLength = textBox1.Text.Length;

            textBox1.Focus();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            _InputText = textBox1.Text;
        }

        private void textBox1_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == '\r')
            {
                e.Handled = true;
                _InputText = textBox1.Text;
                DialogResult = DialogResult.OK;
            }
        }
    }
}
