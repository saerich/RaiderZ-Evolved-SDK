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
    /// Base class for lexical parsing.
    /// </summary>
    public class LexBase
    {
        #region Protected members
        protected ITokenReader _reader;
        protected LexSettings _settings = new LexSettings();
        protected List<string> _tokenList;
        protected IList<string> _errors;
        protected IDictionary<string, string> _whiteSpaceMap;
        #endregion

        public bool AllowNewLine { get; set; }


        /// <summary>
        /// Initalize using default settings.
        /// </summary>
        /// <param name="settings"></param>
        public virtual void Init(LexSettings settings)
        {
            _reader = new TokenReader();
            _errors = new List<string>();
            _tokenList = new List<string>();
            _settings = settings;
        }


        /// <summary>
        /// Parse text
        /// </summary>
        /// <param name="line"></param>
        /// <returns></returns>
        public virtual List<string> ParseText(string line)
        {
            Reset(line);

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
                if (_reader.IsToken())
                {
                    string quote = _reader.CurrentChar;
                    _tokenList.Add(ReadQuotedToken());

                    // Expect ending quote and move to next char advance parsing.                    
                    if (Expect(quote))
                        _reader.ReadChar();
                }
                else // normal char.
                {
                    _tokenList.Add(ReadNonQuotedToken(_whiteSpaceMap, true));
                }

                // Now read next char
                _reader.ReadChar();
            }

            // Handle errors.
            CheckAndThrowErrors();

            // Remove new lines at end.
            ExcludeNewLinesStored();

            return _tokenList;
        }


        /// <summary>
        /// Read a continuous set of characters until 
        /// end of text or separater is reached.
        /// </summary>
        /// <returns></returns>
        protected string ReadNonQuotedToken(IDictionary<string, string> separators, bool breakOnEol)
        {
            // Store the type of quote. single/double ?
            // and move to next char.
            StringBuilder buffer = new StringBuilder();

            // Keep reading until space.
            while (!separators.ContainsKey(_reader.CurrentChar) && !_reader.IsEnd())
            {
                if (breakOnEol && _reader.IsEol())
                {
                    break;
                }
                buffer.Append(_reader.CurrentChar);
                _reader.ReadChar();
            }

            string token = buffer.ToString();
            return token;
        }


        /// <summary>
        /// Read a quoted set of characters.
        /// e.g. 'firstname' or "lastname"
        /// </summary>
        /// <returns></returns>
        protected virtual string ReadQuotedToken()
        {
            // Store the type of quote. single/double ?
            // and move to next char.
            StringBuilder buffer = new StringBuilder();
            string quote = _reader.CurrentChar;
            _reader.ReadChar();

            // Keep reading until ending quote.
            while (_reader.CurrentChar != quote && !_reader.IsEnd())
            {
                // Avoid escape char. \'
                if (!_reader.IsEscape())
                {
                    buffer.Append(_reader.CurrentChar);
                }
                else
                {
                    string nextChar = _reader.ReadChar();
                    buffer.Append(nextChar);
                }
                _reader.ReadChar();
            }

            string token = buffer.ToString();
            return token;
        }


        /// <summary>
        /// Expect the current char to be the char specified.
        /// </summary>
        /// <param name="expectChar"></param>
        /// <returns></returns>
        protected virtual bool Expect(string expectChar)
        {
            // Expect that current char is the ending quote.
            bool isMatch = _reader.CurrentChar == expectChar;
            if (!isMatch)
                AddError("Expected " + expectChar + ", but found end of line/text.");

            return isMatch;
        }


        /// <summary>
        /// Add error to list of errors.
        /// </summary>
        /// <param name="error"></param>
        protected virtual void AddError(string error)
        {
            _errors.Add(error);
        }


        /// <summary>
        /// Check for errors and throw.
        /// </summary>
        protected void CheckAndThrowErrors()
        {
            if (_errors.Count > 0)
                throw new ArgumentException("Errors parsing line : " + StringHelper.Join(_errors, Environment.NewLine));
        }


        /// <summary>
        /// Exclude new lines stored at the end as a result of the parser
        /// reading the \r\n at the end of line.
        /// </summary>
        protected void ExcludeNewLinesStored()
        {
            // Exclude the newline at the end.
            if (_reader.EolChars.ContainsKey(_tokenList[_tokenList.Count - 1]))
                _tokenList.RemoveAt(_tokenList.Count - 1);
        }


        /// <summary>
        /// Reset the state.
        /// </summary>
        /// <param name="line"></param>
        protected virtual void Reset(string line)
        {
            _reader.Reset();
            _errors.Clear();
            _tokenList.Clear();
            _whiteSpaceMap = _settings.WhiteSpaceChars.ToDictionary();
            _reader.Init(line, "\\", _settings.QuotesChars, _settings.WhiteSpaceChars, _settings.EolChars);
        }
    }
}