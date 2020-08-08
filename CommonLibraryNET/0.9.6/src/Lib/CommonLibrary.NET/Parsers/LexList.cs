/*
 * Author: Kishore Reddy
 * Url: http://commonlibrarynet.codeplex.com/
 * Title: CommonLibrary.NET
 * Copyright: � 2009 Kishore Reddy
 * License: LGPL License
 * LicenseUrl: http://commonlibrarynet.codeplex.com/license
 * Description: A C# based .NET 3.5 Open-Source collection of reusable components.
 * Usage: Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;



namespace ComLib.Parsing
{
    /// <summary>
    /// Lexical parser used to parsing text.
    /// e.g. Such as parsing arguments to a program.
    /// </summary>
    /// <example>
    ///     IList<string> items = LexListParser.Parse("'firstname', 25.8, 3/20/2009, true, "lastname" ");
    /// </example>
    public class LexList : LexBase
    {
        #region Private members
        protected List<List<string>> _lines;
        protected IDictionary<string, string> _separatorMap;
        protected static LexListSettings _defaultSettings = new LexListSettings(); 
        #endregion

        /// <summary>
        /// Parse supplied text using default settings.
        /// </summary>
        /// <param name="line"></param>
        /// <returns></returns>
        public static IDictionary<string, string> Parse(string line)
        {
            return Parse(line, _defaultSettings)[0].ToDictionary();
        }


        /// <summary>
        /// Parse supplied text using default settings.
        /// </summary>
        /// <param name="line"></param>
        /// <returns></returns>
        public static List<string> ParseList(string line)
        {
            return Parse(line, _defaultSettings)[0];
        }


        /// <summary>
        /// Parse supplied text using default settings.
        /// </summary>
        /// <param name="line"></param>
        /// <returns></returns>
        public static List<List<string>> ParseTable(string line)
        {
            LexListSettings settings = new LexListSettings();
            settings.MultipleRecordsUsingNewLine = true;
            return Parse(line, settings);
        }


        /// <summary>
        /// Parse supplied text using supplied settings.
        /// </summary>
        /// <param name="line"></param>
        /// <param name="settings"></param>
        /// <returns></returns>
        public static List<List<string>> Parse(string text, LexListSettings settings)
        {
            LexList lex = new LexList(settings);
            return lex.ParseLines(text);
        }

    
        /// <summary>
        /// Create using default settings.
        /// </summary>
        public LexList()
        {
            Init(_defaultSettings);
        }


        /// <summary>
        /// Create with supplied settings.
        /// </summary>
        /// <param name="settings"></param>
        public LexList(LexListSettings settings)
        {
            Init(settings);            
        }


        public override void Init(LexSettings settings)
        {
            base.Init(settings);
            _separatorMap = new Dictionary<string, string>();
            _separatorMap[","] = ",";
            if (settings is LexListSettings)
            {
                var listsettings = (LexListSettings)settings;
                _separatorMap.Clear();
                _separatorMap[listsettings.Delimeter] = listsettings.Delimeter;
            }
        }


        /// <summary>
        /// Parse 
        /// </summary>
        /// <param name="line"></param>
        /// <returns></returns>
        public List<List<string>> ParseLines(string text)
        {
            // First cut of lex parser.
            // This needs to be handled differently and should allow whitespace to 
            // be a separator.
            LexListSettings settings = _settings as LexListSettings;
            Reset(text);

            // Move to first char.
            _reader.ReadChar();

            // Get rid of initial whitespace.
            _reader.ConsumeWhiteSpace();

            while (!_reader.IsEnd())
            {
                if (_reader.IsWhiteSpace())
                {
                    _reader.ConsumeWhiteSpace();

                    // Continue check.
                    // The consuming of white space always leaves
                    // the reader at the beginning of a non-whitespace char.
                    continue;
                }
                // Check for quotes.
                else if (_reader.CurrentChar == "'" || _reader.CurrentChar == "\"" )
                {
                    ParseQuotedItem(settings);               
                }
                else
                {
                    ParseNonQuotedItem(settings);
                }                

                // Check for and consume whitespace.
                CheckAndConsumeWhiteSpace();

                // Check for comma and consume it.
                CheckAndHandleComma();

                // Consume whitespace
                CheckAndConsumeWhiteSpace();

                // Handle new lines and whether to store
                // next line as another record.
                if (!CheckAndHandleNewLine(settings))
                {
                    // Now read next char
                    _reader.ReadChar();
                }
            }
            
            // Handle errors.
            CheckAndThrowErrors();

            // Token list always gets reset in the beginning of this method.
            if (_tokenList.Count > 0)
                _lines.Add(_tokenList);

            return _lines;
        }


        protected override void Reset(string line)
        {
            base.Reset(line);
            _lines = new List<List<string>>();
        }


        /// <summary>
        /// Parse a quoted item. e.g. "batman"
        /// </summary>
        /// <param name="settings"></param>
        private void ParseQuotedItem(LexListSettings settings)
        {
            string quote = _reader.CurrentChar;
            string item = ReadQuotedToken();

            if (settings.TrimWhiteSpace)
                item = item.Trim();

            // Store the current list item.
            _tokenList.Add(item);

            // Check that closing quote is present.
            if (Expect(quote))
                _reader.ReadChar();
        }


        private bool CheckAndHandleComma()
        {
            if (_separatorMap.ContainsKey(_reader.CurrentChar))
                return true;

            // Read past quote for "," or newline.
            string nextChar = _reader.PeekChar();
            
            // If "," return and the parser continue.
            if (_separatorMap.ContainsKey(nextChar))
            {
                _reader.ReadChar();
                return true;
            }
            return false;
        }


        /// <summary>
        /// Check and consume whitespace.
        /// </summary>
        /// <returns></returns>
        public bool CheckAndConsumeWhiteSpace()
        {
            // At this point, it's either whitespace or "new line"
            if (_reader.IsWhiteSpace())
            {
                _reader.ConsumeWhiteSpace();
                return true;
            }
            return false;
        }


        /// <summary>
        /// Check for and handle new line.
        /// </summary>
        /// <returns></returns>
        public bool CheckAndHandleNewLine(LexListSettings settings)
        {
            if (_reader.IsEol())
            {
                _reader.ConsumeNewLine();

                // Check if newline means start of new record.
                if (settings.MultipleRecordsUsingNewLine)
                {
                    _lines.Add(_tokenList);
                    _tokenList = new List<string>();
                }
                return true;
            }
            return false;
        }


        /// <summary>
        /// Has more text.
        /// </summary>
        /// <returns></returns>
        public bool HasMore()
        {
            //  Can't do anything if at end or past it.
            if (_reader.IsAtEnd() || _reader.IsEnd())
                return false;
            return true;
        }


        /// <summary>
        /// Parse a non-quoted item e.g. 123 as opposed to "123"
        /// </summary>
        private void ParseNonQuotedItem(LexListSettings settings)
        {
            // Read non-quoted text until the separator char is hit.  
            string item = ReadNonQuotedToken(_separatorMap, true);          
            
            if (settings.TrimWhiteSpace)
                item = item.Trim();

            // Store the current list item.
            _tokenList.Add(item);
        }
    }



    /// <summary>
    /// Parse settings for the Lexical List parser.
    /// </summary>
    public class LexListSettings : LexSettings
    {
        public string Delimeter = ",";

        /// <summary>
        /// Indicates whether or not to trim the white space if the
        /// separator is not a whitespace char. e.g. if "," trim white space.
        /// </summary>
        public bool TrimWhiteSpace = true;

        /// <summary>
        /// Flag indicating whether or not to handle multiple lines as a single record.
        /// Otherwise, new lines indicate end of record.
        /// </summary>
        public bool MultipleRecordsUsingNewLine = false;


        public bool AllowNewLineInFirstLine = false;


        public bool AllowNewLinesAsTextOnlyAfterFirstLine = true;
    }
}
