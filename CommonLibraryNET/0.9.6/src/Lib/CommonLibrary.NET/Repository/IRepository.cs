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
using System.Collections;
using System.Collections.ObjectModel;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using System.Linq.Expressions;
using System.Data;

using ComLib;
using ComLib.Data;


namespace ComLib.Entities
{    
    /// <summary>
    /// 
    /// </summary>
    public interface IRepositoryConfigurable
    {
        /// <summary>
        /// Gets or sets the db helper.
        /// </summary>
        /// <value>The helper.</value>
        IDatabase Database { get; set; }


        /// <summary>
        /// Gets or sets the connection.
        /// </summary>
        /// <value>The connection.</value>
        ConnectionInfo Connection { get; set; }


        /// <summary>
        /// Gets the connection STR.
        /// </summary>
        /// <value>The connection STR.</value>
        string ConnectionString { get; }
    }



    /// <summary>
    /// Repository that is Queryable withOUT CRUD/persistance methods.
    /// This allows some basic functionality across any table, regardless
    /// of the entities it stores.
    /// </summary>
    public interface IRepositoryQueryable : IRepositoryConfigurable
    {
        /// <summary>
        /// The name of the table associated w/ this repository.
        /// </summary>
        string TableName { get; set; }


        /// <summary>
        /// Prefix to use for parameterized queries.
        /// e.g. @ for sql server.
        /// </summary>
        string ParamPrefix { get; set; }


        #region Named Filters
        /// <summary>
        /// Dictionary of named filters/criteria.
        /// Useful when doing Count("ActiveUsers");
        /// Or Avg("Rating", "PopularPosts")
        /// </summary>
        IDictionary<string, IQuery> NamedFilters { get; }


        /// <summary>
        /// Add named filter.
        /// e.g. "Published last week" => Criteria.Where("CreateDate").Between(1.Week.Ago, DateTime.Now)
        /// </summary>
        /// <param name="filterName"></param>
        /// <param name="criteria"></param>
        void AddNamedFilter(string filterName, IQuery criteria);
        #endregion


        #region Sum, Min, Max, Count, Avg, Count, Distinct, Group By
        double Sum(string columnName);
        double Sum(string columnName, IQuery criteria);
        double Sum(string columnName, string namedFilter);
        
        double Min(string columnName);
        double Min(string columnName, IQuery criteria);
        double Min(string columnName, string namedFilter);

        double Max(string columnName);
        double Max(string columnName, IQuery criteria);
        double Max(string columnName, string namedFilter);

        double Avg(string columnName);
        double Avg(string columnName, IQuery criteria);
        double Avg(string columnName, string namedFilter);

        int Count();
        int Count(IQuery criteria);
        int Count(string filterName);
        int CountBySql(string sql);

        List<TVal> Distinct<TVal>(string columnName);
        List<TVal> Distinct<TVal>(string columnName, IQuery criteria);
        List<TVal> Distinct<TVal>(string columnName, string namedFilter);

        List<KeyValuePair<TGroup, int>> Group<TGroup>(string columnName);
        List<KeyValuePair<TGroup, int>> Group<TGroup>(string columnName, IQuery criteria);
        List<KeyValuePair<TGroup, int>> Group<TGroup>(string columnName, string namedFilter);

        DataTable Group(IQuery criteria, params string[] columnNames);
        DataTable GroupNamedFilter(string namedFilter, params string[] columnNames);
        #endregion


        #region Exists
        /// <summary>
        /// Checks whether there are any records that the named filter
        /// </summary>
        /// <param name="sql"></param>
        /// <returns></returns>
        bool Any(string filtername);


        /// <summary>
        /// Chekcs where there are any records that match the criteria filter.
        /// </summary>
        /// <param name="criteria"></param>
        /// <returns></returns>
        bool Any(IQuery criteria);


        /// <summary>
        /// Get datatable using the sql supplied.
        /// </summary>
        /// <param name="criteria"></param>
        /// <returns></returns>
        bool AnyBySql(string sql);
        #endregion


        #region To Table
        /// <summary>
        /// Get Table containing all the records.
        /// </summary>
        /// <returns></returns>
        DataTable ToTable();


        /// <summary>
        /// Get datatable using the IQuery filter
        /// </summary>
        /// <param name="criteria"></param>
        /// <returns></returns>
        DataTable ToTable(IQuery criteria);


        /// <summary>
        /// Get datatable using the filtername.
        /// </summary>
        /// <param name="criteria"></param>
        /// <returns></returns>
        DataTable ToTable(string filterName);


        /// <summary>
        /// Get datatable using the sql filter.
        /// </summary>
        /// <param name="sql"></param>
        /// <returns></returns>
        DataTable ToTableBySql(string sql);
        #endregion
    }



    /// <summary>
    /// Interface for a DAO(Data Access Object) to support CRUD operations.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <typeparam name="IdT"></typeparam>
    public interface IRepositoryWithId<TId, T> 
    {

        #region CRUD - Create, Get, Update, Delete
        /// <summary>
        /// Create an entity.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        T Create(T entity);


