using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib.Models
{
    public class PropInfo
    {
        public PropInfo()
        {         
        }

        
        /// <summary>
        /// Initialize using name of property and it's datatype.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="dataType"></param>
        public PropInfo(string name, Type dataType)
        {
            Name = name;
            DataType = dataType;
        }


        /// <summary>
        /// Name of the property
        /// </summary>
        public string Name { get; set; }


        /// <summary>
        /// DataType of the property.
        /// </summary>
        public Type DataType { get; set; }    


        /// <summary>
        /// Default value for this property.
        /// </summary>
        public object DefaultValue { get; set; }


        /// <summary>
        /// Regular expression to use for validations.
        /// </summary>
        public string RegEx { get; set; }


        /// <summary>
        /// Is Primary key.
        /// </summary>
        public bool IsKey { get; set; }


        /// <summary>
        /// If this is a required / not-null property.
        /// </summary>
        public bool IsRequired { get; set; }


        /// <summary>
        /// Indicate if this property is only a getter, no setter.
        /// </summary>
        public bool IsGetterOnly { get; set; }


        /// <summary>
        /// Indicate if min length should be checked.
        /// </summary>
        public bool CheckMinLength { get; set; }


        /// <summary>
        /// Indicate if max length should be checked.
        /// </summary>
        public bool CheckMaxLength { get; set; }


        private string _minLength = string.Empty;
        /// <summary>
        /// Maximum length for this property if string.
        /// </summary>
        public string MinLength
        {
            get { return _minLength; }
            set
            {
                if (DataType == typeof(string) && !string.IsNullOrEmpty(value))
                {
                    if (value == "-1")
                    {
                        CheckMinLength = false;
                        IsRequired = false;
                    }
                    else
                    {
                        CheckMinLength = true;
                        _minLength = value;
                        if (_minLength != "0")
                            IsRequired = true;
                    }
                }
            }
        }


        private string _maxLength = string.Empty;
        /// <summary>
        /// Maximum length for this property if string.
        /// </summary>
        public string MaxLength 
        {
            get { return _maxLength; }
            set
            {
                if (DataType == typeof(string) && !string.IsNullOrEmpty(value))
                {
                    if (value == "-1")
                        CheckMaxLength = false;
                    else
                    {
                        CheckMaxLength = true;
                        _maxLength = value;
                    }
                }
            }
        }


        /// <summary>
        /// Is unique.
        /// </summary>
        public bool IsUnique { get; set; }


        private string _columnName;
        /// <summary>
        /// Name if empty.
        /// </summary>
        public string ColumnName
        {
            get
            {
                if (string.IsNullOrEmpty(_columnName))
                    return Name;
                return _columnName;
            }
            set { _columnName = value; }
        }


        private bool _createCode = true;
        /// <summary>
        /// Whether or not to create this property in code.
        /// </summary>
        public bool CreateCode
        {
            get { return _createCode; }
            set { _createCode = value; }
        }


        private bool _createColumn = true;
        /// <summary>
        /// Whether or not to create this property in code.
        /// </summary>
        public bool CreateColumn
        {
            get { return _createColumn; }
            set { _createColumn = value; }
        }


        /// <summary>
        /// Indicates if the regular expression supplied is a constant.
        /// </summary>
        public bool IsRegExConst { get; set; }
    }



    /// <summary>
    /// Relationship to model.
    /// </summary>
    public class Relation
    {
        /// <summary>
        /// Initialize with the model name.
        /// </summary>
        /// <param name="modelName"></param>
        public Relation(string modelName)
        {
            ModelName = modelName;
        }


        public string ModelName { get; set; }

        public string ForeignKey { get; set; }

        public string Key { get; set; }
    }



    public class UISpec
    {
        public bool CreateEditUI { get; set; }
        public bool SummaryUI { get; set; }
        public bool DetailsUI { get; set; }
        public string PropertyName { get; set; }


        public UISpec(string propertyName, bool createEditUI, bool summaryUI, bool detailsUI)
        {
            PropertyName = propertyName;
            CreateEditUI = createEditUI;
            SummaryUI = summaryUI;
            DetailsUI = detailsUI;
        }
    }
}
