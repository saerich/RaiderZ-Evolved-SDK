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
using System.Text;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Data.SqlClient;

using ComLib.Entities;
using ComLib.Data;
using ComLib.LocationSupport;



namespace ComLib.WebModules.Events
{
    /// <summary>
    /// Generic repository for persisting Event.
    /// </summary>
    public partial class EventRepository : RepositorySql<Event>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="NamedQueryRepository"/> class.
        /// </summary>
        public EventRepository() { }

        
        /// <summary>
        /// Initializes a new instance of the <see cref="Repository&lt;TId, T&gt;"/> class.
        /// </summary>
        /// <param name="connectionInfo">The connection string.</param>
        public  EventRepository(string connectionString) : base(connectionString)
        {
        }


        /// <summary>
        /// Initializes a new instance of the <see cref="Repository&lt;TId, T&gt;"/> class.
        /// </summary>
        /// <param name="connectionInfo">The connection info.</param>
        /// <param name="helper">The helper.</param>
        public EventRepository(ConnectionInfo connectionInfo) : base(connectionInfo)
        {            
        }


        /// <summary>
        /// Initializes a new instance of the <see cref="Repository&lt;TId, T&gt;"/> class.
        /// </summary>
        /// <param name="connectionInfo">The connection info.</param>
        /// <param name="helper">The helper.</param>
        public EventRepository(ConnectionInfo connectionInfo, IDatabase db)
            : base(connectionInfo, db)
        {
        }


        /// <summary>
        /// Initialize the rowmapper
        /// </summary>
        public override void Init(ConnectionInfo connectionInfo, IDatabase db)
        {
            base.Init(connectionInfo, db);
            this.RowMapper = new EventRowMapper();
        }


        /// <summary>
        /// Create the entity using sql.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        public override Event Create(Event entity)
        {
            string sql = "insert into Events ( [CreateDate], [UpdateDate], [CreateUser], [UpdateUser], [UpdateComment], [IsActive]"
			 + ", [UserId], [Title], [Summary], [Description], [StartDate], [EndDate]"
			 + ", [StartTime], [EndTime], [Email], [Phone], [Url], [Keywords]"
			 + ", [AverageRating], [TotalLiked], [TotalDisLiked], [TotalBookMarked], [TotalAbuseReports], [Street]"
			 + ", [City], [State], [Country], [Zip], [CityId], [StateId]"
			 + ", [CountryId], [IsOnline] ) values ( @CreateDate, @UpdateDate, @CreateUser, @UpdateUser, @UpdateComment, @IsActive"
			 + ", @UserId, @Title, @Summary, @Description, @StartDate, @EndDate"
			 + ", @StartTime, @EndTime, @Email, @Phone, @Url, @Keywords"
			 + ", @AverageRating, @TotalLiked, @TotalDisLiked, @TotalBookMarked, @TotalAbuseReports, @Street"
			 + ", @City, @State, @Country, @Zip, @CityId, @StateId"
			 + ", @CountryId, @IsOnline );" + IdentityStatement;;
            var dbparams = BuildParams(entity);            
            object result = _db.ExecuteScalarText(sql, dbparams);
            entity.Id = Convert.ToInt32(result);
            return entity;
        }


        /// <summary>
        /// Update the entity using sql.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        public override Event Update(Event entity)
        {
            string sql = "update Events set [CreateDate] = @CreateDate, [UpdateDate] = @UpdateDate, [CreateUser] = @CreateUser, [UpdateUser] = @UpdateUser, [UpdateComment] = @UpdateComment, [IsActive] = @IsActive"
			 + ", [UserId] = @UserId, [Title] = @Title, [Summary] = @Summary, [Description] = @Description, [StartDate] = @StartDate, [EndDate] = @EndDate"
			 + ", [StartTime] = @StartTime, [EndTime] = @EndTime, [Email] = @Email, [Phone] = @Phone, [Url] = @Url, [Keywords] = @Keywords"
			 + ", [AverageRating] = @AverageRating, [TotalLiked] = @TotalLiked, [TotalDisLiked] = @TotalDisLiked, [TotalBookMarked] = @TotalBookMarked, [TotalAbuseReports] = @TotalAbuseReports, [Street] = @Street"
			 + ", [City] = @City, [State] = @State, [Country] = @Country, [Zip] = @Zip, [CityId] = @CityId, [StateId] = @StateId"
			 + ", [CountryId] = @CountryId, [IsOnline] = @IsOnline where Id = " + entity.Id;
            var dbparams = BuildParams(entity); 
            _db.ExecuteNonQueryText(sql, dbparams);
            return entity;
        }


