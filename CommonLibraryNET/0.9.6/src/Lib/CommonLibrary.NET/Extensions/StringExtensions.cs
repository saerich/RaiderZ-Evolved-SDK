using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;


namespace ComLib.Extensions
{
    public static class StringExtensions
    {
        #region Appending
        /// <summary>
        /// Multiply a string N number of times.
        /// </summary>
        /// <param name="str"></param>
        /// <param name="times"></param>
        /// <returns></returns>
        public static string Times(this string str, int times)
        {
            if (string.IsNullOrEmpty(str)) return string.Empty;
            if (times <= 1) return str;

            string strfinal = string.Empty;
            for (int ndx = 0; ndx < times; ndx++)
                strfinal += str;

            return strfinal;
        }


        /// <summary>
        /// Increases the string to the maximum length specified.
        /// If the string is already greater than maxlength, it is truncated if the flag truncate is true.
        /// </summary>
        /// <param name="str">The STR.</param>
        /// <param name="maxLength">Length of the max.</param>
        /// <param name="truncate">if set to <c>true</c> [truncate].</param>
        /// <returns></returns>
        public static string IncreaseTo(this string str, int maxLength, bool truncate)
        {
            if (string.IsNullOrEmpty(str)) return str;
            if (str.Length == maxLength) return str;
            if (str.Length > maxLength && truncate) return str.Truncate(maxLength);

            string original = str;

            while (str.Length < maxLength)
            {
                // Still less after appending by original string.
                if (str.Length + original.Length < maxLength)
                {
                    str += original;
                }
                else // Append partial.
                {
                    str += str.Substring(0, maxLength - str.Length);
                }
            }
            return str;
        }


        /// <summary>
        /// Increases the string to the maximum length specified.
        /// If the string is already greater than maxlength, it is truncated if the flag truncate is true.
        /// </summary>
        /// <param name="str">The STR.</param>
        /// <param name="maxLength">Length of the max.</param>
        /// <param name="truncate">if set to <c>true</c> [truncate].</param>
        /// <returns></returns>
        public static string IncreaseRandomly(this string str, int minLength, int maxLength, bool truncate)
        {
            Random random = new Random(minLength);
            int randomMaxLength = random.Next(minLength, maxLength);
            return IncreaseTo(str, randomMaxLength, truncate);
        }
        #endregion


        #region Truncation
        /// <summary>
        /// Truncates the string.
        /// </summary>
        /// <param name="txt"></param>
        /// <param name="maxChars"></param>
        /// <returns></returns>
        public static string Truncate(this string txt, int maxChars)
        {
            if (string.IsNullOrEmpty(txt))
                return txt;

            if (txt.Length <= maxChars)
                return txt;

            return txt.Substring(0, maxChars);
        }


        /// <summary>
        /// Truncate the text supplied by number of characters specified by <paramref name="maxChars"/>
        /// and then appends the suffix.
        /// </summary>
        /// <param name="txt"></param>
        /// <param name="maxChars"></param>
        /// <param name="suffix"></param>
        /// <returns></returns>
        public static string TruncateWithText(this string txt, int maxChars, string suffix)
        {
            if (string.IsNullOrEmpty(txt))
                return txt;

            if (txt.Length <= maxChars)
                return txt;

            // Now do the truncate and more.
            string partial = txt.Substring(0, maxChars);
            return partial + suffix;
        }
        #endregion


        #region Conversion
        /// <summary>
        /// Convert the text  to bytes.
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static byte[] ToBytesAscii(this string txt)
        {
            if (string.IsNullOrEmpty(txt))
                return new byte[] { };

            System.Text.ASCIIEncoding encoding = new System.Text.ASCIIEncoding();
            return encoding.GetBytes(txt);
        }


        /// <summary>
        /// Converts "yes/no/true/false/0/1"
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static object ToBoolObject(this string txt)
        {
            return ToBool(txt) as object;
        }


        /// <summary>
        /// Converts "yes/no/true/false/0/1"
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static bool ToBool(this string txt)
        {            
            if (string.IsNullOrEmpty(txt))
                return false;

            string trimed = txt.Trim().ToLower();
            if (trimed == "yes" || trimed == "true" || trimed == "1")
                return true;

            return false;
        }


        /// <summary>
        /// Converts "$100 or 100"
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static object ToIntObject(this string txt)
        {
            return ToInt(txt) as object;
        }


        /// <summary>
        /// Converts "$100 or 100"
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static int ToInt(this string txt)
        {
            return ToNumber<int>(txt, (s) => Convert.ToInt32(Convert.ToDouble(s)), 0);
        }


        /// <summary>
        /// Converts "$100 or $100.50 or 100 or 100.5" to 100.
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static object ToLongObject(this string txt)
        {
            return ToLong(txt) as object;
        }


