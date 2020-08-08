using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Data;
using System.Data.Common;
using ComLib;
using ComLib.Arguments;
using ComLib.Authentication;
using ComLib.CsvParse;
using ComLib.Cryptography;
using ComLib.Diagnostics;
using ComLib.Calendars;
using ComLib.Errors;
using ComLib.Logging;
using ComLib.LocationSupport;
using ComLib.Caching;
using ComLib.Environments;
using ComLib.Configuration;
using ComLib.Subs;
using ComLib.Database;
using ComLib.Entities;
using ComLib.Locale;
using ComLib.Models;
using ComLib.EmailSupport;
using ComLib.Notifications;
using ComLib.CodeGeneration;


namespace ComLib.Samples
{
    /// <summary>
    /// Sample application.
    /// </summary>
    public class SampleApp
    {
        private string[] _args;
        private SampleAppsArguments _argsReciever = new SampleAppsArguments();
        private bool _isDatabaseSetup = false;

        /// <summary>
        /// Initialize.
        /// </summary>
        /// <param name="args"></param>
        public SampleApp(string[] args)
        {
            _args = args;
        }


        /// <summary>
        /// Run the application.
        /// </summary>
        public void Run()
        {
            // Validate arguments passed in to application.            
            if (!AcceptArguments()) return;

            // Now configure the following:
            // 1. Set the current environment among PROD, QA, UAT, DEV
            // 2. Set the current inherited configuration ( based on environment - PROD, QA etc )
            // 3. Set the current logging mechanism.
            // 4. Set the current authentication mechanism
            SetEnvironment();
            
            SetConfiguration();
                        
            SetAuthentication();

            SetLogging();
            
            // Now show all the examples.
            ShowExamples();

            Logger.ShutDown();
        }


        /// <summary>
        /// Show examples of 
        /// 1. Encryption
        /// 2. Notifications
        /// 3. Cache
        /// 4. HolidayCalendar
        /// 5. Substitutions
        /// 6. Diagnostics
        /// 7. ActiveRecord
        /// 8. Code Generation
        /// etc...
        /// </summary>
        public void ShowExamples()
        {
            // Now show examples of different components.
            DoLogging();

            DoEncryption();

            DoExceptionHandling();

            DoCache();

            DoHolidayCalendarLoad();

            DoSubstitutions();

            DoCsvLoad();

            DoDiagnostics();

            DoNotifications();

            DoIocContainer();

            DoDatabaseQuery(_isDatabaseSetup);

            DoCodeGenerator();

            DoActiveRecord(_isDatabaseSetup);            
        }


        /// <summary>
        /// Show how to parse the command line args.
        /// </summary>
        /// <returns></returns>
        public bool AcceptArguments()
        {
            // Show the arguments.
            Logger.Info(ArgsUsage.Build(_argsReciever));

            // If args not supplied. simulate them.
            if (_args == null || _args.Length == 0)
                _args = new string[] { "-env:Prod", "-date:${today}", "-dryrun:false", "MySettings", "8" };

            return Args.Accept(_args, "-", ":", _argsReciever);
        }


        /// <summary>
        /// Show how to set the environment and load the config files
        /// based on the environment.
        /// </summary>
        public void SetEnvironment()
        {
            // 1. Get environment passed from command line. -env:Prod
            // 2. Get all the environments supported.
            // 3. Create environment.
            // 4. Set current environment.
            IEnv env = new EnvDef(_argsReciever.Environment, AppConfig.GetSampleEvironments());
            Envs.Set(env);

            // Show the environment selection API.
            Logger.Info("====================================================");
            Logger.Info("ENVIRONMENTS ");
            Logger.Info("Environment name: " + Env.Name);
            Logger.Info("Environment type: " + Env.EnvType);
            Logger.Info("Environment inherits: " + Env.Inherits);
            Logger.Info("Environment file: " + Env.RefPath);
            Logger.Info("Environment prod: " + Env.IsProd);
            Logger.Info(Environment.NewLine);
        }


