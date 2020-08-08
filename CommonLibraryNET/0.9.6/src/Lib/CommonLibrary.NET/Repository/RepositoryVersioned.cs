using System;
using System.Collections.Generic;
using System.Linq;
using System.Collections;
using System.Text;
using System.Data;
using System.Data.Common;
using System.Linq.Expressions;

using ComLib;
using ComLib.Entities;
using ComLib.Data;



namespace ComLib.Entities
{
    /// <summary>
    /// Repository that versions the Entities on updates.
    /// </summary>
    /// <remarks>
    /// This works via the following:
    /// 1. Decorates any existing Repository you want to use.
    /// 2. The entity T of the repository must implement IEntityVersionable
    /// 3. The Update method will create a new version of the entity without changing it's primary key.
    /// 4. The VersionRefId of historic/older versions becomes the reference to the primary key.
    ///    and the historic/older versions have a different primary key.
    /// </remarks>
    /// <typeparam name="T"></typeparam>
    public class RepositoryVersioned<T> : RepositoryBase<T>, IRepositoryVersionable<T> where T : class, IEntity
    {
        protected IRepository<T> _repo;
        protected bool _isVersioned;
        protected bool _isVersionedStrict;
        private bool _useOptimizedQuery;


        public RepositoryVersioned(IRepository<T> repo) : this(repo, true, true)
        {
        }


        public RepositoryVersioned(IRepository<T> repo, bool isVersionedStrict, bool useOptimizedQueries)
        {
            _repo = repo;
            _isVersioned = true;
            _isVersionedStrict = isVersionedStrict;
            _useOptimizedQuery = useOptimizedQueries;
        }


        public bool IsVersioned { get { return _isVersioned; } set { _isVersioned = value; } }


        public bool IsVersioningStrict { get { return _isVersionedStrict; } }


        /// <summary>
        /// Roll back the entity w/ the specified id.
        /// </summary>
        /// <param name="id"></param>
        public void RollBack(int id)
        {
            var entity = _repo.Get(id);
            RollBack(entity);
        }


        /// <summary>
        /// Rollback the entity to the prior version.
        /// </summary>
        /// <param name="entity"></param>
        public void RollBack(T entity)
        {
            // Validation versioning conditions.
            if (!(entity is IEntityVersioned))
                throw new InvalidOperationException("Entity to rollback is not a versioned entity, does not support IEntityVersioned interface");

            if (_isVersioned && _isVersionedStrict && !((IEntityVersioned)entity).IsLatestVersion())
                throw new InvalidOperationException("Entity to rollback is not the latest version.");

            // Use case not supported.
            if (!((IEntityVersioned)entity).IsLatestVersion())
                return;

            int id = ((IEntityVersioned)entity).Id;

            // sql for the next to latest version.
            string optimizedQuery = "VersionRefId = " + id
                       + " and version = select max(version) from " + _repo.TableName
                                       + " where VersionRefId = " + id;

            string sql = optimizedQuery;
            if (!_useOptimizedQuery)
            {
                double versionId = _repo.Max("version", Query<IEntityVersioned>.New().Where(e => e.VersionRefId).Is(id));
                sql = "VersionRefId = " + id
                    + " and version = " + versionId;
            }
            var lastEntity = _repo.First(sql) as IEntityVersioned;
            var lastId = lastEntity.Id;

            // Update the entity by setting it's id ot the original.
            lastEntity.Id = id;
            lastEntity.VersionRefId = -1;
            _repo.Update((T)lastEntity);

            // Delete the last entity.
            _repo.Delete(lastId);
        }


        #region IRepositoryWithId<int,T> Members
        public override IDatabase Database
        {
            get { return _repo.Database; }
            set { _repo.Database = value; }
        }

        public override ConnectionInfo Connection
        {
            get { return _repo.Connection; }
            set { _repo.Connection = value; }
        }

        public override string ConnectionString
        {
            get { return _repo.ConnectionString; }
        }


        public override string TableName
        {
            get { return _repo.TableName; }
        }

        public override T Create(T entity)
        {
            if (entity is IEntityVersioned)
                ((IEntityVersioned)entity).VersionRefId = -1;

            return _repo.Create(entity);
        }

        public override T Get(int id)
        {
            return _repo.Get(id);
        }


        /// <summary>
        /// Returns the latest versions of all entities.
        /// </summary>
        /// <returns></returns>
        public override IList<T> GetAll()
        {
            var criteria = Query<IEntityVersioned>.New();
            criteria.Where(e => e.VersionRefId).Is(-1).End();
            return _repo.Find(criteria.Builder.Build(), false);
        }


        /// <summary>
        /// Returns the latest versions of all entities.
        /// </summary>
        /// <returns></returns>
        public virtual IList<T> GetAllVersions(int id)
        {
            var criteria = Query<IEntityVersioned>.New();
            criteria.Where(e => e.Id).Is(id).Or( e => e.VersionRefId).Is(id);
            return Find(criteria, false); 
        }


