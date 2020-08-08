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
using Val = System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Collections;
using ComLib;


namespace ComLib
{
    public static partial class Validation
    {
        #region IValidatorBasic Members
        /// <summary>
        /// Validates the object using System.ComponentModel.DataAnnotations, and puts any errors into the errors object.
        /// </summary>
        /// <param name="objectToValidate">Object to validate</param>
        /// <param name="errors">The collection to put validation errors into.</param>
        /// <returns></returns>
        public static bool ValidateObject(object objectToValidate, IErrors errors = null)
        {

            var validationResults = new List<Val.ValidationResult>();
            var ctx = new Val.ValidationContext(objectToValidate, null, null);
            var isValid = Val.Validator.TryValidateObject(objectToValidate, ctx, validationResults, true);
            
            if(!isValid && errors != null)
                foreach (var result in validationResults)
                    errors.Add(result.ErrorMessage);
            
            return isValid;
        }


        /// <summary>
        /// Determine if string is valid with regard to minimum / maximum length.
        /// </summary>
        /// <param name="text">Text to check length of.</param>
        /// <param name="allowNull">Indicate whether or not to allow null.</param>
        /// <param name="minLength">-1 to not check min length, > 0 to represent min length.</param>
        /// <param name="maxLength">-1 to not check max length, > 0 to represent max length.</param>
        /// <returns>True if match based on parameter conditions, false otherwise.</returns>
        public static bool IsStringLengthMatch(string text, bool allowNull, bool checkMinLength, bool checkMaxLength, int minLength, int maxLength, IErrors errors, string tag)
        {
            if (string.IsNullOrEmpty(text) && allowNull) return true;

            int textLength = text == null ? 0 : text.Length;  
              
            // Check bounds . -1 value for min/max indicates not to check.
            if (checkMinLength && minLength > 0 && textLength < minLength)
                return CheckError(false, errors, tag, "Text supplied is less than min length(" + minLength + ")");

            if (checkMaxLength && maxLength > 0 && textLength > maxLength)
                return CheckError(false, errors, tag, "Text supplied is more than max length(" + maxLength + ")");            

            return true;
        }


        /// <summary>
        /// Determines if string matches the regex.
        /// </summary>
        /// <param name="text">Text to match.</param>
        /// <param name="allowNull">Whether or not text can be null or empty for successful match.</param>
        /// <param name="regEx">Regular expression to use.</param>
        /// <returns>True if match, false otherwise.</returns>
        public static bool IsStringRegExMatch(string text, bool allowNull, string regEx, IErrors errors, string tag)
        {
            return CheckErrorRegEx(text, allowNull, regEx, errors, tag, "Text supplied does not match expected pattern.");
        }


        /// <summary>
        /// Determines whether the text [is string in] [the specified values].
        /// </summary>
        /// <param name="text">The text.</param>
        /// <param name="allowNull">if set to <c>true</c> [allow null].</param>
        /// <param name="compareCase">if set to <c>true</c> [compare case].</param>
        /// <param name="allowedValues">The allowed values.</param>
        /// <param name="errors">The errors.</param>
        /// <param name="tag">The tag.</param>
        /// <returns>
        /// 	<c>true</c> if [is string in] [the specified text]; otherwise, <c>false</c>.
        /// </returns>
        public static bool IsStringIn(string text, bool allowNull, bool compareCase, string[] allowedValues, IErrors errors, string tag)
        {
            bool isEmpty = string.IsNullOrEmpty(text);
            if (isEmpty && allowNull) return true;
            if (isEmpty && !allowNull)
            {
                string vals = allowedValues.JoinDelimited(",", (val) => val);
                errors.Add(tag, "Text must be in : " + vals);
                return false;
            }
            bool isValid = false;
            foreach (string val in allowedValues)
            {
                if (string.Compare(text, val, compareCase) == 0)
                {
                    isValid = true;
                    break;
                }
            }
            if (!isValid)
            {
                string vals = allowedValues.JoinDelimited(",", (val) => val);
                errors.Add(tag, "Text must be in : " + vals);
                return false;
            }
            return true;
        }

        
        /// <summary>
        /// Determines if text supplied is numeric
        /// </summary>
        /// <param name="text"></param>
        /// <param name="checkMinBound"></param>
        /// <param name="checkMaxBound"></param>
        /// <param name="min"></param>
        /// <param name="max"></param>
        /// <returns></returns>
        public static bool IsNumeric(string text, bool allowNull, IErrors errors, string tag)
        {
            return CheckErrorRegEx(text, allowNull, RegexPatterns.Numeric, errors, tag, "Text supplied is not a valid number.");
        }

        
        /// <summary>
        /// Determines if text supplied is numeric and within the min/max bounds.
        /// </summary>
        /// <param name="text">Text to check if it's numeric and within bounds.</param>
        /// <param name="checkMinBound">Whether or not to check</param>
        /// <param name="checkMaxBound"></param>
        /// <param name="min"></param>
        /// <param name="max"></param>
        /// <returns></returns>
        public static bool IsNumericWithinRange(string text, bool checkMinBound, bool checkMaxBound, double min, double max, IErrors errors, string tag)
        {
            bool isNumeric = Regex.IsMatch(text, RegexPatterns.Numeric);
            if (!isNumeric)
            {
                errors.Add(tag, "Text supplied is not numeric.");
                return false;
            }

            double num = Double.Parse(text);
            return IsNumericWithinRange(num, checkMinBound, checkMaxBound, min, max, errors, tag);
        }