        /// <summary>
        /// Create a list of entities.
        /// </summary>
        /// <param name="entities"></param>
        void Create(IList<T> entities);


        /// <summary>
        /// Creates the entity conditionally based on whether they exists in the datastore.
        /// Existance in the datastore is done by finding any entities w/ matching values for the 
        /// <paramref name="checkFields"/> supplied.
        /// </summary>
        /// <param name="entities"></param>
        /// <param name="conditions"></param>
        void Create(T entity, params Expression<Func<T, object>>[] checkFields);


        /// <summary>
        /// Copies the entity and returns the copy.
        /// </summary>
        /// <param name="entity"></param>
        T Copy(T entity);


        /// <summary>
        /// Retrieve the entity by it's key/id.
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        T Get(TId id);


        /*
        /// <summary>
        /// Gets the specified entity and loads the TRelation. Defaults the foreign key to typeof(TRelation).Name + Id.
        /// Defaults the Property name to typeof(TRelation).
        /// </summary>
        /// <typeparam name="TRelation">The type of the relation.</typeparam>
        /// <param name="id">The id.</param>
        /// <returns></returns>
        T Get<TRelation>(int id) where TRelation : class;


        /// <summary>
        /// Gets the specified entity and loads the 1-to-Many TRelation. Defaults the foreign key to typeof(TRelation).Name + Id.
        /// Defaults the Property name to typeof(TRelation)"s".
        /// </summary>
        /// <typeparam name="TRelation">The type of the relation.</typeparam>
        /// <param name="id">The id.</param>
        /// <param name="pageIndex">Index of the page.</param>
        /// <param name="pageSize">Size of the page.</param>
        /// <returns></returns>
        T Get<TRelation>(int id, int pageIndex, int pageSize) where TRelation : class;
        */

        /// <summary>
        /// Retrieve all the entities.
        /// </summary>
        /// <returns></returns>
        IList<T> GetAll();


        /// <summary>
        /// Retrieve all the entities into a non-generic list.
        /// </summary>
        /// <returns></returns>
        IList GetAllItems();


        /// <summary>
        /// UPdate the entity.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        T Update(T entity);


        /// <summary>
        /// Create a list of entities.
        /// </summary>
        /// <param name="entities"></param>
        void Update(IList<T> entities);


        /// <summary>
        /// Delete the entitiy by it's key/id.
        /// </summary>
        /// <param name="id"></param>
        void Delete(TId id);


        /// <summary>
        /// Delete the entity.
        /// </summary>
        /// <param name="entity"></param>
        void Delete(T entity);


        /// <summary>
        /// Delete using the expression.
        /// e.g. entity.LogLevel == 1
        /// </summary>
        /// <param name="expression"></param>
        void Delete(Expression<Func<T, bool>> expression);


        /// <summary>
        /// Delete by criteria
        /// </summary>
        /// <param name="criteria"></param>
        void Delete(IQuery criteria);


        /// <summary>
        /// Delete by criteria
        /// </summary>
        /// <param name="criteria"></param>
        void DeleteByNamedFilter(string filterName);


        /// <summary>
        /// Delete all entities from the repository.
        /// </summary>
        void DeleteAll();


        /// <summary>
        /// Create or update an entity.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        T Save(T entity);


        /// <summary>
        /// Create or update all entities
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        void Save(IList<T> entities);
        #endregion


        #region Sum, Min, Max, Count, Avg, Count, Distinct, Group By
        double Sum(Expression<Func<T, object>> exp);
        
        
        double Min(Expression<Func<T, object>> exp);
        
        
        double Max(Expression<Func<T, object>> exp);
        
        
        double Avg(Expression<Func<T, object>> exp);
        
        
        List<TVal> Distinct<TVal>(Expression<Func<T, object>> exp);
        
        
        List<KeyValuePair<TGroup, int>> Group<TGroup>(Expression<Func<T, object>> exp);        
        #endregion


        #region Increment / Decrement
        void Increment(string fieldName, int by, int id);
        void Decrement(string fieldName, int by, int id);
        void Increment(Expression<Func<T, object>> exp, int by, int id);
        void Decrement(Expression<Func<T, object>> exp, int by, int id);
        #endregion


        #region Lookup
        /// <summary>
        /// Create dictionary of all entities using the Id.
        /// </summary>
        /// <returns></returns>
        IDictionary<TId, T> ToLookUp();


        /// <summary>
        /// Create dictionary of all entities using the Id after applying named filter.
        /// </summary>
        /// <returns></returns>
        IDictionary<int, T> ToLookUp(string namedFilter);


        /// <summary>
        /// Create dictionary of all entities using the Id after applying named filter.
        /// </summary>
        /// <returns></returns>
        IDictionary<int, T> ToLookUp(IQuery<T> query);


        /// <summary>
        /// Lookup on 2 values. Creates lookup on Id and Id2
        /// </summary>
        /// <typeparam name="Id2"></typeparam>
        /// <param name="propName"></param>
        /// <returns></returns>
        LookupMulti<T> ToLookUpMulti<Id2>(string propName);