        /// <summary>
        /// Load the config files based on the environment and
        /// based on the inheritance of the config files.
        /// INI files are used as samples for simplicity.
        /// </summary>
        public void SetConfiguration()
        {
            // Initialize the configuration from the selected environment.
            //IConfigSource environmentInheritedConfig = EnvUtils.LoadConfigs();
            //Config.Init(environmentInheritedConfig);

            // The Config.Current is now set.
            // The Config.Current is an IConfigSource that represents
            // 1. Multiple config files.
            // 2. The multiple config files reflects the inheritance chain of the environment.            
            Logger.Info("Config Name: " + Config.Name);
            Logger.Info("Config Source: " + Config.SourcePath);
            Logger.Info("Global.ApplicationName :  " + Config.Get<string>("Global", "AppName"));
            Logger.Info("Global.MaxSessions     :  " + Config.Get<int>("Global", "MaxSessions"));
            Logger.Info("Global.IsDatabaseReady :  " + Config.Get<bool>("Global", "IsDatabaseReady"));
            Logger.Info("Global.LastFtpDate     :  " + Config.Get<DateTime>("Global", "LastFtpDate"));
            Logger.Info("Global.LastFtpDate     :  " + Config.Current["Global", "LastFtpDate"]);            
            Logger.Info(Environment.NewLine);
        }


        /// <summary>
        /// Show the logging.
        /// </summary>
        public void SetLogging()
        {
            // Show logging.
            Logger.Info("====================================================");
            Logger.Info("LOGGING ");
            Logger.Info("This is default logger which writes to the console.");
            Logger.Warn("This can be configured to use a log4net adapator.");
            Logger.Info(Environment.NewLine);
        }


        /// <summary>
        /// INitialize the auth service using windows provider.
        /// </summary>
        public void SetAuthentication()
        {
            Auth.Init(new AuthWin());

            // Show authentication API.
            Logger.Info("====================================================");
            Logger.Info("AUTHENTICATION ");
            Logger.Info("Is authenticated : " + Auth.IsAuthenticated());
            Logger.Info("Is guest         : " + Auth.IsGuest());
            Logger.Info("UserName         : " + Auth.UserName);
            Logger.Info(Environment.NewLine);
        }


        /// <summary>
        /// Show examples of logging.
        /// </summary>
        public void DoLogging()
        {            
            // 1. Default logging to console.
            Logger.Info("The default logger log to the console.");
            
            // 2. Append another logger.
            //Logger.Append(new LoggerFileBased("UserLog", Auth.UserShortName + ".txt"));
            Logger.Get("UserLog").Info("Log only to the named logger", null, null);

            // 3. Initialize w/ multiple loggers
            List<ILog> loggers = new List<ILog>() { new LogConsole(), new LogFile("ufc", "ultimatefighting.txt") };
            Logger.Init(new LogMulti("default", loggers));

            // 4. Flush the output for the "UserLog".
            Logger.Flush();
        }


        /// <summary>
        /// Show the encryption.
        /// </summary>
        public void DoEncryption()
        {
            // Encrypt using default provider. ( Symmetric TripleDes )
            string plainText = "www.knowledgedrink.com";
            string encrypted = Crypto.Encrypt(plainText);
            string decrypted = Crypto.Decrypt(encrypted);

            Logger.Info("====================================================");
            Logger.Info("CRYPTOGRAPHY ");
            Logger.Info("Encrypted : " + plainText + " to " + encrypted);
            Logger.Info("Decrypted : " + encrypted + " to " + decrypted);
            Logger.Info(Environment.NewLine);
        }


        /// <summary>
        /// Do cache example.
        /// </summary>
        public void DoCache()
        {
            Cacher.Insert("sampledata", "http://www.knowledgedrink.com");
            string cacheObject = Cacher.Get<string>("sampledata");

            Logger.Info("====================================================");
            Logger.Info("CACHE ");
            Logger.Info("Obtained from cache : '" + cacheObject + "'");
            Logger.Info(Environment.NewLine);
        }