        /// <summary>
        /// Converts "$100 or $100.50 or 100 or 100.5" to 100.
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static double ToLong(this string txt)
        {
            return ToNumber<long>(txt, (s) => Convert.ToInt64(s), 0);
        }


        /// <summary>
        /// Converts "$100 or $100.50 or 100 or 100.5" to 100.
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static object ToDoubleObject(this string txt)
        {
            return ToDouble(txt) as object;
        }


        /// <summary>
        /// Converts "$100 or $100.50 or 100 or 100.5" to 100.
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static double ToDouble(this string txt)
        {
            return ToNumber<double>(txt, (s) => Convert.ToDouble(s), 0);
        }


        /// <summary>
        /// Converts "$100 or $100.50 or 100 or 100.5" to 100.
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static object ToFloatObject(this string txt)
        {
            return ToFloat(txt) as object;
        }


        /// <summary>
        /// Converts "$100 or $100.50 or 100 or 100.5" to 100.
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static double ToFloat(this string txt)
        {
            return ToNumber<float>(txt, (s) => Convert.ToSingle(s), 0);
        }


        /// <summary>
        /// Converts to a number using the callback.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="txt"></param>
        /// <param name="callback"></param>
        /// <returns></returns>
        public static T ToNumber<T>(string txt, Func<string, T> callback, T defaultValue)
        {            
            if (string.IsNullOrEmpty(txt))
                return defaultValue;

            string trimed = txt.Trim().ToLower();
            if (trimed.StartsWith("$"))
            {
                trimed = trimed.Substring(1);
            }
            return callback(trimed);
        }


        /// <summary>
        /// Converts "$100 or $100.50 or 100 or 100.5" to 100. Does not round up.
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static object ToTimeObject(this string txt)
        {
            return ToTime(txt) as object;
        }


        /// <summary>
        /// Converts "$100 or $100.50 or 100 or 100.5" to 100. Does not round up.
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static TimeSpan ToTime(this string txt)
        {
            if (string.IsNullOrEmpty(txt))
                return TimeSpan.MinValue;

            string trimmed = txt.Trim().ToLower();
            return TimeHelper.Parse(trimmed).Item;
        }


        /// <summary>
        /// Converts "$100 or $100.50 or 100 or 100.5" to 100. Does not round up.
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static object ToDateTimeObject(this string txt)
        {
            return ToDateTime(txt) as object;
        }


        /// <summary>
        /// Converts "$100 or $100.50 or 100 or 100.5" to 100. Does not round up.
        /// </summary>
        /// <param name="txt"></param>
        /// <returns></returns>
        public static DateTime ToDateTime(this string txt)
        {
            if (string.IsNullOrEmpty(txt))
                return DateTime.MinValue;

            string trimmed = txt.Trim().ToLower();
            if (trimmed.StartsWith("$"))
            {
                if (trimmed == "${today}") return DateTime.Today;
                if (trimmed == "${yesterday}") return DateTime.Today.AddDays(-1);
                if (trimmed == "${tommorrow}") return DateTime.Today.AddDays(1);
                if (trimmed == "${t}") return DateTime.Today;
                if (trimmed == "${t-1}") return DateTime.Today.AddDays(-1);
                if (trimmed == "${t+1}") return DateTime.Today.AddDays(1);
                if (trimmed == "${today+1}") return DateTime.Today.AddDays(1);
                if (trimmed == "${today-1}") return DateTime.Today.AddDays(-1);

                // Handles ${t+4} or ${t-9}
                string internalVal = trimmed.Substring(2, (trimmed.Length -1 ) - 2);
                DateTime result = DateParser.ParseTPlusMinusX(internalVal);
                return result;
            }
            DateTime parsed = DateTime.Parse(trimmed);
            return parsed;
        }
        #endregion


        #region Lists
        /// <summary>
        /// Prefixes all items in the list w/ the prefix value.
        /// </summary>
        /// <typeparam name="string">The type of the tring.</typeparam>
        /// <param name="items">The items.</param>
        /// <param name="prefix">The prefix.</param>
        public static List<string> PreFixWith(this List<string> items, string prefix)
        {
            for(int ndx = 0; ndx < items.Count; ndx++)
            {
                items[ndx] = prefix + items[ndx];
            }
            return items;
        }
        #endregion


        #region Matching
        /// <summary>
        /// Determines whether or not the string value supplied represents a "not applicable" string value by matching on na, n.a., n/a etc.
        /// </summary>
        /// <param name="val"></param>
        /// <param name="useNullOrEmptyStringAsNotApplicable"></param>
        /// <returns></returns>
        public static bool IsNotApplicableValue(this string val, bool useNullOrEmptyStringAsNotApplicable = false)
        {
            bool isEmpty = string.IsNullOrEmpty(val);
            if(isEmpty && useNullOrEmptyStringAsNotApplicable) return true;
            if(isEmpty && !useNullOrEmptyStringAsNotApplicable) return false;
            val = val.Trim().ToLower();

            if (val == "na" || val == "n.a." || val == "n/a" || val == "n\\a" || val == "n.a" || val == "not applicable")
                return true;
            return false;
        }