        /// <summary>
        /// Lookup on 2 values. Creates lookup on Id and Id2
        /// </summary>
        /// <typeparam name="Id2"></typeparam>
        /// <param name="propName"></param>
        /// <returns></returns>
        LookupMulti<T> ToLookUpMulti<Id2>(Expression<Func<T, object>> exp);


        /// <summary>
        /// Lookup on 2 values. Creates lookup on Id and Id2 after applying the named Filter.
        /// </summary>
        /// <typeparam name="Id2"></typeparam>
        /// <param name="propName"></param>
        /// <returns></returns>
        LookupMulti<T> ToLookUpMulti<Id2>(string propName, string filterName);
        #endregion


        #region Find
        /// <summary>
        /// Find by using the sql string
        /// </summary>
        /// <param name="sql"></param>
        /// <returns></returns>
        IList<T> Find(string sql);


        /// <summary>
        /// Find by query
        /// </summary>
        /// <returns></returns>
        IList<T> Find(IQuery criteria);


        /// <summary>
        /// Find by named filter.
        /// </summary>
        /// <param name="namedFilter">e.g. "Most Popular Posts"</param>
        /// <returns></returns>
        IList<T> FindByNamedFilter(string namedFilter);


        /// <summary>
        /// Find using the sql provided.
        /// </summary>
        /// <param name="sql"></param>
        /// <param name="isFullSql">If true, assumes that the sql contains "select * from table ..."</param>
        /// <returns></returns>
        IList<T> Find(string sql, bool isFullSql);


        /// <summary>
        /// Find items by page using sql
        /// </summary>        
        /// <param name="sql">Sql to use for filter</param>
        /// <param name="pageNumber">1</param>
        /// <param name="pageSize">15 ( records per page )</param>
        /// <returns></returns>
        PagedList<T> Find(string sql, int pageNumber, int pageSize);
        
        
        /// <summary>
        /// Find items by page using criteria
        /// </summary>
        /// <param name="criteria">Criteria object representing filter.</param>
        /// <param name="pageNumber">1</param>
        /// <param name="pageSize">15 ( records per page )</param>
        /// <returns></returns>
        PagedList<T> Find(IQuery criteria, int pageNumber, int pageSize);


        /// <summary>
        /// Find items by page using named filter
        /// </summary>
        /// <param name="pageNumber">1</param>
        /// <param name="pageSize">15 ( records per page )</param>
        /// <returns></returns>
        PagedList<T> FindByNamedFilter(string filterName, int pageNumber, int pageSize);


        /// <summary>
        /// Get items by page based on latest / most recent create date.
        /// </summary>
        /// <param name="pageNumber">1</param>
        /// <param name="pageSize">15 ( records per page )</param>
        /// <returns></returns>
        PagedList<T> FindRecent(int pageNumber, int pageSize);


        /// <summary>
        /// Find the recent items and convert them to a different type.
        /// </summary>
        /// <typeparam name="T2"></typeparam>
        /// <param name="pageNumber"></param>
        /// <param name="pageSize"></param>
        /// <returns></returns>
        IList<T2> FindRecentAs<T2>(int pageNumber, int pageSize);
        #endregion


        #region Misc
        /// <summary>
        /// Get the first one that matches the filter.
        /// </summary>
        /// <param name="filter"></param>
        /// <returns></returns>
        T First(string filter);


        /// <summary>
        /// Get the first one that matches the filter.
        /// </summary>
        /// <param name="filter"></param>
        /// <returns></returns>
        T First(IQuery filter);


        /// <summary>
        /// Get the first one that matches the filter.
        /// </summary>
        /// <param name="filter"></param>
        /// <returns></returns>
        T FirstByNamedFilter(string filterName);
        #endregion
    }



    /// <summary>
    /// Interface for Repository with default primary key as int.
    /// </summary>
    public interface IRepository<T> : IRepositoryWithId<int, T>, IRepositoryQueryable, IRepositoryConfigurable
    {
    }



    /// <summary>
    /// Wrapper around Repository to version the entities.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public interface IRepositoryVersionable<T> : IRepository<T>
    {
        /// <summary>
        /// To enable / disable versioning.
        /// </summary>
        bool IsVersioned { get; set; }


        bool IsVersioningStrict { get; }


        /// <summary>
        /// Roll back the version of the entity with the specified id.
        /// </summary>
        /// <param name="id"></param>
        void RollBack(int id);


        /// <summary>
        /// Rollback the specified entity.
        /// </summary>
        /// <param name="entity"></param>
        void RollBack(T entity);


        /// <summary>
        /// Get all the versions ( latest / historic ) for the specified entity with id.
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        IList<T> GetAllVersions(int id);


        /// <summary>
        /// Find entities using the criteria with option to get only the latest versions and
        /// not all ( latest/historic ).
        /// </summary>
        /// <param name="criteria"></param>
        /// <param name="onlyLatestVersions"></param>
        /// <returns></returns>
        IList<T> Find(IQuery criteria, bool onlyLatestVersions);
    }    
}