        /// <summary>
        /// Initialize the holiday calendars.
        /// </summary>
        public void DoHolidayCalendarLoad()
        {
            /* The Holiday Calendar is defaulted with U.S. holidays. So this is unneccessary.
             * Just showing example of how to initialize the provider.
             *             
            List<HolidayItem> holidays = AppConfig.GetSampleUnitedStatesHolidays();
            HolidayCalanderDao dao = new HolidayCalanderDao("usa-holidays", holidays);
            HolidayCalendar.Init("usa-holidays", dao, DateTime.Today.Year - 1, DateTime.Today.Year + 1);            
             * 
             */
            // Get the next business date after Jan 1st.
            DateTime nextBusDay = Calendar.NextBusinessDate(new DateTime(DateTime.Today.Year, 1, 1));
            DateTime previousBusinessDay = Calendar.PreviousBusinessDate(new DateTime(DateTime.Today.Year, 1, 1));

            Logger.Info("====================================================");
            Logger.Info("HOLIDAY CALENDAR ");
            Logger.Info("Next business day after New Years : " + nextBusDay.ToShortDateString());
            Logger.Info("Previous business day before New Years : " + previousBusinessDay.ToShortDateString());
            Logger.Info(Environment.NewLine);
        }


        /// <summary>
        /// Show the default exception handling.
        /// This can be configured to use an email notifier as
        /// the exception handling provider instance.
        /// </summary>
        public void DoExceptionHandling()
        {
            ErrorManager.Register("myErrorManager", false, new CustomExceptionHandler("myErrorManager"));
            Logger.Info("====================================================");
            Logger.Info("EXCEPTION HANDLING ");

            try
            {
                throw new ArgumentException("exception handling testing");
            }
            catch (Exception ex)
            {
                // Default error handler.
                ErrorManager.Handle("Default error handling uses the Logger static class", ex);

                // Custom named error handler "myErrorManager"
                ErrorManager.Handle("Example with custom NAMED error handler.", ex, "myErrorManager");
            }
            Logger.Info(Environment.NewLine);
        }


        public void DoCsvLoad()
        {
            // This is a LEXICAL parser.
            // So this can handle quoted and non-quoted values
            // where the separator "," can be inside the quotes such as in 'Art, Classes'
            string csv = "Id, Name,      'Desc'" + Environment.NewLine
                       + "0,  Art,       'Art classes'" + Environment.NewLine
                       + "1,  Painting,  'Any type of painting' " + Environment.NewLine
                       + "2,  Sports,    'Sports classes'" + Environment.NewLine
                       + "3,  Boxing,	 'Boxing classes'";
            CsvDoc doc = new CsvDoc(csv, false);

            Logger.Info("====================================================");
            Logger.Info("CSV FILES ");
            Logger.Info("Columns : " + doc.Columns.ToString());
            Logger.Info("Row 1 - Col 1 : " + doc.Data[0][0]);
            Logger.Info("Row 2 - Col 2: " + doc.Data[1][1]);
            Logger.Info("Row 3['Name'] : " + doc.Data[2]["name"].ToString());
            Logger.Info(Environment.NewLine);
        }


        /// <summary>
        /// Substitutions
        /// </summary>
        public void DoSubstitutions()
        {
            Logger.Info("====================================================");
            Logger.Info("SUBSTITUTIONS ");
            Logger.Info("Evaluating ${today} to : " + Substitutor.Substitute("${today}"));
            Logger.Info("Evaluating ${t-1}   to : " + Substitutor.Substitute("${t-1}"));
            Logger.Info("Evaluating ${t+1}   to : " + Substitutor.Substitute("${t+1}"));
            Logger.Info("Evaluating ${username}   to : " + Substitutor.Substitute("${username}"));
            Logger.Info(Environment.NewLine);
        }


        /// <summary>
        /// Substitutions
        /// </summary>
        public void DoImportExport()
        {
            Logger.Info("====================================================");
            Logger.Info("IMPORT EXPORT VIA INI ");
            Logger.Info(Environment.NewLine);
        }


        /// <summary>
        /// Diagnostics
        /// </summary>
        public void DoDiagnostics()
        {
            string info = Diagnostics.Diagnostics.GetInfo("MachineInfo,Drives,Env_System,Env_User,AppDomain");
            File.WriteAllText("CommonLibrary_Diagnostics.txt", info);
        }