        /// <summary>
        /// Use the Levenshtein algorithm to determine the similarity between
        /// two strings. The higher the number, the more different the two
        /// strings are.
        /// TODO: This method needs to be rewritten to handle very large strings
        /// </summary>
        /// <param name="source">Source string to compare</param>
        /// <param name="comparison">Comparison string</param>
        /// <returns>0 if both strings are identical, otherwise a number indicating the level of difference</returns>
        /// <see cref="http://www.merriampark.com/ld.htm"/>
        /// <see cref="http://en.wikipedia.org/wiki/Levenshtein_distance"/>
        public static int Levenshtein(this string source, string comparison)
        {
            if (source == null)
            {
                throw new ArgumentNullException("source", "Can't parse null string");
            }
            if (comparison == null)
            {
                throw new ArgumentNullException("comparison", "Can't parse null string");
            }

            var s = source.ToCharArray();
            var t = comparison.ToCharArray();
            var n = source.Length;
            var m = comparison.Length;
            var d = new int[n + 1, m + 1];

            // shortcut calculation for zero-length strings
            if (n == 0) { return m; }
            if (m == 0) { return n; }

            for (var i = 0; i <= n; d[i, 0] = i++) {}
            for (var j = 0; j <= m; d[0, j] = j++) {}

            for (var i = 1; i <= n; i++)
            {
                for (var j = 1; j <= m; j++)
                {
                    var cost = t[j - 1].Equals(s[i - 1]) ? 0 : 1;

                    d[i, j] = Math.Min(Math.Min(
                        d[i - 1, j] + 1,
                        d[i, j - 1] + 1),
                        d[i - 1, j - 1] + cost);
                }
            }

            return d[n, m];
        }


        /// <summary>
        /// Calculate the simplified soundex value for the specified string.
        /// </summary>
        /// <see cref="http://en.wikipedia.org/wiki/Soundex"/>
        /// <see cref="http://west-penwith.org.uk/misc/soundex.htm"/>
        /// <param name="source">String to calculate</param>
        /// <returns>Soundex value of string</returns>
        public static string SimplifiedSoundex(this string source)
        {
            return source.SimplifiedSoundex(4);
        }

        /// <summary>
        /// Calculate the simplified soundex value for the specified string.
        /// </summary>
        /// <see cref="http://en.wikipedia.org/wiki/Soundex"/>
        /// <see cref="http://west-penwith.org.uk/misc/soundex.htm"/>
        /// <param name="source">String to calculate</param>
        /// <param name="length">Length of soundex value (typically 4)</param>
        /// <returns>Soundex value of string</returns>
        public static string SimplifiedSoundex(this string source, int length)
        {
            if (source == null) throw new ArgumentNullException("source");
            if (source.Length < 3)
            {
                throw new ArgumentException(
                    "Source string must be at least two characters", "source");
            }

            var t = source.ToUpper().ToCharArray();
            var buffer = new StringBuilder();

            short prev = -1;

            foreach (var c in t)
            {
                short curr = 0;
                switch (c)
                {
                    case 'A':
                    case 'E':
                    case 'I':
                    case 'O':
                    case 'U':
                    case 'H':
                    case 'W':
                    case 'Y':
                        curr = 0;
                        break;
                    case 'B':
                    case 'F':
                    case 'P':
                    case 'V':
                        curr = 1;
                        break;
                    case 'C':
                    case 'G':
                    case 'J':
                    case 'K':
                    case 'Q':
                    case 'S':
                    case 'X':
                    case 'Z':
                        curr = 2;
                        break;
                    case 'D':
                    case 'T':
                        curr = 3;
                        break;
                    case 'L':
                        curr = 4;
                        break;
                    case 'M':
                    case 'N':
                        curr = 5;
                        break;
                    case 'R':
                        curr = 6;
                        break;
                    default:
                        throw new ApplicationException(
                            "Invalid state in switch statement");
                }

                /* Change all consecutive duplicate digits to a single digit
                 * by not processing duplicate values. 
                 * Ignore vowels (i.e. zeros). */
                if (curr != prev)
                {
                    buffer.Append(curr);
                }

                prev = curr;
            }

            // Prefix value with first character
            buffer.Remove(0, 1).Insert(0, t.First());
            
            // Remove all vowels (i.e. zeros) from value
            buffer.Replace("0", "");
            
            // Pad soundex value with zeros until output string equals length))))
            while (buffer.Length < length) { buffer.Append('0'); }
            
            // Truncate values that are longer than the supplied length
            return buffer.ToString().Substring(0, length);
        }

        #endregion
    }
}