        /// <summary>
        /// Determines if text supplied is numeric and within the min/max bounds.
        /// </summary>
        /// <param name="text">Text to check if it's numeric and within bounds.</param>
        /// <param name="checkMinBound">Whether or not to check</param>
        /// <param name="checkMaxBound"></param>
        /// <param name="min"></param>
        /// <param name="max"></param>
        /// <returns></returns>
        public static bool IsNumericWithinRange(double num, bool checkMinBound, bool checkMaxBound, double min, double max, IErrors errors, string tag)
        {
            if (checkMinBound && num < min)
            {
                errors.Add(tag, "Number supplied is less than " + min + ".");
                return false;
            }

            if (checkMaxBound && num > max)
            {
                errors.Add(tag, "Number supplied is more than " + max + ".");
                return false;
            }

            return true;
        }


        /// <summary>
        /// Determines if text is either lowercase/uppercase alphabets.
        /// </summary>
        /// <param name="text">The text check</param>
        /// <param name="allowNull">Whether or not the text can be null</param>
        /// <param name="errors">List of errors.</param>
        /// <param name="tag">Tag used to identify the error.</param>
        /// <returns></returns>
        public static bool IsAlpha(string text, bool allowNull, IErrors errors, string tag)
        {
            return CheckErrorRegEx(text, allowNull, RegexPatterns.Alpha, errors, tag, "Text supplied must only contain chars " + RegexPatterns.Alpha);
        }


        /// <summary>
        /// Determines if text is either lowercase/uppercase alphabets or numbers.
        /// </summary>
        /// <param name="text">The text check</param>
        /// <param name="allowNull">Whether or not the text can be null</param>
        /// <param name="errors">List of errors.</param>
        /// <param name="tag">Tag used to identify the error.</param>
        /// <returns></returns>
        public static bool IsAlphaNumeric(string text, bool allowNull, IErrors errors, string tag)
        {
            return CheckErrorRegEx(text, allowNull, RegexPatterns.AlphaNumeric, errors, tag, "Text supplied must only contain chars and numbers " + RegexPatterns.AlphaNumeric);
        }


        /// <summary>
        /// Determines if the date supplied is a date.
        /// </summary>
        /// <param name="text"></param>
        /// <param name="checkBounds"></param>
        /// <param name="minDate"></param>
        /// <param name="maxDate"></param>
        /// <returns></returns>
        public static bool IsDate(string text, IErrors errors, string tag)
        {
            DateTime result = DateTime.MinValue;
            return CheckError(DateTime.TryParse(text, out result), errors, tag, "Text supplied is not a valid date");
        }


        /// <summary>
        /// Determines if the date supplied is a date.
        /// </summary>
        /// <param name="text"></param>
        /// <param name="checkBounds"></param>
        /// <param name="minDate"></param>
        /// <param name="maxDate"></param>
        /// <returns></returns>
        public static bool IsDateWithinRange(string text, bool checkMinBound, bool checkMaxBound, DateTime minDate, DateTime maxDate, IErrors errors, string tag)
        {
            DateTime result = DateTime.MinValue;
            if (!DateTime.TryParse(text, out result))
            {
                errors.Add(tag, "Text supplied is not a valid date");
                return false;
            }

            return IsDateWithinRange(result, checkMinBound, checkMaxBound, minDate, maxDate, errors, tag);
        }