        /// <summary>
        /// Notifications.
        /// </summary>
        public void DoNotifications()
        {
            // Configure the notification service.
            // Since emailing is disabled, the EmailServiceSettings are not configured.
            // The emails are not sent as "EnableNotifications" = false above.
            // Debugging is turned on so you can see the emails in the folder "Notifications.Tests".
            Notifier.Init(new EmailService(new EmailServiceSettings()), new NotificationSettings());
            Notifier.Settings["website.name"] = "KnowledgeDrink.com";
            Notifier.Settings["website.url"] = "http://www.KnowledgeDrink.com";
            Notifier.Settings["website.urls.contactus"] = "http://www.KnowledgeDrink.com/contactus.aspx";
            Notifier.Settings.EnableNotifications = false;
            Notifier.Settings.DebugOutputMessageToFile = true;
            Notifier.Settings.DebugOutputMessageFolderPath = @"Notifications.Tests";

            Logger.Info("====================================================");
            Logger.Info("NOTIFICATION EMAILS ");
            Logger.Info("Emails are generated to folder : " + Notifier.Settings.DebugOutputMessageFolderPath);
            Notifier.WelcomeNewUser("user1@mydomain.com", "Welcome to www.knowledgedrink.com", "batman", "user1", "password");
            Notifier.RemindUserPassword("user1@mydomain.com", "Welcome to www.knowledgedrink.com", "batman", "user1", "password");
            Notifier.SendToFriend("batman@mydomain.com", "Check out www.knowledgedrink.com", "superman", "bruce", "Learn to fight.");
            Notifier.SendToFriendPost("superman@mydomain.com", "Check out class at www.knowledgedrink.com", "batman", "clark", "Punk, learn to fly.",
                "Learn to fly", "http://www.knowledgedrink.com/classes/learn-to-fly.aspx");
            Notifier.Process();

            Logger.Info(Environment.NewLine);
        }


        /// <summary>
        /// Show how to use activerecord.
        /// </summary>
        public void DoDatabaseQuery(bool isDatabaseSetup)
        {
            if (!isDatabaseSetup)
                return;

            ConnectionInfo con = new ConnectionInfo("Server=server1;Database=db1;User=user1;Password=password;", "System.Data.SqlClient");
            DBHelper db = new DBHelper(con);

            // Show examples of each type of query.
            // 1. ExecuteTable    =  query table.
            // 2. ExecuteScalar   =  get total post count
            // 3. ExecuteReader   =  get reader
            // 4. ExecuteNonQuery =  update table.
            // 5. Execute         =  call stored procedure with 1 arg
            // 6. Execute         =  call stored procedure with 2 args.
            DataTable table = db.ExecuteDataTable("select * from posts", CommandType.Text, null);
            int totalPosts = Convert.ToInt32(db.ExecuteScalar("select count(*) from posts", CommandType.Text, null));
            IDataReader reader = db.ExecuteReader("select * from posts", CommandType.Text, null); reader.Close();
            db.ExecuteNonQuery("update users set name = 'commonlibrary' where id = 1", CommandType.Text, null);
            db.Execute("post_activate", CommandType.StoredProcedure, "id", DbType.String, 2);

            // Stored proc with multiple params.
            DbParameter[] paramList = new DbParameter[2];
            paramList[0] = db.BuildInParam("id", DbType.Int32, 1);
            paramList[1] = db.BuildInParam("status", DbType.Int32, 3);
            db.Execute("post_updatestatus", CommandType.StoredProcedure, paramList, null);
        }


        /// <summary>
        /// Iniitalize ioc containter.
        /// </summary>
        public void DoIocContainer()
        {
            IocContainerInMemory ioc = new IocContainerInMemory();
            Ioc.Init(ioc);
        }


        public void DoCodeGenerator()
        {
            SampleCodeGenerator codeGenSampler = new SampleCodeGenerator();
            // The directories in the code generator must exit.
            // The beta version of this will properly create the directories.
            // ModelCodeLocation = @"C:\dev\MyApp\src\Lib\CommonLibrary.WebModules\Src\Generated",
            // ModelCodeLocationTemplate = @"C:\dev\MyApp\src\Lib\GenericCode\CodeGen\Templates\Default",
            // ModelInstallLocation = @"C:\dev\MyApp\install\",
            // ModelCodeLocationUI = @"C:\dev\MyApp\models\ui\",
            // ModelDbStoredProcTemplates = @"C:\dev\MyApp\src\Lib\GenericCode\CodeGen\Templates\DefaultSql",
            // ModelOrmLocation = @"C:\dev\MyApp\src\Lib\CommonLibrary.WebModules\Config\hbm.xml",

            // codeGenSampler.CanCreate();
        }


        
        /// <summary>
        /// Diagnostics
        /// </summary>
        public void DoActiveRecord(bool isDatabaseSetup)
        {
            /*
            if (!isDatabaseSetup)
                return;

            //Config.InitActiveRecord();

            // This shows how NamedQuery extends the ActiveRecordBase object.
            NamedQuery query = new NamedQuery();
            query.Name = "Top 20 recent posts";
            query.Sql = "select top 10 * from wk_workshops order by id desc";
            query.Description = "Get the 20 most recent posts";
            
            // Save
            NamedQuery.Save(query);
            
            // Get
            NamedQuery fromdb = NamedQuery.Get(query.Id).Item;

            // Get all
            IList<NamedQuery> allNamedQueries = NamedQuery.GetAll().Item;

            // Delete
            NamedQuery.Delete(query.Id);
            */
        }
    }