        public override Event Get(int id)
        {
            Event entity = base.Get(id);
            
            return entity;
        }


        protected virtual DbParameter[] BuildParams(Event entity)
        {
            var dbparams = new List<DbParameter>();
            			dbparams.Add(BuildParam("@CreateDate", SqlDbType.DateTime, entity.CreateDate));
			dbparams.Add(BuildParam("@UpdateDate", SqlDbType.DateTime, entity.UpdateDate));
			dbparams.Add(BuildParam("@CreateUser", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.CreateUser) ? "" : entity.CreateUser));
			dbparams.Add(BuildParam("@UpdateUser", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.UpdateUser) ? "" : entity.UpdateUser));
			dbparams.Add(BuildParam("@UpdateComment", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.UpdateComment) ? "" : entity.UpdateComment));
			dbparams.Add(BuildParam("@IsActive", SqlDbType.Bit, entity.IsActive));
			dbparams.Add(BuildParam("@UserId", SqlDbType.Int, entity.UserId));
			dbparams.Add(BuildParam("@Title", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.Title) ? "" : entity.Title));
			dbparams.Add(BuildParam("@Summary", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.Summary) ? "" : entity.Summary));
			dbparams.Add(BuildParam("@Description", SqlDbType.NText, string.IsNullOrEmpty(entity.Description) ? "" : entity.Description));
			dbparams.Add(BuildParam("@StartDate", SqlDbType.DateTime, entity.StartDate));
			dbparams.Add(BuildParam("@EndDate", SqlDbType.DateTime, entity.EndDate));
			dbparams.Add(BuildParam("@StartTime", SqlDbType.Int, entity.StartTime));
			dbparams.Add(BuildParam("@EndTime", SqlDbType.Int, entity.EndTime));
			dbparams.Add(BuildParam("@Email", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.Email) ? "" : entity.Email));
			dbparams.Add(BuildParam("@Phone", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.Phone) ? "" : entity.Phone));
			dbparams.Add(BuildParam("@Url", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.Url) ? "" : entity.Url));
			dbparams.Add(BuildParam("@Keywords", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.Keywords) ? "" : entity.Keywords));
			dbparams.Add(BuildParam("@AverageRating", SqlDbType.Int, entity.AverageRating));
			dbparams.Add(BuildParam("@TotalLiked", SqlDbType.Int, entity.TotalLiked));
			dbparams.Add(BuildParam("@TotalDisLiked", SqlDbType.Int, entity.TotalDisLiked));
			dbparams.Add(BuildParam("@TotalBookMarked", SqlDbType.Int, entity.TotalBookMarked));
			dbparams.Add(BuildParam("@TotalAbuseReports", SqlDbType.Int, entity.TotalAbuseReports));
			dbparams.Add(BuildParam("@Street", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.Address.Street) ? "" : entity.Address.Street));
			dbparams.Add(BuildParam("@City", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.Address.City) ? "" : entity.Address.City));
			dbparams.Add(BuildParam("@State", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.Address.State) ? "" : entity.Address.State));
			dbparams.Add(BuildParam("@Country", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.Address.Country) ? "" : entity.Address.Country));
			dbparams.Add(BuildParam("@Zip", SqlDbType.NVarChar, string.IsNullOrEmpty(entity.Address.Zip) ? "" : entity.Address.Zip));
			dbparams.Add(BuildParam("@CityId", SqlDbType.Int, entity.Address.CityId));
			dbparams.Add(BuildParam("@StateId", SqlDbType.Int, entity.Address.StateId));
			dbparams.Add(BuildParam("@CountryId", SqlDbType.Int, entity.Address.CountryId));
			dbparams.Add(BuildParam("@IsOnline", SqlDbType.Bit, entity.Address.IsOnline));

            return dbparams.ToArray();
        }


        protected virtual DbParameter BuildParam(string name, SqlDbType dbType, object val)
        {
            var param = new SqlParameter(name, dbType);
            param.Value = val;
            return param;
        }

    }


    
    /// <summary>
    /// RowMapper for Event.
    /// </summary>
    /// <typeparam name="?"></typeparam>
    public partial class EventRowMapper : EntityRowMapper<Event>, IEntityRowMapper<Event>
    {
        public override Event MapRow(IDataReader reader, int rowNumber)
        {
            Event entity = Event.New();
            			entity.Id = reader["Id"] == DBNull.Value ? 0 : (int)reader["Id"];
			entity.CreateDate = reader["CreateDate"] == DBNull.Value ? DateTime.MinValue : (DateTime)reader["CreateDate"];
			entity.UpdateDate = reader["UpdateDate"] == DBNull.Value ? DateTime.MinValue : (DateTime)reader["UpdateDate"];
			entity.CreateUser = reader["CreateUser"] == DBNull.Value ? string.Empty : reader["CreateUser"].ToString();
			entity.UpdateUser = reader["UpdateUser"] == DBNull.Value ? string.Empty : reader["UpdateUser"].ToString();
			entity.UpdateComment = reader["UpdateComment"] == DBNull.Value ? string.Empty : reader["UpdateComment"].ToString();
			entity.IsActive = reader["IsActive"] == DBNull.Value ? false : (bool)reader["IsActive"];
			entity.UserId = reader["UserId"] == DBNull.Value ? 0 : (int)reader["UserId"];
			entity.Title = reader["Title"] == DBNull.Value ? string.Empty : reader["Title"].ToString();
			entity.Summary = reader["Summary"] == DBNull.Value ? string.Empty : reader["Summary"].ToString();
			entity.Description = reader["Description"] == DBNull.Value ? string.Empty : reader["Description"].ToString();
			entity.StartDate = reader["StartDate"] == DBNull.Value ? DateTime.MinValue : (DateTime)reader["StartDate"];
			entity.EndDate = reader["EndDate"] == DBNull.Value ? DateTime.MinValue : (DateTime)reader["EndDate"];
			entity.StartTime = reader["StartTime"] == DBNull.Value ? 0 : (int)reader["StartTime"];
			entity.EndTime = reader["EndTime"] == DBNull.Value ? 0 : (int)reader["EndTime"];
			entity.Email = reader["Email"] == DBNull.Value ? string.Empty : reader["Email"].ToString();
			entity.Phone = reader["Phone"] == DBNull.Value ? string.Empty : reader["Phone"].ToString();
			entity.Url = reader["Url"] == DBNull.Value ? string.Empty : reader["Url"].ToString();
			entity.Keywords = reader["Keywords"] == DBNull.Value ? string.Empty : reader["Keywords"].ToString();
			entity.AverageRating = reader["AverageRating"] == DBNull.Value ? 0 : (int)reader["AverageRating"];
			entity.TotalLiked = reader["TotalLiked"] == DBNull.Value ? 0 : (int)reader["TotalLiked"];
			entity.TotalDisLiked = reader["TotalDisLiked"] == DBNull.Value ? 0 : (int)reader["TotalDisLiked"];
			entity.TotalBookMarked = reader["TotalBookMarked"] == DBNull.Value ? 0 : (int)reader["TotalBookMarked"];
			entity.TotalAbuseReports = reader["TotalAbuseReports"] == DBNull.Value ? 0 : (int)reader["TotalAbuseReports"];
            entity.Address = new Address();
			entity.Address.Street = reader["Street"] == DBNull.Value ? string.Empty : reader["Street"].ToString();
			entity.Address.City = reader["City"] == DBNull.Value ? string.Empty : reader["City"].ToString();
			entity.Address.State = reader["State"] == DBNull.Value ? string.Empty : reader["State"].ToString();
			entity.Address.Country = reader["Country"] == DBNull.Value ? string.Empty : reader["Country"].ToString();
			entity.Address.Zip = reader["Zip"] == DBNull.Value ? string.Empty : reader["Zip"].ToString();
			entity.Address.CityId = reader["CityId"] == DBNull.Value ? 0 : (int)reader["CityId"];
			entity.Address.StateId = reader["StateId"] == DBNull.Value ? 0 : (int)reader["StateId"];
			entity.Address.CountryId = reader["CountryId"] == DBNull.Value ? 0 : (int)reader["CountryId"];
			entity.Address.IsOnline = reader["IsOnline"] == DBNull.Value ? false : (bool)reader["IsOnline"];

            return entity;
        }
    }
}