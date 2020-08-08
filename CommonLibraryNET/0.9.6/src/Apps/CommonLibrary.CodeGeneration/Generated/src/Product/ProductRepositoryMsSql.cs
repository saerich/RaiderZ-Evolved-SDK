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



namespace ComLib.WebModules.Products
{
    /// <summary>
    /// Generic repository for persisting Product.
    /// </summary>
    public partial class ProductRepository : RepositorySql<Product>
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="NamedQueryRepository"/> class.
        /// </summary>
        public ProductRepository() { }

        
        /// <summary>
        /// Initializes a new instance of the <see cref="Repository&lt;TId, T&gt;"/> class.
        /// </summary>
        /// <param name="connectionInfo">The connection string.</param>
        public  ProductRepository(string connectionString) : base(connectionString)
        {
        }


        /// <summary>
        /// Initializes a new instance of the <see cref="Repository&lt;TId, T&gt;"/> class.
        /// </summary>
        /// <param name="connectionInfo">The connection info.</param>
        /// <param name="helper">The helper.</param>
        public ProductRepository(ConnectionInfo connectionInfo) : base(connectionInfo)
        {            
        }


        /// <summary>
        /// Initializes a new instance of the <see cref="Repository&lt;TId, T&gt;"/> class.
        /// </summary>
        /// <param name="connectionInfo">The connection info.</param>
        /// <param name="helper">The helper.</param>
        public ProductRepository(ConnectionInfo connectionInfo, IDatabase db)
            : base(connectionInfo, db)
        {
        }


        /// <summary>
        /// Initialize the rowmapper
        /// </summary>
        public override void Init(ConnectionInfo connectionInfo, IDatabase db)
        {
            base.Init(connectionInfo, db);
            this.RowMapper = new ProductRowMapper();
        }


        /// <summary>
        /// Create the entity using sql.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        public override Product Create(Product entity)
        {
            string sql = "insert into Products ( "
                       + "[CreateDate], [UpdateDate], [CreateUser], [UpdateUser], [UpdateComment], [IsActive], "
			           + "[Make], [Model], [AvailableDate], [Cost], [IsInStock]"
			           + ") " + "VALUES (@CreateDate, @UpdateDate, @CreateUser, @UpdateUser, @UpdateComment, @IsActive, "
                       + "@Make, @Model, @AvailableDate, @Cost, @IsInStock" + ");" + IdentityStatement;

            var dbparams = BuildParams(entity);
            object result = _db.ExecuteScalarText(sql, dbparams);
            entity.Id = Convert.ToInt32(result);
            return entity;
        }


        private DbParameter[] BuildParams(Product entity)
        {
            var dbparams = new List<DbParameter>();
            dbparams.Add(BuildParam("@CreateDate", SqlDbType.DateTime, entity.CreateDate));
            dbparams.Add(BuildParam("@UpdateDate", SqlDbType.DateTime, entity.UpdateDate));
            dbparams.Add(BuildParam("@CreateUser", SqlDbType.NVarChar, entity.CreateUser));
            dbparams.Add(BuildParam("@UpdateUser", SqlDbType.NVarChar, entity.UpdateUser));
            dbparams.Add(BuildParam("@UpdateComment", SqlDbType.NVarChar, entity.UpdateComment));
            dbparams.Add(BuildParam("@IsActive", SqlDbType.Bit, entity.IsActive));
            dbparams.Add(BuildParam("@Make", SqlDbType.NVarChar, entity.Make));
            dbparams.Add(BuildParam("@Model", SqlDbType.NVarChar, entity.Model));
            dbparams.Add(BuildParam("@AvailableDate", SqlDbType.DateTime, entity.AvailableDate));
            dbparams.Add(BuildParam("@Cost", SqlDbType.Decimal, entity.Cost));
            dbparams.Add(BuildParam("@IsInStock", SqlDbType.Bit, entity.IsInStock));
            return dbparams.ToArray();
        }


        private DbParameter BuildParam(string name, SqlDbType dbType, object val)
        {
            var param = new SqlParameter(name, dbType);
            param.Value = val;
            return param;
        }


        /// <summary>
        /// Update the entity using sql.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        public override Product Update(Product entity)
        {
            string sql = "update Products set "
                + "  [CreateDate] = @CreateDate"
                + ", [UpdateDate] = @UpdateDate"
                + ", [CreateUser] = @CreateUser"
                + ", [UpdateUser] = @UpdateUser"
                + ", [UpdateComment] = @UpdateComment"
                + ", [IsActive] = @IsActive"
                + ", [Make] = @Make"
                + ", [Model] = @Model"
                + ", [AvailableDate] = @AvailableDate"
                + ", [Cost] = @Cost"
                + ", [IsInStock] = @IsInStock" 
                + " where Id = " + entity.Id;

            var dbparams = BuildParams(entity);
            _db.ExecuteNonQueryText(sql, dbparams);
            return entity;
        }


        public override Product Get(int id)
        {
            Product entity = base.Get(id);
            
            return entity;
        }
    }


    
    /// <summary>
    /// RowMapper for Product.
    /// </summary>
    /// <typeparam name="?"></typeparam>
    public partial class ProductRowMapper : EntityRowMapper<Product>, IEntityRowMapper<Product>
    {
        public override Product MapRow(IDataReader reader, int rowNumber)
        {
            Product entity = Product.New();
            entity.Id = (int)reader["Id"];
			entity.CreateDate = reader["CreateDate"] == DBNull.Value ? DateTime.MinValue : (DateTime)reader["CreateDate"];
			entity.UpdateDate = reader["UpdateDate"] == DBNull.Value ? DateTime.MinValue : (DateTime)reader["UpdateDate"];
			entity.CreateUser = reader["CreateUser"] == DBNull.Value ? string.Empty : reader["CreateUser"].ToString();
			entity.UpdateUser = reader["UpdateUser"] == DBNull.Value ? string.Empty : reader["UpdateUser"].ToString();
			entity.UpdateComment = reader["UpdateComment"] == DBNull.Value ? string.Empty : reader["UpdateComment"].ToString();
			entity.IsActive = reader["IsActive"] == DBNull.Value ? false : (bool)reader["IsActive"];
			entity.Make = reader["Make"] == DBNull.Value ? string.Empty : reader["Make"].ToString();
			entity.Model = reader["Model"] == DBNull.Value ? string.Empty : reader["Model"].ToString();
			entity.AvailableDate = reader["AvailableDate"] == DBNull.Value ? DateTime.MinValue : (DateTime)reader["AvailableDate"];
			entity.Cost = reader["Cost"] == DBNull.Value ? 0 : Convert.ToDouble(reader["Cost"]);
			entity.IsInStock = reader["IsInStock"] == DBNull.Value ? false : (bool)reader["IsInStock"];
            return entity;
        }
    }
}