    public class AppConfig
    {
        /// <summary>
        /// This builds a datastructure of all the environments supported
        /// and the links to the config files for each environment 
        /// and how they are inherited.
        /// 
        /// THIS CAN BE LOADED FROM AN XML, JSON, YAML, INI file or whatever.
        /// </summary>
        /// <returns></returns>
        public static List<EnvItem> GetSampleEvironments()
        {
            List<EnvItem> envs = new List<EnvItem>()
            {
                new EnvItem(){ Name = "Dev",    RefPath =@"${rootfolder}\dev.config",    InheritsDeeply = true,  EnvType = EnvType.Dev,       Inherits = "" },
                new EnvItem(){ Name = "Dev2",   RefPath =@"${rootfolder}\dev2.config",   InheritsDeeply = true,  EnvType = EnvType.Dev,       Inherits = "" },
                new EnvItem(){ Name = "Qa",     RefPath =@"${rootfolder}\qa.config",     InheritsDeeply = true,  EnvType = EnvType.Qa,        Inherits = "Dev" },
                new EnvItem(){ Name = "Prod",   RefPath =@"${rootfolder}\prod.config",   InheritsDeeply = true,  EnvType = EnvType.Prod,      Inherits = "Qa" },
                new EnvItem(){ Name = "Custom", RefPath =@"${rootfolder}\custom.config", InheritsDeeply = true,  EnvType = EnvType.MixedProd, Inherits = "Prod,Dev2" }
            };
            return envs;
        }


        /// <summary>
        /// Get sample united states holidays.
        /// 
        /// This can be loaded from an XML file.
        /// </summary>
        /// <returns></returns>
        public static List<Holiday> GetSampleUnitedStatesHolidays()
        {
            // For testing, New Years, July 4th, Christmas.
            // This can be loaded from the database.
            List<Holiday> holidays = new List<Holiday>()
            {
                new Holiday(1, 1, true, DayOfWeek.Monday, -1, "New Years"),
                new Holiday(1, 19, true, DayOfWeek.Monday, -1, "Martin Luther King"),
                new Holiday(2, 16, true, DayOfWeek.Monday, -1, "Washingtons Birthday"),
                new Holiday(5, 25, true, DayOfWeek.Monday, -1, "Memorial Day"),
                new Holiday(7, 4, true, DayOfWeek.Monday, -1, "Independence Day"),
                new Holiday(9, 7, true, DayOfWeek.Monday, -1, "Labor Day"),
                new Holiday(10, 12, true, DayOfWeek.Monday, -1, "Columbus Day"),
                new Holiday(11, 11, true, DayOfWeek.Monday, -1, "Veterans Day"),
                new Holiday(11, 26, true, DayOfWeek.Monday, -1, "Thanks Giving"),
                new Holiday(12, 25, true, DayOfWeek.Monday, -1, "Christmas Day")
            };
            return holidays;
        }


        public static void InitActiveRecord()
        {
            // Register the active record.
            // This is done by 
            // 1. giving a name "namedQuery" for each entitytype.
            // 2. specifying the type of ActionContext that the entity service takes "ActionContext"
            // 3. Adding the entity service to the IocContainer.
            // 4. By default, the active record system looks for a entity service
            //    with the suffix "Service" at the entity name "namedQuery".
            // EntityRegistration.Register(new ActiveRecordRegistrationContext("NamedQuery", typeof(NamedQuery), typeof(ActionContext)));
            //NamedQueryService service = new NamedQueryService(new EntityRepositoryInMemory<NamedQuery>(), new NamedQueryValidator(), new NamedQuerySettings());
            //IocContainer.AddObject("NamedQueryService", service);
        }
    }


