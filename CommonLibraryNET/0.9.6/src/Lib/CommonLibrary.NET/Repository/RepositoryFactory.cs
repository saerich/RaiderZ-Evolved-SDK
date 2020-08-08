using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using ComLib;
using ComLib.Data;
using ComLib.Entities;


namespace ComLib.Entities
{
    /// <summary>
    /// Repository factory for building/initializing a entity specific repository.
    /// </summary>
    public class RepositoryFactory
    {
        private static Action<string, IRepositoryConfigurable> _configurator;
        private static RepositoryConfiguratorDefault _defaultConfigurator = new RepositoryConfiguratorDefault();

        static RepositoryFactory()
        {
            Init();
        }


        /// <summary>
        /// Initialize the repository configurator.
        /// </summary>
        /// <param name="configurator"></param>
        public static void Init(Action<string, IRepositoryConfigurable> configurator)
        {
            _configurator = configurator;
        }


        /// <summary>
        /// Initialize the repository configurator using only the connection string.
        /// This results in using the RepositoryConfiguratorDefault and the default database provider "System.Data.SqlClient".
        /// </summary>
        /// <param name="configurator"></param>
        public static void Add(string connectionString)
        {
            Add(new ConnectionInfo(connectionString));
        }


        /// <summary>
        /// Initialize the repository configurator using only the connection provided.
        /// This results in using the RepositoryConfiguratorDefault.
        /// </summary>
        /// <param name="configurator"></param>
        public static void Add(ConnectionInfo connection)
        {
            _defaultConfigurator.Add(connection);
        }


        /// <summary>
        /// Add connection associated w/ key.
        /// </summary>
        /// <param name="id"></param>
        /// <param name="connectionInfo"></param>
        public static void Add(string id, ConnectionInfo connectionInfo)
        {
            _defaultConfigurator.Add(id, connectionInfo);
        }


        /// <summary>
        /// Default initialization.
        /// </summary>
        private static void Init()
        {            
            _configurator = new Action<string, IRepositoryConfigurable>(_defaultConfigurator.Configure);
        }


        /// <summary>
        /// Initialize the repository.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="repository"></param>
        /// <returns></returns>
        public static IRepository<T> Create<T>(Func<IRepository<T>> action) where T: IEntity
        {
            // Allow Entity to create repository but initialize it here.
            IRepository<T> repository = action();
            _configurator(null, repository);
            return repository;
        }


        /// <summary>
        /// Initialize the repository.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="repository"></param>
        /// <returns></returns>
        public static void Configure(string connectionId, IRepositoryConfigurable repository)
        {
            // Configure the repository but initialize it here.
            _configurator(connectionId, repository);
        }
    }



    /// <summary>
    /// Default repository configurator to set the connection connection string and dbhelper.
    /// </summary>
    public class RepositoryConfiguratorDefault 
    {
        private IDictionary<string, ConnectionInfo> _connections = new Dictionary<string, ConnectionInfo>();
        private Func<string, string> _connectionFetcher;
        private bool _hasConnectionFetcher;


        /// <summary>
        /// The connection object.
        /// </summary>
        public ConnectionInfo DefaultConnection { get; set; }


        /// <summary>
        /// Initialize the connection.
        /// </summary>
        /// <param name="connection"></param>
        public RepositoryConfiguratorDefault()
        {
        }


        /// <summary>
        /// Initialize the connection.
        /// </summary>
        /// <param name="connection"></param>
        public RepositoryConfiguratorDefault(ConnectionInfo connection)
        {
            Add(connection);
        }


        /// <summary>
        /// Initialize the connection.
        /// </summary>
        /// <param name="connection"></param>
        public void Add(ConnectionInfo connection)
        {
            DefaultConnection = connection;
            _connections["default"] = connection;
        }


        /// <summary>
        /// Initialize the connection.
        /// </summary>
        /// <param name="connection"></param>
        public void Add(string id, ConnectionInfo connection)
        {
            _connections[id] = connection;

            if (string.Compare(id, "default", true) == 0)
                DefaultConnection = connection;
        }


        /// <summary>
        /// Configure the repository with the connection and dbhelper.
        /// </summary>
        /// <param name="connectionId"></param>
        /// <param name="repository"></param>
        public void Configure(string connectionId, IRepositoryConfigurable repository)
        {
            ConnectionInfo con = null;

            // No connection Id? Use default.
            if (string.IsNullOrEmpty(connectionId))
            {
                con = DefaultConnection;
                
            }
            else if (_hasConnectionFetcher)
            {
                var constr = _connectionFetcher(connectionId);
                con = new ConnectionInfo(constr);
            }
            else if (!_connections.ContainsKey(connectionId))
                throw new ArgumentException("Connection Id : " + connectionId + " does not exist.");
            else
            {
                con = _connections[connectionId];
            }

            repository.Connection = con;
            repository.Database = new Database(con);
        }
    }
}