        /// <summary>
        /// Updates the entity while storing historic versions
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        public override T Update(T entity)
        {
            if (!_isVersioned) return _repo.Update(entity);

            // Validation versioning conditions.
            if (!(entity is IEntityVersioned))
                throw new InvalidOperationException("Entity to rollback is not a versioned entity, does not support IEntityVersioned interface");

            if (_isVersionedStrict && !((IEntityVersioned)entity).IsLatestVersion())
                throw new InvalidOperationException("Entity to rollback is not the latest version.");

            var ventity = entity as IEntityVersioned;

            // Check if this is latest version.
            if (!ventity.IsLatestVersion()) return _repo.Update(entity);

            //  The original id.
            var id = ventity.Id;

            // Create historic version by making the VersionRefId = id 
            // instead of -1 on the current version in the database.
            var current = Get(id);
            ((IEntityVersioned)current).VersionRefId = id;
            _repo.Create(current);

            // Now increment the version and make this the lastest version.
            ventity.Version++;
            ventity.VersionRefId = -1;

            // Update as usual.
            return _repo.Update(entity);	
        }


        /// <summary>
        /// Deletes the entity and all it's versions.
        /// </summary>
        /// <param name="id"></param>
        public override void Delete(int id)
        {
            var criteria = Query<IEntityVersioned>.New();
            criteria.Where(e => e.Id).Is(id).Or(e => e.VersionRefId).Is(id);
            _repo.Delete(criteria);
        }


        public override void Delete(Expression<Func<T, bool>> expression)
        {
            _repo.Delete(expression);
        }

        public override void Delete(IQuery criteria)
        {
            _repo.Delete(criteria);
        }

        public override void DeleteAll()
        {
            _repo.DeleteAll();
        }

        public override double Sum(string columnName)
        {
            return _repo.Sum(columnName);
        }

        public override double Sum(Expression<Func<T, object>> exp)
        {
            return _repo.Sum(exp);
        }

        public override double Min(string columnName)
        {
            return _repo.Min(columnName);
        }

        public override double Min(Expression<Func<T, object>> exp)
        {
            return _repo.Min(exp);
        }

        public override double Max(string columnName)
        {
            return _repo.Max(columnName);
        }

        public override double Max(Expression<Func<T, object>> exp)
        {
            return _repo.Max(exp);
        }

        public override double Avg(string columnName)
        {
            return _repo.Avg(columnName);
        }

        public override double Avg(Expression<Func<T, object>> exp)
        {
            return _repo.Avg(exp);
        }

        public override int Count()
        {
            return _repo.Count();
        }

        public override List<TVal> Distinct<TVal>(string columnName)
        {
            return _repo.Distinct<TVal>(columnName);
        }

        public override List<TVal> Distinct<TVal>(Expression<Func<T, object>> exp)
        {
            return _repo.Distinct<TVal>(exp);
        }

        public override List<KeyValuePair<TGroup, int>> Group<TGroup>(string columnName)
        {
            return _repo.Group<TGroup>(columnName);
        }

        public override List<KeyValuePair<TGroup, int>> Group<TGroup>(Expression<Func<T, object>> exp)
        {
            return _repo.Group<TGroup>(exp);
        }

        public override IList<T> Find(string sql)
        {
            return _repo.Find(sql);
        }


        /// <summary>
        /// Finds the latest versions, excluding historic ones.
        /// </summary>
        /// <param name="criteria"></param>
        /// <returns></returns>
        public override IList<T> Find(IQuery criteria)
        {
            return Find(criteria, true);
        }


        /// <summary>
        /// Finds the latest versions, excluding historic ones.
        /// </summary>
        /// <param name="criteria"></param>
        /// <returns></returns>
        public virtual IList<T> Find(IQuery criteria, bool onlyLatestVersions)
        {
            if (onlyLatestVersions)
                criteria.AddCondition("VersionRefId", ExpressionType.Equal, -1);

            return _repo.Find(criteria);
        }


        public override IList<T> Find(string sql, bool isFullSql)
        {
            return _repo.Find(sql, isFullSql);
        }

        public override PagedList<T> Find(string sql, int pageNumber, int pageSize)
        {
            return _repo.Find(sql, pageNumber, pageSize);
        }


        /// <summary>
        /// Finds latest versions, excluding historic ones.
        /// </summary>
        /// <param name="criteria"></param>
        /// <param name="pageNumber"></param>
        /// <param name="pageSize"></param>
        /// <returns></returns>
        public override PagedList<T> Find(IQuery criteria, int pageNumber, int pageSize)
        {
            criteria.AddCondition("VersionRefId", ExpressionType.Equal, -1);
            return _repo.Find(criteria, pageNumber, pageSize);
        }

        public override PagedList<T> FindRecent(int pageNumber, int pageSize)
        {
            return _repo.FindRecent(pageNumber, pageSize);
        }

        public override T First(string filter)
        {
            return _repo.First(filter);
        }

        #endregion
    }
}