    /// <summary>
    /// This class is used recieve the arguments supplied on the command line.
    /// </summary>
    public class SampleAppsArguments
    {
        [Arg("env", "Environment(DEV,QA,PROD)", typeof(string), true, "Dev", "Dev|Qa|Prod")]
        public string Environment { get; set; }


        [Arg("date", "The business date", typeof(DateTime), true, "${today}", "yyyy-mm-dd|${today}")]
        public DateTime BusinessDate { get; set; }


        [Arg("readonly", "Run env in readonly mode", typeof(bool), false, false, "true|false")]
        public bool ReadOnlyMode { get; set; }


        [Arg(1, "Number of categories to display", typeof(int), false, 5, "5")]
        public int CategoriesToDisplay { get; set; }


        [Arg(0, "settings id to load on startup", typeof(string), true, "", "common_user_settings01")]
        public string DefaultSettingsId { get; set; }
    }



    /// <summary>
    /// Custom excepton handler.
    /// </summary>
    public class CustomExceptionHandler : ErrorManagerBase, IErrorManager
    {
        /// <summary>
        /// Custom exception handler.
        /// </summary>
        /// <param name="name"></param>
        public CustomExceptionHandler(string name)
        {
            _name = name;
        }


        protected override void InternalHandle(object error, Exception exception, IStatusResults errorResults, object[] arguments)
        {
            Logger.Info("This is from the custom exception handler");
            base.InternalHandle(error, exception, errorResults, arguments);
        }
    }



