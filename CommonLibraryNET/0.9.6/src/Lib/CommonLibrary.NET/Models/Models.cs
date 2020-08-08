using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ComLib.Data;


namespace ComLib.Models
{
    public struct StringClob
    {
        public override string ToString()
        {
            return "StringClob";
        }
    }


    public struct Image
    {        
    }



    public class CustomType
    {
        public CustomType(string name)
        {
            Name = name;
        }

        public string Name { get; set; }

        public override string ToString()
        {
            return Name;
        }

    }



    public enum DbCreateType
    {
        DropCreate,
        Create,
        Update
    }



    /// <summary>
    /// DomainModel representing class/table mappings.
    /// </summary>
    public class Model
    {
        private Relation _lastRelation;

        public Model() : this(string.Empty)
        {
        }


        public Model(string name)
        {
            Name = name;
            this.Properties = new List<PropInfo>();
            this.Includes = new List<Include>();
            this.ComposedOf = new List<Composition>();
            this.OneToMany = new List<Relation>();
            this.OneToOne = new List<Relation>();
        }


        public string Name { get; set; }
        public string TableName { get; set; }
        public string NameSpace { get; set; }
        public string Inherits { get; set; }
        public bool GenerateTable { get; set; }
        public bool GenerateOrMap { get; set; } 
        public bool GenerateCode { get; set; }
        public bool GenerateValidation { get; set; }
        public bool GenerateTests { get; set; }
        public bool GenerateUI { get; set; }
        public bool GenerateRestApi { get; set; }
        public bool GenerateFeeds { get; set; }
        public bool IsWebUI { get; set; }
        public List<PropInfo> Properties { get; set; }
        public int PropertiesSortOrder { get; set; }
        public string ExcludeFiles { get; set; }
        public bool IsComposite { get; set; }
        public string InstallSqlFile { get; set; }


        /// <summary>
        /// Get /set the repository type.
        /// </summary>
        public string RepositoryType { get; set; }


        /// <summary>
        /// List of names of model whose properties to include.
        /// </summary>
        public List<Include> Includes { get; set; }


        /// <summary>
        /// List of objects that compose this model.
        /// </summary>
        public List<Composition> ComposedOf { get; set; }


        public List<UISpec> UI { get; set; }


        /// <summary>
        /// One-to-many relationships.
        /// </summary>
        public List<Relation> OneToMany { get; set; }


        /// <summary>
        /// One-to-many relationships.
        /// </summary>
        public List<Relation> OneToOne { get; set; }


        /// <summary>
        /// Validations to perform on entity.
        /// </summary>
        public List<ValidationItem> Validations { get; set; }


        /// <summary>
        /// List of data massage items to apply.
        /// </summary>
        ///public List<DataMassageItem> DataMassages { get; set; }


        /// <summary>
        /// List of roles that can manage ( delete ) instances of this model.
        /// </summary>
        /// <example>ManagedBy = new List(string){ "Owner", "Moderator", "Admin" }; </example>
        public List<string> ManagedBy { get; set; }


        /// <summary>
        /// List of properties that can be used to lookup up an entity.
        /// e.g. These should typically be the Id ( integer ) and "Name" ( string )
        /// </summary>
        /// <example>LookupOn = new List(string){ "Id", "Name" };</example>
        public List<string> LookupOn { get; set; }


        /// <summary>
        /// Assembly name.
        /// </summary>
        public string AssemblyName { get; set; }


        /// <summary>
        /// Additional settings to make it easy to add new settings dynamically.
        /// Also allows for inheritance.
        /// </summary>
        public Dictionary<string, string> Settings { get; set; }


        #region Fluent-Based
        public Model AddProperty<T>(string name)
        {
            PropInfo p = new PropInfo();
            p.DataType = typeof(T);
            p.Name = name;
            p.CreateCode = true;
            p.CreateColumn = true;
            this.Properties.Add(p);
            return this;
        }


        public Model Required
        {
            get { this.Properties[this.Properties.Count - 1].IsRequired = true; return this; }
        }


        public Model Key
        {
            get { this.Properties[this.Properties.Count - 1].IsKey = true; return this; }
        }


        public Model MaxLength(string max)
        {
            this.Properties[this.Properties.Count - 1].MaxLength = max;
            return this;
        }


        public Model DefaultTo(object val)
        {
            this.Properties[this.Properties.Count - 1].DefaultValue = val;
            return this;
        }


        public Model Persist
        {
            get { this.Properties[this.Properties.Count - 1].CreateColumn = true; return this; }
        }


