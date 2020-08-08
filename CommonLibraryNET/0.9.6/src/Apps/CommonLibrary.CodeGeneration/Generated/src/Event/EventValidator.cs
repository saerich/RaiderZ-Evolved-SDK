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
using System.Text;

using ComLib;
using ComLib.Entities;
using ComLib.ValidationSupport;


namespace CommonLibrary.WebModules.Events
{
    /// <summary>
    /// Validator for Event
    /// </summary>
    public partial class EventValidator : EntityValidator
    {
        /// <summary>
        /// Validation method for the entity.
        /// </summary>
        /// <param name="target">The target.</param>
        /// <param name="useTarget">if set to <c>true</c> [use target].</param>
        /// <param name="results">The results.</param>
        /// <returns></returns>
        protected override bool ValidateInternal(ValidationEvent validationEvent)
        {
            var val = new Validator( (valEvent) =>
            {
                int initialErrorCount = validationEvent.Results.Count;
                IValidationResults results = validationEvent.Results;
                Event entity = (Event)validationEvent.Target;
                Validation.IsStringLengthMatch(entity.Title, false, true, true, 10, 150, results, "Title");
                Validation.IsStringLengthMatch(entity.Summary, false, false, true, -1, 200, results, "Summary");
                Validation.IsStringLengthMatch(entity.Description, false, false, false, -1, -1, results, "Description");
                Validation.IsDateWithinRange(entity.StartDate, false, false, DateTime.MinValue, DateTime.MaxValue, results, "StartDate");
                Validation.IsDateWithinRange(entity.EndDate, false, false, DateTime.MinValue, DateTime.MaxValue, results, "EndDate");
                Validation.IsNumericWithinRange(entity.StartTime, false, false, -1, -1, results, "StartTime");
                Validation.IsNumericWithinRange(entity.EndTime, false, false, -1, -1, results, "EndTime");
                Validation.IsStringRegExMatch(entity.Email, false, RegexPatterns.Email, results, "Email");
                Validation.IsStringRegExMatch(entity.Phone, false, RegexPatterns.PhoneUS, results, "Phone");
                Validation.IsStringRegExMatch(entity.Url, false, RegexPatterns.Url, results, "Url");
                Validation.IsStringLengthMatch(entity.Keywords, true, false, true, -1, 100, results, "Keywords");

                return initialErrorCount == validationEvent.Results.Count;
            });
            return val.IsValid;
        }
    }
}
