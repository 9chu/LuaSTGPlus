using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace mbg2luastg
{
    /// <summary>
    /// CrazyStorm 1 弹幕配置文件解析器
    /// </summary>
    public class MBGParser
    {
        /// <summary>
        /// 错误的文件格式
        /// </summary>
        public class InvalidFileFormat : Exception
        {
            public InvalidFileFormat(int line, string message)
                : base(String.Format("行 {0}: {1}", line, message)) {}
        }

        /// <summary>
        /// 行读取器
        /// </summary>
        private class LineParser
        {

        }

        void parse(string[] content)
        {
            // 读取文件版本号
            if (content.Length < 1 || content[0] != "Crazy Storm Data 1.01")
                throw new InvalidFileFormat(content.Length, "文件格式无效，需要Crazy Storm 1.01版本");

            // 读取中心数据

        }

        public MBGParser(string path)
        {
            parse(File.ReadAllLines(path, Encoding.UTF8));

        }
    }
}
