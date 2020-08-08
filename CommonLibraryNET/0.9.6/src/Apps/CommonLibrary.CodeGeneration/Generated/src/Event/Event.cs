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

using ComLib.Entities;
using ComLib.LocationSupport;
using ComLib.ValidationSupport;



namespace ComLib.WebModules.Events
{
    /// <summary>
    /// Event entity.
    /// </summary>
    public partial class Event : ActiveRecordBaseEntity<Event>, IEntity
    {
        /// <summary>
        /// Creates a new instance of BlogPost and 
        /// initializes it with a validator and settings.
        /// </summary>
        /// <returns></returns>
        public static Event New()
        {
            Event entity = new Event(); 
            return entity;
        }       


        		/// <summary>
		/// Get/Set UserId
		/// </summary>
		public int UserId { get; set; }


		/// <summary>
		/// Get/Set Title
		/// </summary>
		public string Title { get; set; }


		/// <summary>
		/// Get/Set Summary
		/// </summary>
		public string Summary { get; set; }


		/// <summary>
		/// Get/Set Description
		/// </summary>
		public string Description { get; set; }


		/// <summary>
		/// Get/Set StartDate
		/// </summary>
		public DateTime StartDate { get; set; }


		/// <summary>
		/// Get/Set EndDate
		/// </summary>
		public DateTime EndDate { get; set; }


		/// <summary>
		/// Get/Set StartTime
		/// </summary>
		public int StartTime { get; set; }


		/// <summary>
		/// Get/Set EndTime
		/// </summary>
		public int EndTime { get; set; }


		/// <summary>
		/// Get/Set Email
		/// </summary>
		public string Email { get; set; }


		/// <summary>
		/// Get/Set Phone
		/// </summary>
		public string Phone { get; set; }


		/// <summary>
		/// Get/Set Url
		/// </summary>
		public string Url { get; set; }


		/// <summary>
		/// Get/Set Keywords
		/// </summary>
		public string Keywords { get; set; }


		/// <summary>
		/// Get/Set AverageRating
		/// </summary>
		public int AverageRating { get; set; }


		/// <summary>
		/// Get/Set TotalLiked
		/// </summary>
		public int TotalLiked { get; set; }


		/// <summary>
		/// Get/Set TotalDisLiked
		/// </summary>
		public int TotalDisLiked { get; set; }


		/// <summary>
		/// Get/Set TotalBookMarked
		/// </summary>
		public int TotalBookMarked { get; set; }


		/// <summary>
		/// Get/Set TotalAbuseReports
		/// </summary>
		public int TotalAbuseReports { get; set; }


		private Address _address;
		/// <summary>
		/// Get/Set Address
		/// </summary>
		public Address Address
		 { 
		 get { return _address;  }
		 set { _address = value; }
		 }





        protected virtual IEntityValidator GetValidatorInternal()
        {
            var val = new EntityValidator((validationEvent) =>
            {
                int initialErrorCount = validationEvent.Results.Count;
                IValidationResults results = validationEvent.Results;            
                Event entity = (Event)validationEvent.Target;
				Validation.IsNumericWithinRange(entity.UserId, false, false, -1, -1, results, "UserId");
				Validation.IsStringLengthMatch(entity.Title, false, true, true, 2, 150, results, "Title" );
				Validation.IsStringLengthMatch(entity.Summary, true, false, true, -1, 200, results, "Summary" );
				Validation.IsStringLengthMatch(entity.Description, false, false, false, -1, -1, results, "Description" );
				Validation.IsDateWithinRange(entity.StartDate, false, false, DateTime.MinValue, DateTime.MaxValue, results, "StartDate" );
				Validation.IsDateWithinRange(entity.EndDate, false, false, DateTime.MinValue, DateTime.MaxValue, results, "EndDate" );
				Validation.IsNumericWithinRange(entity.StartTime, false, false, -1, -1, results, "StartTime");
				Validation.IsNumericWithinRange(entity.EndTime, false, false, -1, -1, results, "EndTime");
				Validation.IsStringRegExMatch(entity.Email, false, @"RegexPatterns.Email", results, "Email" );
				Validation.IsStringRegExMatch(entity.Phone, false, @"RegexPatterns.PhoneUS", results, "Phone" );
				Validation.IsStringRegExMatch(entity.Url, false, @"RegexPatterns.Url", results, "Url" );
				Validation.IsStringRegExMatch(entity.Keywords, false, @"RegexPatterns.Url", results, "Keywords" );
				Validation.IsNumericWithinRange(entity.AverageRating, false, false, -1, -1, results, "AverageRating");
				Validation.IsNumericWithinRange(entity.TotalLiked, false, false, -1, -1, results, "TotalLiked");
				Validation.IsNumericWithinRange(entity.TotalDisLiked, false, false, -1, -1, results, "TotalDisLiked");
				Validation.IsNumericWithinRange(entity.TotalBookMarked, false, false, -1, -1, results, "TotalBookMarked");
				Validation.IsNumericWithinRange(entity.TotalAbuseReports, false, false, -1, -1, results, "TotalAbuseReports");

                return initialErrorCount == validationEvent.Results.Count;
            });
            return val;
        }
    }
}