        /// <summary>
        /// Determines if the date supplied is a date within the specified bounds.
        /// </summary>
        /// <param name="date"></param>
        /// <param name="checkMinBound"></param>
        /// <param name="checkMaxBound"></param>
        /// <param name="minDate"></param>
        /// <param name="maxDate"></param>
        /// <returns></returns>
        public static bool IsDateWithinRange(DateTime date, bool checkMinBound, bool checkMaxBound, DateTime minDate, DateTime maxDate, IErrors errors, string tag)
        {
            if (checkMinBound && date.Date < minDate.Date)
            {
                errors.Add(tag, "Date supplied is less than minimum date " + minDate.ToShortDateString());
                return false;
            }
            if (checkMaxBound && date.Date > maxDate.Date)
            {
                errors.Add(tag, "Date supplied is more than maximum date " + maxDate.ToShortDateString());
                return false;
            }

            return true;
        }


        /// <summary>
        /// Determines if the time string specified is a time of day. e.g. 9am
        /// and within the bounds specified.
        /// </summary>
        /// <param name="time"></param>
        /// <param name="checkBounds"></param>
        /// <param name="min"></param>
        /// <param name="max"></param>
        /// <returns></returns>
        public static bool IsTimeOfDay(string time, IErrors errors, string tag)
        {
            TimeSpan span = TimeSpan.MinValue;
            bool isMatch = TimeSpan.TryParse(time, out span);
            return CheckError(isMatch, errors, tag, "Text supplied is not a valid time.");
        }


        /// <summary>
        /// Determines if the time string specified is a time of day. e.g. 9am
        /// and within the bounds specified.
        /// </summary>
        /// <param name="time"></param>
        /// <param name="checkBounds"></param>
        /// <param name="min"></param>
        /// <param name="max"></param>
        /// <returns></returns>
        public static bool IsTimeOfDayWithinRange(string time, bool checkMinBound, bool checkMaxBound, TimeSpan min, TimeSpan max, IErrors errors, string tag)
        {
            TimeSpan span = TimeSpan.MinValue;
            if (!TimeSpan.TryParse(time, out span))
                return CheckError(false, errors, tag, "Text supplied is not a valid time.");

            return IsTimeOfDayWithinRange(span, checkMinBound, checkMaxBound, min, max, errors, tag);
        }

        
        /// <summary>
        /// Determines if the time string specified is a time of day. e.g. 9am
        /// and within the bounds specified.
        /// </summary>
        /// <param name="time"></param>
        /// <param name="checkBounds"></param>
        /// <param name="min"></param>
        /// <param name="max"></param>
        /// <returns></returns>
        public static bool IsTimeOfDayWithinRange(TimeSpan time, bool checkMinBound, bool checkMaxBound, TimeSpan min, TimeSpan max, IErrors errors, string tag)
        {
            return true;
        }


        /// <summary>
        /// Determines if the phone number supplied is a valid US phone number.
        /// </summary>
        /// <param name="phone"></param>
        /// <returns></returns>
        public static bool IsPhoneUS(string text, bool allowNull, IErrors errors, string tag)
        {
            return CheckErrorRegEx(text, allowNull, RegexPatterns.PhoneUS, errors, tag, "Text supplied is not a valid US phone number.");
        }


        /// <summary>
        /// Determines if the phone number supplied is a valid US phone number.
        /// </summary>
        /// <param name="phone"></param>
        /// <returns></returns>
        public static bool IsPhoneUS(int phone, IErrors errors, string tag)
        {
            return CheckErrorRegEx(phone.ToString(), false, RegexPatterns.PhoneUS, errors, tag, "Text supplied is not a valid US phone number.");
        }


        /// <summary>
        /// Determines if ssn supplied is a valid ssn.
        /// </summary>
        /// <param name="ssn"></param>
        /// <returns></returns>
        public static bool IsSsn(string text, bool allowNull, IErrors errors, string tag)
        {
            return CheckErrorRegEx(text, allowNull, RegexPatterns.SocialSecurity, errors, tag, "Text supplied is not a valid social security number.");
        }

        
        /// <summary>
        /// Determines if ssn supplied is a valid ssn.
        /// </summary>
        /// <param name="text">The text check</param>
        /// <param name="allowNull">Whether or not the text can be null</param>
        /// <param name="errors">List of errors.</param>
        /// <param name="tag">Tag used to identify the error.</param>
        /// <returns></returns>
        public static bool IsSsn(int ssn, IErrors errors, string tag)
        {
            return CheckErrorRegEx(ssn.ToString(), false, RegexPatterns.SocialSecurity, errors, tag, "Text supplied is not a valid social security number.");
        }


