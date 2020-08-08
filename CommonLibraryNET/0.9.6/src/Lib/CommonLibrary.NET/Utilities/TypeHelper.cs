using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib
{
    public class TypeHelper
    {
        private static IDictionary<string, bool> _numericTypes;
        private static IDictionary<string, bool> _basicTypes;

        static TypeHelper()
        {
            _numericTypes = new Dictionary<string, bool>();
            _numericTypes[typeof(int).Name] = true;
            _numericTypes[typeof(long).Name] = true;
            _numericTypes[typeof(float).Name] = true;
            _numericTypes[typeof(double).Name] = true;
            _numericTypes[typeof(decimal).Name] = true;
            _numericTypes[typeof(sbyte).Name] = true;
            _numericTypes[typeof(Int16).Name] = true;
            _numericTypes[typeof(Int32).Name] = true;
            _numericTypes[typeof(Int64).Name] = true;
            _numericTypes[typeof(Double).Name] = true;
            _numericTypes[typeof(Decimal).Name] = true;

            _basicTypes = new Dictionary<string, bool>();
            _basicTypes[typeof(int).Name] = true;
            _basicTypes[typeof(long).Name] = true;
            _basicTypes[typeof(float).Name] = true;
            _basicTypes[typeof(double).Name] = true;
            _basicTypes[typeof(decimal).Name] = true;
            _basicTypes[typeof(sbyte).Name] = true;
            _basicTypes[typeof(Int16).Name] = true;
            _basicTypes[typeof(Int32).Name] = true;
            _basicTypes[typeof(Int64).Name] = true;
            _basicTypes[typeof(Double).Name] = true;
            _basicTypes[typeof(Decimal).Name] = true;
            _basicTypes[typeof(bool).Name] = true;
            _basicTypes[typeof(DateTime).Name] = true;            
            _basicTypes[typeof(string).Name] = true;            
        }


        public static bool IsNumeric(object val)
        {
            return _numericTypes.ContainsKey(val.GetType().Name);
        }


        public static bool IsNumeric(Type type)
        {
            return _numericTypes.ContainsKey(type.Name);
        }


        public static bool IsBasicType(Type type)
        {
            return _basicTypes.ContainsKey(type.Name);
        }


        public static string Join(object[] vals)
        {
            StringBuilder buffer = new StringBuilder();
            Type arrayType = vals[0].GetType();
            if (arrayType == typeof(int[]))
            {
                int[] iarray = vals[0] as int[];
                buffer.Append(iarray[0].ToString());
                for (int ndx = 1; ndx < iarray.Length; ndx++)
                    buffer.Append(", " + iarray[ndx]);
            }
            else if (arrayType == typeof(long[]))
            {
                long[] iarray = vals[0] as long[];
                buffer.Append(iarray[0].ToString());
                for (int ndx = 1; ndx < iarray.Length; ndx++)
                    buffer.Append(", " + iarray[ndx]);
            }
            else if (arrayType == typeof(float[]))
            {
                float[] iarray = vals[0] as float[];
                buffer.Append(iarray[0].ToString());
                for (int ndx = 1; ndx < iarray.Length; ndx++)
                    buffer.Append(", " + iarray[ndx]);
            }
            else if (arrayType == typeof(double[]))
            {
                double[] iarray = vals[0] as double[];
                buffer.Append(iarray[0].ToString());
                for (int ndx = 1; ndx < iarray.Length; ndx++)
                    buffer.Append(", " + iarray[ndx]);
            }
            else
            {
                buffer.Append(vals[0].ToString());
                for (int ndx = 1; ndx < vals.Length; ndx++)
                    buffer.Append(", " + vals[ndx]);
            }
            return buffer.ToString();
        }
    }
}