        public Model Code
        {
            get { this.Properties[this.Properties.Count - 1].CreateCode = true; return this; }
        }


        public Model NotPersisted
        {
            get { this.Properties[this.Properties.Count - 1].CreateColumn = false; return this; }
        }


        public Model NoCode
        {
            get { this.Properties[this.Properties.Count - 1].CreateCode = false; return this; }
        }


        public Model GetterOnly
        {
            get { this.Properties[this.Properties.Count - 1].IsGetterOnly = true; return this; }
        }


        public Model Range(string min, string max)
        {
            this.Properties[this.Properties.Count - 1].MinLength = min;
            this.Properties[this.Properties.Count - 1].MaxLength = max;
            return this;          
        }


        public Model BuildCode()
        {
            this.GenerateCode = true;
            this.GenerateValidation = true;
            return this;
        }


        
        public Model NoValidation()
        {
            this.GenerateValidation = false;
            return this;
        }


        public Model BuildTable(string tableName)
        {
            this.TableName = tableName;
            this.GenerateTable = true;
            return this;
        }


        public Model BuildInstallSqlFile()
        {
            this.InstallSqlFile = string.Format("{0}.sql", this.Name);
            return this;
        }


        public Model NameSpaceIs(string nameSpace)
        {
            this.NameSpace = nameSpace;
            return this;
        }


        public Model InheritsFrom(string modelName)
        {
            this.Inherits = modelName;
            return this;
        }


        public Model BuildActiveRecordEntity()
        {
            this.ExcludeFiles = "ActiveRecord.cs,Service.cs,Validator.cs,Repository.cs,Settings.cs,Feeds.cs,ImportExport.cs,Serializer.cs";
            this.RepositoryType = "RepositorySql";
            return this;
        }


        public Model RegExConst(string regExPattern)
        {
            this.Properties[this.Properties.Count - 1].RegEx = regExPattern;
            this.Properties[this.Properties.Count - 1].IsRegExConst = true;
            return this;
        }


        public Model RegEx(string regExPattern)
        {
            this.Properties[this.Properties.Count-1].RegEx = regExPattern;
            return this;
        }


        public Model HasComposition(string modelName)
        {
            this.ComposedOf.Add(new Composition(modelName));
            return this;
        }


        public Model HasInclude(string modelName)
        {
            this.Includes.Add(new Include(modelName));
            return this;
        }


        public Model HasOne(string modelName)
        {
            Relation rel = new Relation(modelName);
            this.OneToOne.Add(rel);
            _lastRelation = rel;
            return this;
        }


        public Model HasMany(string modelName)
        {
            var rel = new Relation(modelName);
            this.OneToMany.Add(rel);
            _lastRelation = rel;
            return this;
        }


        public Model OnKey(string key)
        {
            _lastRelation.Key = key;
            return this;
        }


        public Model OnForeignKey(string key)
        {
            _lastRelation.ForeignKey = key;
            return this;
        }


        public Model Mod { get { return this; } }
        #endregion
    }



    /// <summary>
    /// Composition information.
    /// </summary>
    public class Include
    {
        public Include(string refModel)
        {
            Name = refModel;
            GenerateOrMap = true;
            GenerateCode = true;
            GenerateUI = true;
        }


        /// <summary>
        /// Name of the model that in the <see cref="ModelContainer"/> that
        /// represents this composition.
        /// </summary>
        public string Name { get; set; }
        public bool GenerateOrMap { get; set; }
        public bool GenerateCode { get; set; }
        public bool GenerateUI { get; set; }
        public Model ModelRef { get; set; }
    }



    /// <summary>
    /// Composition information.
    /// </summary>
    public class Composition : Include
    {
        public Composition(string refModel) : base(refModel) { }
    }


    /// <summary>
    /// Validation definition for a specific property.
    /// </summary>
    public class ValidationItem
    {
        /// <summary>
        /// Initialize the validator the property.
        /// </summary>
        /// <param name="property"></param>
        /// <param name="validator"></param>
        public ValidationItem(string property, Type validator)
        {
            PropertyToValidate = property;
            PropertyValidator = validator;
        }


        /// <summary>
        /// The name of the property on the entity to validate.
        /// </summary>
        public string PropertyToValidate { get; set; }


        /// <summary>
        /// The datatype of the validator to use for validating this property.
        /// </summary>
        public Type PropertyValidator { get; set; }


        /// <summary>
        /// Whether or not the validator is instance based or can be statically called.
        /// </summary>
        public bool IsStatic { get; set; }
    }
}