    /// <summary>
    /// Code generator sample code.
    /// </summary>
    public class SampleCodeGenerator
    {
        /// <summary>
        /// Test creation of models
        /// </summary>
        public void CanCreate()
        {
            ModelContainer models = new ModelContainer()
            {    
                Settings = new ModelBuilderSettings()
                {
                    ModelCodeLocation = @"C:\dev\MyApp\src\Lib\CommonLibrary.WebModules\Src\Generated",
                    ModelCodeLocationTemplate = @"C:\dev\MyApp\src\Lib\GenericCode\CodeGen\Templates\Default",
                    ModelInstallLocation = @"C:\dev\MyApp\install\",
                    ModelCodeLocationUI = @"C:\dev\MyApp\models\ui\",
                    ModelDbStoredProcTemplates = @"C:\dev\MyApp\src\Lib\GenericCode\CodeGen\Templates\DefaultSql",
                    ModelOrmLocation = @"C:\dev\MyApp\src\Lib\CommonLibrary.WebModules\Config\hbm.xml",
                    DbAction_Create = DbCreateType.DropCreate,
                    Connection = new ConnectionInfo("Server=server1;Database=db1;User=user1;Password=password;", "System.Data.SqlClient"),
                    AssemblyName = "CommonLibrary.WebModules",
                    OrmGenerationDef = new OrmGeneration(true, "<!-- ORM_GEN_START -->", "<!-- ORM_GEN_END -->"),
                },

                ExtendedSettings = new Dictionary<string, object>() { },
                
                // Model definition.
                AllModels = new List<Model>()
                {
                    new Model("ModelBase")
                    {
                        NameSpace = "CommonLibrary.WebModules",
                        GenerateCode = false,
                        GenerateTable = false,
                        GenerateOrMap = false,
                        PropertiesSortOrder = 1,
                        Properties = new List<PropertyInfo>()
                        {
                            new PropertyInfo( "Id",            typeof(int)      ) { IsRequired = true, ColumnName = "Id", IsKey = true },
                            new PropertyInfo( "CreateDate",    typeof(DateTime) ) { IsRequired = true },
                            new PropertyInfo( "UpdateDate",    typeof(DateTime) ) { IsRequired = true },
                            new PropertyInfo( "CreateUser",    typeof(string)   ) { IsRequired = true, MaxLength = "20" },
                            new PropertyInfo( "UpdateUser",    typeof(string)   ) { IsRequired = true, MaxLength = "20" },
                            new PropertyInfo( "UpdateComment", typeof(string)   ) { IsRequired = false, MaxLength = "150" },
                            new PropertyInfo( "Version",       typeof(int)      ) { IsRequired = true, DefaultValue = 1 },
                            new PropertyInfo( "IsActive",      typeof(bool)     ) { IsRequired = true, DefaultValue = 0 }
                        }
                    },
                    new Model("RatingPostBase")
                    {
                        NameSpace = "CommonLibrary.WebModules",
                        Inherits = "ModelBase",
                        GenerateCode = false,
                        GenerateTable = false,
                        GenerateOrMap = false,
                        PropertiesSortOrder = 100,
                        Properties = new List<PropertyInfo>()
                        {
                            new PropertyInfo( "AverageRating",     typeof(double) ),
                            new PropertyInfo( "TotalLiked",        typeof(int)    ),
                            new PropertyInfo( "TotalDisLiked",     typeof(int)    ),
                            new PropertyInfo( "TotalBookMarked",   typeof(int)    ),
                            new PropertyInfo( "TotalAbuseReports", typeof(int)    )
                        }
                    },
                    new Model("Address")
                    {
                        Properties = new List<PropertyInfo>()
                        {
                            new PropertyInfo( "Street",    typeof(string)) { MaxLength = "40" },
                            new PropertyInfo( "City",      typeof(string)) { MaxLength = "40" },
                            new PropertyInfo( "State",     typeof(string)) { MaxLength = "20" },
                            new PropertyInfo( "Country",   typeof(string)) { MaxLength = "20", DefaultValue = "U.S." },
                            new PropertyInfo( "Zip",       typeof(string)) { MaxLength = "10" },
                            new PropertyInfo( "CityId",    typeof(int) ),
                            new PropertyInfo( "StateId",   typeof(int) ),
                            new PropertyInfo( "CountryId", typeof(int) )                            
                        }
                    },
                    new Model("BlogPost")
                    {
                        TableName = "BlogPosts",
                        NameSpace = "CommonLibrary.WebModules.BlogPosts",
                        GenerateTable = true, GenerateCode = true, GenerateTests = true, 
                        GenerateUI = true, GenerateRestApi = true, GenerateFeeds = true, GenerateOrMap = true,
                        IsWebUI = true,
                        PropertiesSortOrder = 50,
                        Properties = new List<PropertyInfo>()
                        {
                            new PropertyInfo("Title",         typeof(string)) { ColumnName = "Title", MinLength = "10", MaxLength = "150", IsRequired = true },
                            new PropertyInfo("Summary",       typeof(string)) { MaxLength = "200", IsRequired = true },
                            new PropertyInfo("Description",   typeof(StringClob)) { MinLength = "10", MaxLength = "-1", IsRequired = true },
                            new PropertyInfo("Url",           typeof(string)) { MaxLength = "150", RegEx = "" },
                            new PropertyInfo("AllowComments", typeof(bool)) { DefaultValue = true }                            
                        },
                        Inherits = "ModelBase",
                        Includes = new List<Include>() { new Include("RatingPostBase") },
                        HasMany = new List<Relation>() { new Relation( "Comment" ) },
                        ExcludeFiles = "Feeds.cs,ImportExport.cs,Serializer.cs"
                    },                
                    new Model("Event")
                    {
                        TableName = "Events",
                        NameSpace = "CommonLibrary.WebModules.Events",
                        GenerateTable = true, GenerateCode = true, GenerateTests = true, 
                        GenerateUI = true, GenerateRestApi = true, GenerateFeeds = true, GenerateOrMap = true,
                        IsWebUI = true,
                        PropertiesSortOrder = 50,
                        Properties = new List<PropertyInfo>()
                        {
                            new PropertyInfo("Title",       typeof(string))     { ColumnName = "Title", MinLength = "10", MaxLength = "150", IsRequired = true },
                            new PropertyInfo("Summary",     typeof(string))     { MaxLength = "200", IsRequired = true },
                            new PropertyInfo("Description", typeof(StringClob)) { MinLength = "10", MaxLength = "-1", IsRequired = true },
                            new PropertyInfo("StartDate",   typeof(DateTime))   { IsRequired = true },
                            new PropertyInfo("EndDate",     typeof(DateTime))   { IsRequired = true},
                            new PropertyInfo("StartTime",   typeof(DateTime)),
                            new PropertyInfo("EndTime",     typeof(DateTime)),
                            new PropertyInfo("IsOnline",    typeof(bool))       { DefaultValue = false },
                            new PropertyInfo("Email",       typeof(string))     { IsRequired = false, MaxLength = "30",  RegEx = "RegexPatterns.Email", IsRegExConst = true },
                            new PropertyInfo("Phone",       typeof(string))     { IsRequired = false, MaxLength = "20",  RegEx = "RegexPatterns.PhoneUS", IsRegExConst = true },
                            new PropertyInfo("Url",         typeof(string))     { IsRequired = false, MaxLength = "150", RegEx = "RegexPatterns.Url", IsRegExConst = true },
                            new PropertyInfo("Keywords",    typeof(string))     { MaxLength = "100"}
                        },
                        Inherits = "ModelBase",
                        Includes     = new List<Include>()          { new Include("RatingPostBase") },
                        ComposedOf   = new List<Composition>()     { new Composition("Address") },
                        Validations  = new List<ValidationItem>()  { new ValidationItem("Address", typeof(LocationValidator)) { IsStatic = false} },
                        DataMassages = new List<DataMassageItem>() { new DataMassageItem("Address", typeof(LocationDataMassager), Massage.AfterValidation) { IsStatic = true} },
                        ExcludeFiles = "Feeds.cs,ImportExport.cs,Serializer.cs"
                    },                    
                    new Model("Job")
                    {
                        TableName = "Jobs",
                        NameSpace = "CommonLibrary.WebModules.Jobs",
                        GenerateTable = true, GenerateCode = true, GenerateTests = true, 
                        GenerateUI = true, GenerateRestApi = true, GenerateFeeds = true, GenerateOrMap = true,
                        IsWebUI = true,
                        PropertiesSortOrder = 50,
                        Properties = new List<PropertyInfo>()
                        {
                            new PropertyInfo("Title",       typeof(string))     { ColumnName = "Title", MinLength = "10", MaxLength = "150", IsRequired = true },
                            new PropertyInfo("Summary",     typeof(string))     { MaxLength = "200", IsRequired = true },
                            new PropertyInfo("Description", typeof(StringClob)) { MinLength = "10", MaxLength = "-1", IsRequired = true },
                            new PropertyInfo("StartDate",   typeof(DateTime))   { IsRequired = true },
                            new PropertyInfo("EndDate",     typeof(DateTime))   { IsRequired = true},
                            new PropertyInfo("StartTime",   typeof(DateTime)),
                            new PropertyInfo("EndTime",     typeof(DateTime)),
                            new PropertyInfo("IsOnline",    typeof(bool))       { DefaultValue = false },
                            new PropertyInfo("Email",       typeof(string))     { IsRequired = false, MaxLength = "30",  RegEx = "RegexPatterns.Email", IsRegExConst = true },
                            new PropertyInfo("Phone",       typeof(string))     { IsRequired = false, MaxLength = "20",  RegEx = "RegexPatterns.PhoneUS", IsRegExConst = true },
                            new PropertyInfo("Url",         typeof(string))     { IsRequired = false, MaxLength = "150", RegEx = "RegexPatterns.Url", IsRegExConst = true },
                            new PropertyInfo("Keywords",    typeof(string))     { MaxLength = "100"}
                        },
                        Inherits = "ModelBase",
                        Includes     = new List<Include>()          { new Include("RatingPostBase") },
                        ComposedOf   = new List<Composition>()     { new Composition("Address") },
                        Validations  = new List<ValidationItem>()  { new ValidationItem("Address", typeof(LocationValidator)) { IsStatic = false} },
                        DataMassages = new List<DataMassageItem>() { new DataMassageItem("Address", typeof(LocationDataMassager), Massage.AfterValidation) { IsStatic = true} },
                        ExcludeFiles = "Feeds.cs,ImportExport.cs,Serializer.cs"
                    }
                }
            };

            ModelContext ctx = new ModelContext() { AllModels = models };
            IList<ICodeBuilder> builders = new List<ICodeBuilder>()
            {
                new CodeBuilderWebUI(),
                new CodeBuilderDb(ctx.AllModels.Settings.Connection),
                new CodeBuilderORMHibernate(),
                new CodeBuilderDomain()
            };
            BoolMessage message = CodeBuilder.Process(ctx, builders);
        }
    }
}
