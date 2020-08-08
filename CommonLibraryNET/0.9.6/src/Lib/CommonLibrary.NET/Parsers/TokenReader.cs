/*
 * Author: Kishore Reddy
 * Url: http://commonlibrarynet.codeplex.com/
 * Title: CommonLibrary.NET
 * Copyright: ? 2009 Kishore Reddy
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
using System.Text;
using System.Collections;
using System.Linq;


using ComLib;


namespace ComLib.Parsing
{
    /// <summary>
    /// Settings for the token reader class.
    /// </summary>
    public class TokenReaderSettings
    {
        /// <summary>
        /// Char used to escape.
        /// </summary>
        public string EscapeChar;


        /// <summary>
        /// Tokens
        /// </summary>
        public string[] Tokens;


        /// <summary>
        /// White space tokens.
        /// </summary>
        public string[] WhiteSpaceTokens;


        /// <summary>
        /// End of line chars.
        /// </summary>
        public string[] EolTokens;
    }



    /// <summary>
    /// Interface for a Token reader.
    /// </summary>
    public interface ITokenReader
    {
        void RegisterEol(IDictionary<string, string> eolchars);
        void RegisterWhiteSpace(IDictionary<string, string> whitespaceChars);
        void Init(string text, string escapeChar, string[] tokens, string[] whiteSpaceTokens, string[] eolTokens);
        void Init(string text, TokenReaderSettings settings);
        void Reset();

        int Position { get; }
        string PeekChar();
        string PeekChars(int count);
        string PeekLine();

        void ConsumeChar();
        void ConsumeChars(int count);
        void ConsumeWhiteSpace();
        void ConsumeWhiteSpace(bool readFirst);
        void ConsumeWhiteSpace(ref int tabCount, ref int whiteSpace);
        void ConsumeWhiteSpace(bool readFirst, bool consumeLastWhiteSpace);
        void ConsumeNewLine();
        void ConsumeNewLines();

        void ReadBackChar();
        void ReadBackChar(int count);
        
        string ReadChar();
        string ReadChars(int count);
        string ReadToEol();
        string ReadToken(string endChar, string escapeChar, bool includeEndChar, bool advanceFirst, bool expectEndChar, bool readPastEndChar);
        string ReadTokenUntil(string endChar, string escapeChar, bool advanceFirst, bool expectEndChar, bool includeEndChar, bool moveToEndChar, bool readPastEndChar);

        string CurrentChar { get; }
        string PreviousChar { get; }
        string PreviousCharAny { get; }

        int CurrentCharIndex();
        int CurrentCharInt();

        bool IsEscape();
        bool IsToken();
        bool IsEnd();
        bool IsAtEnd();
        bool IsEol();
        bool IsEol(IDictionary eolChars);
        bool IsWhiteSpace(IDictionary whitespaceChars);
        bool IsWhiteSpace();

        IDictionary<string, string> EolChars { get; }
    }



    public class TokenReader : ITokenReader
    {
        private int _pos;
        private string _text; 
        private IDictionary<string, string> _whiteSpaceChars;
        private IDictionary<string, string> _eolChars;
        private IDictionary<string, string> _tokens;
        private Dictionary<string, string> _readonlyWhiteSpaceMap;
        private Dictionary<string, string> _readonlyEolMap;
        private string _currentChar;
        private StringBuilder _backBuffer;
        private string _escapeChar;
        private int _currentCharInt;
        private int LAST_POSITION;
        private List<string> _errors = new List<string>();
        private string END_CHAR = "";



        /// <summary>
        /// Initialize
        /// </summary>
        /// <param name="text"></param>
        /// <param name="escapeChar"></param>
        /// <param name="tokens"></param>
        /// <param name="whiteSpaceTokens"></param>
        public TokenReader()
        {
            Init(string.Empty, "\\", new string[] { "\"", "'" }, new string[] { " ", "\t" }, new string[] {"\n", "\r\n"} );
        }


        /// <summary>
        /// Initialize
        /// </summary>
        /// <param name="text"></param>
        /// <param name="escapeChar"></param>
        /// <param name="tokens"></param>
        /// <param name="whiteSpaceTokens"></param>
        public TokenReader(string text, string escapeChar, string[] tokens, string[] whiteSpaceTokens, string[] eolChars)
        {
            Init(text, escapeChar, tokens, whiteSpaceTokens, eolChars);
        }


        /// <summary>
        /// Initialize using settings.
        /// </summary>
        /// <param name="settings"></param>
        public void Init(string text, TokenReaderSettings settings)
        {            
            Init(text, settings.EscapeChar, settings.Tokens, settings.WhiteSpaceTokens, settings.EolTokens);
        }


        /// <summary>
        /// Initialize default instance.
        /// </summary>
        /// <param name="text"></param>
        public void Init(string text, string escapeChar, string[] tokens, string[] whiteSpaceTokens, string[] eolTokens)
        {
            Reset();
            _text = text;
            LAST_POSITION = _text.Length - 1;
            _tokens = tokens.ToDictionary();
            _whiteSpaceChars = whiteSpaceTokens.ToDictionary();
            _eolChars = eolTokens.ToDictionary();

            _readonlyEolMap = new Dictionary<string, string>(_eolChars);
            _readonlyWhiteSpaceMap = new Dictionary<string, string>(_whiteSpaceChars);
        }

        

        #region ITokenReader Members
        /// <summary>
        /// The current position.
        /// </summary>
        public int Position
        {
            get { return _pos; }
        }


        /// <summary>
        /// Store the end of line chars.
        /// </summary>
        /// <param name="eolchars"></param>
        public void RegisterEol(IDictionary<string, string> eolchars)
        {
            _eolChars = eolchars;
        }

        
        /// <summary>
        /// Store the white space chars.
        /// </summary>
        /// <param name="whitespaceChars"></param>
        public void RegisterWhiteSpace(IDictionary<string, string> whitespaceChars)
        {
            _whiteSpaceChars = whitespaceChars;
        }


        /// <summary>
        /// Reset reader for parsing again.
        /// </summary>
        public void Reset()
        {
            _pos = -1;
            _text = string.Empty ;
            _backBuffer = new StringBuilder();
            _whiteSpaceChars = new Dictionary<string, string>();
            _tokens = new Dictionary<string, string>();
            _eolChars = new Dictionary<string, string>();
            _escapeChar = "\\";            
        }


        /// <summary>
        /// Returns the char at current position + 1.
        /// </summary>
        /// <param name="count"></param>
        /// <returns>Next char or string.empty if end of text.</returns>
        public string PeekChar()
        {
            // Validate.
            // a b c d e
            // 0 1 2 3 4
            // Lenght = 5
            // 4 >= 5 - 1
            if(_pos >= _text.Length - 1 )
                return string.Empty;

            string next = _text[_pos + 1].ToString();
            return next;
        }


        /// <summary>
        /// Returns the chars starting at current position + 1 and
        /// including the <paramref name="count"/> number of characters.
        /// </summary>
        /// <param name="count"></param>
        /// <returns>Range of chars as string or string.empty if end of text.</returns>
        public string PeekChars(int count)
        {
            // Validate.
            if (_pos + count > _text.Length)
                return string.Empty;

            return _text.Substring(_pos + 1, count);
        }


        /// <summary>
        /// Peeks at the string all the way until the end of line.
        /// </summary>
        /// <returns></returns>
        public string PeekLine()
        {
            int ndxEol = _text.IndexOf(Environment.NewLine, _pos + 1);
            if (ndxEol == -1)
                return _text.Substring(_pos + 1);

            return _text.Substring(_pos + 1, (ndxEol - _pos));
        }


        /// <summary>
        /// Advance and consume the current current char without storing 
        /// the char in the additional buffer for undo.
        /// </summary>
        public void ConsumeChar()
        {
            _pos++;
        }


        /// <summary>
        /// Consume the next <paramref name="count"/> chars without
        /// storing them in the additional buffer for undo.
        /// </summary>
        /// <param name="count"></param>
        public void ConsumeChars(int count)
        {
            _pos += count;
        }


        /// <summary>
        /// 
        /// </summary>
        /// <param name="readFirst"></param>
        public void ConsumeWhiteSpace()
        {
            ConsumeWhiteSpace(false);
        }


        /// <summary>
        /// Consume all white space.
        /// This works by checking the next char against
        /// the chars in the dictionary of chars supplied during initialization.
        /// </summary>
        public void ConsumeWhiteSpace(bool readFirst)
        {
            string currentChar = _currentChar;
            
            if (readFirst)
                currentChar = ReadChar();

            while ( !IsEnd() && _whiteSpaceChars.ContainsKey(currentChar))
            {
                // Advance reader and next char.
                ReadChar();
                currentChar = _currentChar;
            }            
        }


        /// <summary>
        /// Consume white space and return number of times the tab char was consumed,
        /// and the number of 
        /// </summary>
        /// <param name="tabCount"></param>
        /// <param name="whiteSpace"></param>
        public void ConsumeWhiteSpace(ref int tabCount, ref int whiteSpace)
        {
            throw new Exception("The method or operation is not implemented.");
        }


        /// <summary>
        /// Consume new line.
        /// </summary>
        public void ConsumeNewLine()
        {
            // Check 
            string combinedNewLine = _currentChar + PeekChar();
            if (_eolChars.ContainsKey(combinedNewLine))
            {
                // Move to \n in \r\n
                ReadChar();
                // Move past \n to consume
                ReadChar();
                return;
            }

            // Just \n
            if (_eolChars.ContainsKey(_currentChar))
                ReadChar();
        }


        /// <summary>
        /// Consume New Lines.
        /// </summary>
        public void ConsumeNewLines()
        {
            string combinedNewLine = _currentChar + PeekChar();
            while (_eolChars.ContainsKey(_currentChar) || _eolChars.ContainsKey(combinedNewLine))
            {
                ConsumeNewLine();
                combinedNewLine = _currentChar + PeekChar();            
            }
        }


        /// <summary>
        /// Read back the last char and reset
        /// </summary>
        public void ReadBackChar()
        {
            _pos--;
            _backBuffer.Remove(_backBuffer.Length - 1, 1);
            _currentChar = _text[_pos].ToString();
        }


        /// <summary>
        /// Unwinds the reader by <paramref name="count"/> chars.
        /// </summary>
        /// <param name="count"></param>
        public void ReadBackChar(int count)
        {
            // Unwind            
            _pos -= count;
            _backBuffer.Remove(_backBuffer.Length - count, count);
            _currentChar = _text[_pos].ToString();
        }


        /// <summary>
        /// Read the next char.
        /// </summary>
        /// <returns></returns>
        public string ReadChar()
        {
            _pos++;

            // If the last char, can't read any more.
            if (_pos >= _text.Length)
            {
                _currentChar = END_CHAR;
                return _currentChar;
            }
            
            _currentChar = _text[_pos].ToString();
            _backBuffer.Append(_currentChar);
            //TO_DO: Convert to char.
            return _currentChar;
        }


        /// <summary>
        /// Read the next <paramref name="count"/> number of chars.
        /// </summary>
        /// <param name="count"></param>
        /// <returns></returns>
        public string ReadChars(int count)
        {
            string text = _text.Substring(_pos + 1, count);
            _pos += count;
            _currentChar = _text[_pos].ToString();
            _backBuffer.Append(text);
            return text;
        }


        /// <summary>
        /// Read text up to the eol.
        /// </summary>
        /// <returns></returns>
        public string ReadToEol()
        {
            StringBuilder buffer = new StringBuilder();

            // Read until ":" colon and while not end of string.
            while (!IsEol(_eolChars as IDictionary) && _pos <= LAST_POSITION)
            {
                buffer.Append(_currentChar);
                ReadChar();
            }
            return buffer.ToString();
        }


        /// <summary>
        /// Read text up to the eol.
        /// </summary>
        /// <returns></returns>
        public string ReadToken(string endChar, string escapeChar, bool includeEndChar, bool advanceFirst, bool expectEndChar, bool readPastEndChar)
        {
            StringBuilder buffer = new StringBuilder();
            string currentChar = advanceFirst ? ReadChar() : _currentChar;

            // Read until ":" colon and while not end of string.
            while (currentChar != endChar && _pos <= LAST_POSITION)
            {
                // Escape char
                if (currentChar == escapeChar)
                {
                    currentChar = ReadChar();
                    buffer.Append(currentChar);
                }
                else
                    buffer.Append(currentChar);

                currentChar = ReadChar();
            }
            bool matchedEndChar = true;

            // Error out if current char is not ":".
            if (expectEndChar && _currentChar != endChar)
            {
                _errors.Add("Expected " + endChar + " at : " + _pos);
                matchedEndChar = false;
            }

            // Read past char.
            if (matchedEndChar && readPastEndChar)
                ReadChar();

            return buffer.ToString();
        }


        /// <summary>
        /// Consume all white space.
        /// This works by checking the next char against
        /// the chars in the dictionary of chars supplied during initialization.
        /// </summary>
        public void ConsumeWhiteSpace(bool readFirst, bool consumeLastWhiteSpace)
        {
            string currentChar = readFirst ? ReadChar() : _currentChar;
            string nextChar = PeekChar();


            while (!IsEnd() && _whiteSpaceChars.ContainsKey(nextChar))
            {
                // Advance reader and next char.
                ReadChar();
                nextChar = PeekChar();
            } 
        }   


        /// <summary>
        /// Read text up to the eol.
        /// </summary>
        /// <returns></returns>
        public string ReadTokenUntil(string endChar, string escapeChar, bool advanceFirst, bool expectEndChar, bool includeEndChar, bool moveToEndChar, bool readPastEndChar)
        {
            // abcd <div>
            var buffer = new StringBuilder();
            var currentChar = advanceFirst ? ReadChar() : _currentChar;
            var nextChar = PeekChar();
            while (nextChar != endChar && _pos <= LAST_POSITION)
            {
                // Escape char
                if (currentChar == escapeChar)
                {
                    currentChar = ReadChar();
                    buffer.Append(currentChar);
                }
                else
                    buffer.Append(currentChar);

                currentChar = ReadChar();
                nextChar = PeekChar();
            }
            bool matchedEndChar = nextChar == endChar;
            if( expectEndChar && !matchedEndChar)
                _errors.Add("Expected " + endChar + " at : " + _pos);

            if (matchedEndChar)
            {
                buffer.Append(currentChar);
                if (includeEndChar)
                    buffer.Append(nextChar);

                if (moveToEndChar)
                    ReadChar();

                else if (readPastEndChar && !IsAtEnd())
                    ReadChars(2);
            }
            
            return buffer.ToString();
        }


        public bool Expect(string charToExpect)
        {
            bool isMatch = _currentChar == charToExpect;
            if(!isMatch)
                _errors.Add("Expected " + charToExpect + " at : " + _pos);
            return isMatch;
        }


        /// <summary>
        /// Current char.
        /// </summary>
        /// <returns></returns>
        public string CurrentChar
        {
            get { return _currentChar; }
        }


        /// <summary>
        /// Get the previous char that was read in.
        /// </summary>
        public string PreviousChar
        {
            get
            {
                // Check.
                if (_pos <= 0 || _backBuffer.Length <= 0)
                    return string.Empty;

                // Get the last char from the back buffer.
                // This is the last valid char that is not escaped.
                return _backBuffer[_backBuffer.Length - 2].ToString();
            }
        }


        /// <summary>
        /// Get the end of line chars.
        /// </summary>
        public IDictionary<string, string> EolChars
        {
            get { return _readonlyEolMap; }
        }


        /// <summary>
        /// Get the previous char that is part of the input and which may be an escape char.
        /// </summary>
        public string PreviousCharAny
        {
            get
            {
                // Check.
                if (_pos <= 0 )
                    return string.Empty;

                // Get the last char from the back buffer.
                // This is the last valid char that is not escaped.
                return _text[_pos - 1].ToString();
            }
        }

        
        /// <summary>
        /// Current position in text.
        /// </summary>
        /// <returns></returns>
        public int CurrentCharIndex()
        {
            return _pos;
        }


        /// <summary>
        /// An integer representation of the current char.
        /// </summary>
        /// <returns></returns>
        public int CurrentCharInt()
        {
            throw new Exception("The method or operation is not implemented.");
        }


        /// <summary>
        /// Determine if current char is token.
        /// </summary>
        /// <returns></returns>
        public bool IsToken()
        {
            return _tokens.ContainsKey(_currentChar);
        }


        /// <summary>
        /// Determine if current char is escape char.
        /// </summary>
        /// <returns></returns>
        public bool IsEscape()
        {
            return string.Compare(_currentChar, _escapeChar, false) == 0;
        }


        /// <summary>
        /// Determine if the end of the text input has been reached.
        /// </summary>
        /// <returns></returns>
        public bool IsEnd()
        {
            return _pos >= _text.Length;
        }


        /// <summary>
        /// Determine if at last char.
        /// </summary>
        /// <returns></returns>
        public bool IsAtEnd()
        {
            return _pos == _text.Length - 1;
        }


        /// <summary>
        /// Is End of line
        /// </summary>
        /// <returns></returns>
        public bool IsEol()
        {
            return IsEol(_eolChars as IDictionary);
        }


        /// <summary>
        /// Determine if current char is EOL.
        /// </summary>
        /// <param name="eolChars"></param>
        /// <returns></returns>
        public bool IsEol(IDictionary eolChars)
        {
            // Check for "\r\n"
            string combined = _currentChar + PeekChar();

            if (eolChars.Contains(combined) || eolChars.Contains(_currentChar) )
                return true;

            return false;
        }


        /// <summary>
        /// Determine if current char is whitespace.
        /// </summary>
        /// <param name="whitespaceChars"></param>
        /// <returns></returns>
        public bool IsWhiteSpace(IDictionary whitespaceChars)
        {
            return whitespaceChars.Contains(_currentChar);
        }


        /// <summary>
        /// Determine if current char is whitespace.
        /// </summary>
        /// <returns></returns>
        public bool IsWhiteSpace()
        {
            return this._whiteSpaceChars.ContainsKey(_currentChar);
        }
        #endregion


        #region Private methods
        private void Init(IDictionary<string, bool> tokens, string[] tokenList)
        {
            foreach (string token in tokenList)
            {
                tokens[token] = true;
            }
        }
        #endregion
    }
}
