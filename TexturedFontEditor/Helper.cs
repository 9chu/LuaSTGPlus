using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.ComponentModel;
using System.Drawing;
using System.Runtime;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Globalization;
using System.Collections;
using System.IO;

namespace TexturedFontEditor
{
    public abstract class Helper
    {
        public static void ShowErr(string Msg, string Caption = "错误")
        {
            MessageBox.Show(Msg, Caption, MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        public static bool IsXmlHasElement(XmlElement Xml, string TagName)
        {
            XmlNodeList tList = Xml.GetElementsByTagName(TagName);

            if (tList.Count == 0)
                return false;
            else
                return true;
        }

        public static XmlElement GetFirstElement(XmlElement Xml, string TagName)
        {
            XmlNodeList tList = Xml.GetElementsByTagName(TagName);

            if (tList.Count == 0)
                throw new Exception("找不到元素'"+TagName+"'.");
            else
                return (XmlElement)tList[0];
        }

        public static string GetXmlResValue(XmlElement Xml, string TagName, string Attribute = "Value")
        {
            XmlElement tFirstElemet = GetFirstElement(Xml, TagName);

            if (!tFirstElemet.HasAttribute(Attribute))
                throw new Exception("找不到属性'"+Attribute+"'于节点'"+TagName+"'.");

            return tFirstElemet.GetAttribute(Attribute);
        }

        public static PointF StringToPointF(string Data)
        {
            string[] tRectArr = Data.Split(',');

            if (tRectArr.Length != 2)
                throw new Exception("格式不正确: 请确保为'x,y'.");

            return new PointF(
                Convert.ToSingle(tRectArr[0]),
                Convert.ToSingle(tRectArr[1])
                );
        }

        public static RectangleF StringToRectangleF(string Data)
        {
            string[] tRectArr = Data.Split(',');

            if (tRectArr.Length != 4)
                throw new Exception("格式不正确: 请确保为'x1,y1,x2,y2'.");

            return new RectangleF(
                Convert.ToSingle(tRectArr[0]),
                Convert.ToSingle(tRectArr[1]),
                Convert.ToSingle(tRectArr[2]) - Convert.ToSingle(tRectArr[0]),
                Convert.ToSingle(tRectArr[3]) - Convert.ToSingle(tRectArr[1])
            );
        }

        public static Color StringToColor(string Data)
        {
            string[] tColorArr = Data.Split(',');

            if (tColorArr.Length != 4)
                throw new Exception("属性格式不正确: 请确保为'a,r,g,b'.");

            return Color.FromArgb(
                Convert.ToInt32(tColorArr[0]),
                Convert.ToInt32(tColorArr[1]),
                Convert.ToInt32(tColorArr[2]),
                Convert.ToInt32(tColorArr[3])
                );
        }

        public static PointF XmlElementToPointF(XmlElement Node, string NodeName, string Attribute="Value")
        {
            string[] tRectArr = Helper.GetXmlResValue(Node, NodeName, Attribute).Split(',');

            if (tRectArr.Length != 2)
                throw new Exception("'" + NodeName + "'属性格式不正确: 请确保为'x,y'.");

            return new PointF(
                Convert.ToSingle(tRectArr[0]),
                Convert.ToSingle(tRectArr[1])
                );
        }

        public static Color XmlElementToColor(XmlElement Node, string NodeName, string Attribute = "Value")
        {
            string[] tRectArr = Helper.GetXmlResValue(Node, NodeName, Attribute).Split(',');

            if (tRectArr.Length != 4)
                throw new Exception("'" + NodeName + "'属性格式不正确: 请确保为'a,r,g,b'.");

            return Color.FromArgb(
                Convert.ToInt32(tRectArr[0]),
                Convert.ToInt32(tRectArr[1]),
                Convert.ToInt32(tRectArr[2]),
                Convert.ToInt32(tRectArr[3])
                );
        }

        public static RectangleF XmlElementToRectangleF(XmlElement Node, string NodeName, string Attribute = "Value")
        {
            string[] tRectArr = Helper.GetXmlResValue(Node, NodeName, Attribute).Split(',');

            if (tRectArr.Length != 4)
                throw new Exception("'" + NodeName + "'属性格式不正确: 请确保为'x1,y1,x2,y2'.");

            return new RectangleF(
                Convert.ToSingle(tRectArr[0]),
                Convert.ToSingle(tRectArr[1]),
                Convert.ToSingle(tRectArr[2]) - Convert.ToSingle(tRectArr[0]),
                Convert.ToSingle(tRectArr[3]) - Convert.ToSingle(tRectArr[1])
            );
        }

        public static void WriteFSTGXmlType(XmlDocument XmlDoc, string TargetFile)
        {
            FileStream tFile = new FileStream(TargetFile, FileMode.Create, FileAccess.Write, FileShare.Read);
            
            try
            {
                XmlWriterSettings tSetting = new XmlWriterSettings();
                tSetting.CloseOutput = false;
                tSetting.Encoding = Encoding.UTF8;
                tSetting.Indent = true;
                tSetting.IndentChars = "  ";
                tSetting.NewLineOnAttributes = false;
                tSetting.OmitXmlDeclaration = true;

                XmlWriter tWritter = XmlWriter.Create(tFile, tSetting);

                XmlDoc.Save(tWritter);
            }
            catch
            {
                tFile.Close();

                throw;
            }

            tFile.Close();
        }
    }

    public class PointFConverter : TypeConverter
    {
        // Methods
        public PointFConverter()
        {
        }
        public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
        {
            return ((sourceType == typeof(string)) || base.CanConvertFrom(context, sourceType));
        }
        public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
        {
            return ((destinationType == typeof(PointF)) || base.CanConvertTo(context, destinationType));
        }
        public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
        {
            if (culture == null)
                culture = CultureInfo.CurrentCulture;

            if (!(value is string))
                return base.ConvertFrom(context, culture, value);

            string text = ((string)value).Trim();
            if (text.Length == 0)
                return null;
            
            char ch = culture.TextInfo.ListSeparator[0];
            string[] textArray = text.Split(new char[] { ch });
            float[] numArray = new float[textArray.Length];
            if (numArray.Length != 2)
                throw new ArgumentException("格式不正确");

            TypeConverter converter = TypeDescriptor.GetConverter(typeof(float));
            for (int i = 0; i < numArray.Length; i++)
                numArray[i] = (float)converter.ConvertFromString(context, culture, textArray[i]);
            
            return new PointF(numArray[0], numArray[1]);
        }
        public override object ConvertTo(ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
        {
            if (culture == null)
                culture = CultureInfo.CurrentCulture;
            if (destinationType == null)
                throw new ArgumentNullException("destinationType");

            if ((destinationType == typeof(string)) && (value is PointF))
            {
                PointF pointf = (PointF)value;
                
                string separator = culture.TextInfo.ListSeparator + " ";
                TypeConverter converter = TypeDescriptor.GetConverter(typeof(float));
                string[] textArray = new string[2];
                int num = 0;
                textArray[num++] = converter.ConvertToString(context, culture, pointf.X);
                textArray[num++] = converter.ConvertToString(context, culture, pointf.Y);
                return string.Join(separator, textArray);
            }

            return base.ConvertTo(context, culture, value, destinationType);
        }
        public override object CreateInstance(ITypeDescriptorContext context, IDictionary propertyValues)
        {
            return new PointF((float)propertyValues["X"], (float)propertyValues["Y"]);
        }
        public override bool GetCreateInstanceSupported(ITypeDescriptorContext context)
        {
            return true;
        }
        public override PropertyDescriptorCollection GetProperties(ITypeDescriptorContext context, object value, Attribute[] attributes)
        {
            return TypeDescriptor.GetProperties(typeof(PointF), attributes).Sort(new string[] { "X", "Y" });
        }
        public override bool GetPropertiesSupported(ITypeDescriptorContext context)
        {
            return true;
        }
    }

    public class RectangleFConverter : TypeConverter
    {
        // Methods
        public RectangleFConverter()
        {
        }
        public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
        {
            return ((sourceType == typeof(string)) || base.CanConvertFrom(context, sourceType));
        }
        public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
        {
            return ((destinationType == typeof(RectangleF)) || base.CanConvertTo(context, destinationType));
        }
        public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
        {
            if (culture == null)
                culture = CultureInfo.CurrentCulture;

            if (!(value is string))
                return base.ConvertFrom(context, culture, value);

            string text = ((string)value).Trim();
            if (text.Length == 0)
                return null;

            char ch = culture.TextInfo.ListSeparator[0];
            string[] textArray = text.Split(new char[] { ch });
            float[] numArray = new float[textArray.Length];
            if (numArray.Length != 4)
                throw new ArgumentException("格式不正确");

            TypeConverter converter = TypeDescriptor.GetConverter(typeof(float));
            for (int i = 0; i < numArray.Length; i++)
                numArray[i] = (float)converter.ConvertFromString(context, culture, textArray[i]);

            return new RectangleF(numArray[0], numArray[1], numArray[2], numArray[3]);
        }
        public override object ConvertTo(ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
        {
            if (culture == null)
                culture = CultureInfo.CurrentCulture;
            if (destinationType == null)
                throw new ArgumentNullException("destinationType");

            if ((destinationType == typeof(string)) && (value is RectangleF))
            {
                RectangleF pointf = (RectangleF)value;

                string separator = culture.TextInfo.ListSeparator + " ";
                TypeConverter converter = TypeDescriptor.GetConverter(typeof(float));
                string[] textArray = new string[4];
                int num = 0;
                textArray[num++] = converter.ConvertToString(context, culture, pointf.X);
                textArray[num++] = converter.ConvertToString(context, culture, pointf.Y);
                textArray[num++] = converter.ConvertToString(context, culture, pointf.Width);
                textArray[num++] = converter.ConvertToString(context, culture, pointf.Height);
                return string.Join(separator, textArray);
            }

            return base.ConvertTo(context, culture, value, destinationType);
        }
        public override object CreateInstance(ITypeDescriptorContext context, IDictionary propertyValues)
        {
            return new RectangleF((float)propertyValues["X"], (float)propertyValues["Y"], (float)propertyValues["Width"], (float)propertyValues["Height"]);
        }
        public override bool GetCreateInstanceSupported(ITypeDescriptorContext context)
        {
            return true;
        }
        public override PropertyDescriptorCollection GetProperties(ITypeDescriptorContext context, object value, Attribute[] attributes)
        {
            return TypeDescriptor.GetProperties(typeof(RectangleF), attributes).Sort(new string[] { "X", "Y", "Width", "Height" });
        }
        public override bool GetPropertiesSupported(ITypeDescriptorContext context)
        {
            return true;
        }
    }

    public class ColorConverter : TypeConverter
    {
        // Methods
        public ColorConverter()
        {
        }
        public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
        {
            return ((sourceType == typeof(string)) || base.CanConvertFrom(context, sourceType));
        }
        public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
        {
            return ((destinationType == typeof(Color)) || base.CanConvertTo(context, destinationType));
        }
        public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
        {
            if (culture == null)
                culture = CultureInfo.CurrentCulture;

            if (!(value is string))
                return base.ConvertFrom(context, culture, value);

            string text = ((string)value).Trim();
            if (text.Length == 0)
                return null;

            char ch = culture.TextInfo.ListSeparator[0];
            string[] textArray = text.Split(new char[] { ch });
            int[] numArray = new int[textArray.Length];
            if (numArray.Length != 4)
                throw new ArgumentException("格式不正确");

            TypeConverter converter = TypeDescriptor.GetConverter(typeof(int));
            for (int i = 0; i < numArray.Length; i++)
                numArray[i] = (int)converter.ConvertFromString(context, culture, textArray[i]);

            return Color.FromArgb(numArray[0], numArray[1], numArray[2], numArray[3]);
        }
        public override object ConvertTo(ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
        {
            if (culture == null)
                culture = CultureInfo.CurrentCulture;
            if (destinationType == null)
                throw new ArgumentNullException("destinationType");

            if ((destinationType == typeof(string)) && (value is Color))
            {
                Color pointf = (Color)value;

                string separator = culture.TextInfo.ListSeparator + " ";
                TypeConverter converter = TypeDescriptor.GetConverter(typeof(int));
                string[] textArray = new string[4];
                int num = 0;
                textArray[num++] = converter.ConvertToString(context, culture, pointf.A);
                textArray[num++] = converter.ConvertToString(context, culture, pointf.R);
                textArray[num++] = converter.ConvertToString(context, culture, pointf.G);
                textArray[num++] = converter.ConvertToString(context, culture, pointf.B);
                return string.Join(separator, textArray);
            }

            return base.ConvertTo(context, culture, value, destinationType);
        }
        public override object CreateInstance(ITypeDescriptorContext context, IDictionary propertyValues)
        {
            return Color.FromArgb((byte)propertyValues["A"], (byte)propertyValues["R"], (byte)propertyValues["G"], (byte)propertyValues["B"]);
        }
        public override bool GetCreateInstanceSupported(ITypeDescriptorContext context)
        {
            return true;
        }
        public override PropertyDescriptorCollection GetProperties(ITypeDescriptorContext context, object value, Attribute[] attributes)
        {
            PropertyDescriptorCollection tProperties = TypeDescriptor.GetProperties(typeof(Color), attributes);
            PropertyDescriptor[] tDesc = { tProperties["A"], tProperties["R"], tProperties["G"], tProperties["B"] };

            return new PropertyDescriptorCollection(tDesc, true);
        }
        public override bool GetPropertiesSupported(ITypeDescriptorContext context)
        {
            return true;
        }
    }
}