        /// <summary>
        /// Determines if email supplied is a valid email.
        /// </summary>
        /// <param name="text">The text check</param>
        /// <param name="allowNull">Whether or not the text can be null</param>
        /// <param name="errors">List of errors.</param>
        /// <param name="tag">Tag used to identify the error.</param>
        /// <returns></returns>
        public static bool IsEmail(string text, bool allowNull, IErrors errors, string tag)
        {
            return CheckErrorRegEx(text, allowNull, RegexPatterns.Email, errors, tag, "Text supplied is not a valid email.");
        }


        /// <summary>
        /// Determines if url supplied is a valid url.
        /// </summary>
        /// <param name="text">The text check</param>
        /// <param name="allowNull">Whether or not the text can be null</param>
        /// <param name="errors">List of errors.</param>
        /// <param name="tag">Tag used to identify the error.</param>
        /// <returns></returns>
        public static bool IsUrl(string text, bool allowNull, IErrors errors, string tag)
        {
            return CheckErrorRegEx(text, allowNull, RegexPatterns.Url, errors, tag, "Text supplied is not a valid url.");
        }


        /// <summary>
        /// Determines if email supplied is a valid email.
        /// </summary>
        /// <param name="text">The text check</param>
        /// <param name="allowNull">Whether or not the text can be null</param>
        /// <param name="errors">List of errors.</param>
        /// <param name="tag">Tag used to identify the error.</param>
        /// <returns></returns>
        public static bool IsZipCode(string text, bool allowNull, IErrors errors, string tag)
        {
            return CheckErrorRegEx(text, allowNull, RegexPatterns.ZipCodeUS, errors, tag, "Text supplied is not a valid US zipcode.");
        }


        /// <summary>
        /// Determines if email supplied is a valid zip with 4 additional chars.
        /// e.g. 12345-2321
        /// </summary>
        /// <param name="text">The text check</param>
        /// <param name="allowNull">Whether or not the text can be null</param>
        /// <param name="errors">List of errors.</param>
        /// <param name="tag">Tag used to identify the error.</param>
        /// <returns></returns>
        public static bool IsZipCodeWith4Char(string text, bool allowNull, IErrors errors, string tag)
        {
            return CheckErrorRegEx(text, allowNull, RegexPatterns.ZipCodeUSWithFour, errors, tag, "Text supplied is not a valid US zipcode.");
        }


        /// <summary>
        /// Determines if email supplied is a valid zip with 4 additional chars.
        /// e.g. 12345-2321
        /// </summary>
        /// <param name="text">The text check</param>
        /// <param name="allowNull">Whether or not the text can be null</param>
        /// <param name="errors">List of errors.</param>
        /// <param name="tag">Tag used to identify the error.</param>
        /// <returns></returns>
        public static bool IsZipCodeWith4CharOptional(string text, bool allowNull, IErrors errors, string tag)
        {
            return CheckErrorRegEx(text, allowNull, RegexPatterns.ZipCodeUSWithFourOptional, errors, tag, "Text supplied is not a valid US zipcode.");
        }
        #endregion


        /// <summary>
        /// Check the condition and add the error.
        /// </summary>
        /// <param name="isValid"></param>
        /// <param name="errors"></param>
        /// <param name="tag"></param>
        /// <param name="error"></param>
        /// <param name="target"></param>
        /// <returns></returns>
        public static bool CheckError(bool isValid, IErrors errors, string tag, string error)
        {
            if (!isValid)
            {
                errors.Add(tag, error);                
            }
            return isValid;
        }


        /// <summary>
        /// Check the text for the regex pattern and adds errors in incorrect.
        /// </summary>
        /// <param name="inputText"></param>
        /// <param name="allowNull"></param>
        /// <param name="regExPattern"></param>
        /// <param name="errors"></param>
        /// <param name="tag"></param>
        /// <param name="error"></param>
        /// <param name="target"></param>
        /// <returns></returns>
        public static bool CheckErrorRegEx(string inputText, bool allowNull, string regExPattern, IErrors errors, string tag, string error)
        {
            bool isEmpty = string.IsNullOrEmpty(inputText);
            if (allowNull && isEmpty)
                return true;

            if (!allowNull && isEmpty)
            {
                errors.Add(tag, error);
                return false;
            }

            bool isValid = Regex.IsMatch(inputText, regExPattern);
            if (!isValid) errors.Add(tag, error);

            return isValid;
        }
    }
}
