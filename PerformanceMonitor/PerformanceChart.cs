using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace PerformanceMonitor
{
    public partial class PerformanceChart : UserControl
    {
        /// <summary>
        /// 性能参数
        /// </summary>
        public enum PerformanceArg
        {
            CPU = 0,
            Memory,
            FPS,
            Objects,
            FrameTime,
            RenderTime
        }

        private const int LogMaxCount = 100;
        private const float DefaultPenSize = 3;
        private float[,] _PerformanceData = new float[6, LogMaxCount];
        private int[] _PerformanceDataCount = new int[6];
        private float[] _PerformaceUnit = new float[6];  // 性能最小显示单位
        private float[] _PerformaceMax = new float[6];  // 性能显示的上限值
        private Pen[] _PerformancePen = new Pen[6];  // 绘制画笔

        public PerformanceChart()
        {
            InitializeComponent();

            _PerformancePen[0] = new Pen(Color.LightSkyBlue, DefaultPenSize);
            _PerformancePen[1] = new Pen(Color.BlueViolet, DefaultPenSize);
            _PerformancePen[2] = new Pen(Color.ForestGreen, DefaultPenSize);
            _PerformancePen[3] = new Pen(Color.Maroon, DefaultPenSize);
            _PerformancePen[4] = new Pen(Color.LightSalmon, DefaultPenSize);
            _PerformancePen[5] = new Pen(Color.Peru, DefaultPenSize);

            Reset();
        }

        /// <summary>
        /// 清空性能分析器
        /// </summary>
        public void Reset()
        {
            // 初始化参数
            _PerformaceUnit[0] = 100;  // CPU显示单位
            _PerformaceUnit[1] = 1000 * 1024 * 1024;  // 内存显示单位
            _PerformaceUnit[2] = 80;  // FPS显示单位
            _PerformaceUnit[3] = 1000;  // 对象显示单位
            _PerformaceUnit[4] = 1;  // 帧逻辑时间
            _PerformaceUnit[5] = 1;  // 帧渲染时间

            for (int i = 0; i < _PerformaceMax.Count(); ++i)
                _PerformaceMax[i] = _PerformaceUnit[i];

            // 通知控件刷新
            Invalidate();
        }

        /// <summary>
        /// 将数据添加到性能分析器中
        /// </summary>
        /// <param name="arg">参数</param>
        /// <param name="value">值</param>
        public void AddPerformance(PerformanceArg arg, float value)
        {
            int tIndex = (int)arg;

            if (_PerformanceDataCount[tIndex] == LogMaxCount)
            {
                for (int i = 1; i < LogMaxCount; ++i)
                {
                    // 前移一个数据，腾出最后一个位置
                    _PerformanceData[tIndex, i - 1] = _PerformanceData[tIndex, i];
                }
                _PerformanceData[tIndex, LogMaxCount - 1] = value;
            }
            else
                _PerformanceData[tIndex, _PerformanceDataCount[tIndex]++] = value;

            // 检查数据最大值
            float tMaxValue = 0;
            for (int i = 0; i < _PerformanceDataCount[tIndex]; ++i)
                tMaxValue = Math.Max(tMaxValue, _PerformanceData[tIndex, i]);
            if (tMaxValue != 0)
                _PerformaceMax[tIndex] = _PerformaceUnit[tIndex] * (float)Math.Ceiling(tMaxValue / _PerformaceUnit[tIndex]);
            else
                _PerformaceMax[tIndex] = _PerformaceUnit[tIndex];

            // 通知控件刷新
            Invalidate();
        }

        private void PerformanceChart_Load(object sender, EventArgs e)
        {
        }

        private void PerformanceChart_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;

            float tXUnit = Width / (float)LogMaxCount;
            for (int i = 0; i < 6; ++i)
            {
                float tPerformanceMax = _PerformaceMax[i];

                for (int j = 1; j < _PerformanceDataCount[i]; ++j)
                {
                    g.DrawLine(_PerformancePen[i],
                        (j - 1) * tXUnit,
                        (1 - _PerformanceData[i, j - 1] / tPerformanceMax) * Height,
                        j * tXUnit,
                        (1 - _PerformanceData[i, j] / tPerformanceMax) * Height
                        );
                }
            }
        }
    }
}
