using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

using ComLib;


namespace ComLib.Data
{
    public interface ICriteria<T> : ICriteria
    {
        ICriteria<T> Select(params Expression<Func<T, object>>[] colExpressions);
        ICriteria<T> Where(Expression<Func<T, object>> exp);
        ICriteria<T> And(Expression<Func<T, object>> exp);
        ICriteria<T> Or(Expression<Func<T, object>> exp);
        ICriteria<T> OrderBy(Expression<Func<T, object>> exp); 

        ICriteria<T> Select(params string[] cols);
        ICriteria<T> Select(string field);
        ICriteria<T> As(string alias);
        ICriteria<T> From(string tableName);
        ICriteria<T> Where(string field);
        ICriteria<T> And(string field);
        ICriteria<T> Or(string field);
        ICriteria<T> In<TParam>(params object[] vals);
        ICriteria<T> NotIn<TParam>(params object[] vals);
        ICriteria<T> Is(object val);
        ICriteria<T> Not(object val);
        ICriteria<T> Null();
        ICriteria<T> NotNull();
        ICriteria<T> LessThan(object val);
        ICriteria<T> LessEqual(object val);
        ICriteria<T> Like(string val);
        ICriteria<T> Like(string val, bool addWildcardPrefix, bool addWildcardSuffix);
        ICriteria<T> MoreThan(object val);
        ICriteria<T> MoreEqual(object val);
        ICriteria<T> OrderBy(string field);
        ICriteria<T> Asc { get; }
        ICriteria<T> Desc { get; }
        ICriteria<T> Limit(int maxRecords);
        ICriteria<T> End();
    }



    public interface ICriteria
    {
        int GetLimit();
        bool HasLimit { get; }
        void SetFrom(string from);

        string Sql();
        string Sql(string from);
        string SqlSelect();
        string SqlSelect(bool includeSelect);
        string SqlLimit();
        string SqlConditions();
        string SqlConditions(bool includeWhere);
        string SqlOrderBy();
        string SqlOrderBy(bool includeOrderBy);
    }



    public class CriteriaData
    {
        public List<Condition> Conditions;
        public List<OrderByClause> _orderings;
        public OrderByClause LastOrderBy;
        public List<SelectField> SelectFields;
        public string From;
        public int RecordLimit;
        public bool IsRecordLimitEnabled;
    }



    public class Criteria<T> : ICriteria<T>
    {
        public Condition _lastCondition;
        protected bool _surroundFields;
        protected string _surroundFieldLeftChar = "[";
        protected string _surroundFieldRightChar = "]";


        public static ICriteria<T> New()
        {
            return New(string.Empty);
        }


        // Static factory
        public static ICriteria<T> New(string dbProviderName)
        {
            if (string.IsNullOrEmpty(dbProviderName)) return new Criteria<T>();
            
            // No support for other databases yet.
            // Not sure it's necessay but just in case.
            return new Criteria<T>();
        }


        public Criteria()
        {            
            _conditions = new List<Condition>();
            _orderings = new List<OrderByClause>();
        }


        public void SetFrom(string fromtable)
        {
            _from = fromtable;
        }


        public ICriteria<T> Select(params Expression<Func<T, object>>[] colExpressions)
        {
            if (colExpressions == null || colExpressions.Length == 0)
                return this;

            string[] cols = new string[colExpressions.Length];
            for (int ndx = 0; ndx < colExpressions.Length; ndx++)
                cols[ndx] = ExpressionHelper.GetPropertyName<T>(colExpressions[ndx]);

            return Select(cols);
        }


        public ICriteria<T> Where(Expression<Func<T, object>> exp)
        {
            string field = ExpressionHelper.GetPropertyName<T>(exp);
            return StartNewCondition(ConditionType.None, field);
        }


        public ICriteria<T> And(Expression<Func<T, object>> exp)
        {
            string field = ExpressionHelper.GetPropertyName<T>(exp);
            return StartNewCondition(ConditionType.And, field);
        }


        public ICriteria<T> Or(Expression<Func<T, object>> exp)
        {
            string field = ExpressionHelper.GetPropertyName<T>(exp);
            return StartNewCondition(ConditionType.Or, field);
        }


        public ICriteria<T> OrderBy(Expression<Func<T, object>> exp)
        {
            string field = ExpressionHelper.GetPropertyName<T>(exp);
            return StartOrderBy(field);
        }


        public ICriteria<T> Select(params string[] cols)
        {
            if( cols == null || cols.Length == 0)
                return this;

            if (_selectFields == null) 
                _selectFields = new List<SelectField>();

            foreach (string col in cols)
                _selectFields.Add(new SelectField() { Field = col });

            return this;
        }


        public ICriteria<T> Select(string col)
        {
            if (_selectFields == null)
                _selectFields = new List<SelectField>();

            _selectFields.Add(new SelectField() { Field = col });
            return this;
        }


        public ICriteria<T> As(string columnAlias)
        {
            if (_selectFields == null || _selectFields.Count == 0)
                return this;

            _selectFields[_selectFields.Count - 1].Alias = columnAlias;
            return this;
        }


        public ICriteria<T> From(string tableName)
        {
            _from = tableName;
            return this;
        }


        public ICriteria<T> Where(string field)
        {
            return StartNewCondition(ConditionType.None, field);
        }     


        public ICriteria<T> And(string field)
        {
            return StartNewCondition(ConditionType.And, field);
        }


        public ICriteria<T> Or(string field)
        {
            return StartNewCondition(ConditionType.Or, field);
        }


        public ICriteria<T> Is(object val)
        {
            return BuildCondition("=", val);
        }


        public ICriteria<T> Not(object val)
        {
            return BuildCondition("<>", val);
        }


        public ICriteria<T> Null()
        {
            return BuildCondition("is null", string.Empty);
        }


        public ICriteria<T> NotNull()
        {
            return BuildCondition("is not null", string.Empty);
        }


        public ICriteria<T> In<TParam>(params object[] vals)
        {
            return BuildCondition<TParam>("in", vals);
        }


        public ICriteria<T> NotIn<TParam>(params object[] vals)
        {
            return BuildCondition<TParam>("not in", vals);
        }


        public ICriteria<T> Like(string val)
        {
            return BuildCondition("like", val);
        }


        public ICriteria<T> Like(string val, bool addWildcardPrefix, bool addWildcardSuffix)
        {
            if (addWildcardPrefix) val = "%" + val;
            if (addWildcardSuffix) val = val + "%";
            return BuildCondition("like", val);
        }


        public ICriteria<T> MoreThan(object val)
        {
            return BuildCondition(">", val);
        }


        public ICriteria<T> MoreEqual(object val)
        {
            return BuildCondition(">=", val);             
        }
        
        
        public ICriteria<T> LessThan(object val)
        {
            return BuildCondition("<", val);
        }


        public ICriteria<T> LessEqual(object val)
        {
            return BuildCondition("<=", val);
        }


        public ICriteria<T> OrderBy(string field)
        {
            return StartOrderBy(field);
        }


        public ICriteria<T> Asc
        {
            get { return BuildOrderBy(OrderByType.Asc); }
        }


        public ICriteria<T> Desc
        {
            get { return BuildOrderBy(OrderByType.Desc); }
        }


        public ICriteria<T> Limit(int maxRecords)
        {
            _recordLimit = maxRecords;
            _recordLimitEnabled = true;
            return this;
        }


        public ICriteria<T> End()
        {
            if (_lastCondition != null)
            {
                _conditions.Add(_lastCondition);
                _lastCondition = null;
            }
            if (_lastOrderBy != null)
            {
                _orderings.Add(_lastOrderBy);
                _lastOrderBy = null;
            }
            return this;
        }


        #region ICriteriaBuilder 
        public int GetLimit() { return _recordLimit; } 
        public bool HasLimit { get { return _recordLimitEnabled; } }

        public string Sql()
        {
            return Sql(_from);
        }


        public virtual string Sql(string from)
        {
            this.End();
            var buffer = new StringBuilder();
            string select = SqlSelect(false);
            string limit = SqlLimit();
            string conditions = SqlConditions(false);
            string orderby = SqlOrderBy(false);

            buffer.Append("select ");
            if(!string.IsNullOrEmpty(limit)) buffer.Append(limit + " ");
            if(!string.IsNullOrEmpty(select)) buffer.Append(select);
            buffer.Append(" from " + from + " ");
            if(!string.IsNullOrEmpty(conditions)) buffer.Append("where " + conditions);
            if(!string.IsNullOrEmpty(orderby)) buffer.Append(" order by " + orderby);

            return buffer.ToString();
        }


        public virtual string SqlSelect()
        {
            return SqlSelect(true);
        }


        public virtual string SqlSelect(bool includeSelectStatement)
        {
            if (_selectFields == null || _selectFields.Count == 0)
                return includeSelectStatement ? "select *" : "*";
            
            var buffer = new StringBuilder();

            buffer.Append(_selectFields[0].ToString(_surroundFields, _surroundFieldLeftChar, _surroundFieldRightChar));
            if (_selectFields.Count > 1)
            {
                for (int ndx = 1; ndx < _selectFields.Count; ndx++)
                {
                    buffer.Append(", " + _selectFields[ndx].ToString(_surroundFields, _surroundFieldLeftChar, _surroundFieldRightChar));
                }
            }
            if(includeSelectStatement)
                return "select " + buffer.ToString();

            return buffer.ToString();
        }


        public virtual string SqlLimit()
        {
            if (!_recordLimitEnabled) return string.Empty;

            return "top " + _recordLimit.ToString();
        }


        public string SqlConditions()
        {
            return SqlConditions(true);
        }


        public virtual string SqlConditions(bool includeWhere)
        {
            this.End();

            if (_conditions == null || _conditions.Count == 0)
                return string.Empty;

            var buffer = new StringBuilder();
            // Build the conditions.
            // id > 5 or name = 'kishore'
            foreach (var condition in _conditions)
            {
                if (condition.ConditionType != ConditionType.None)
                {
                    buffer.Append(condition.ConditionType.ToString() + " ");
                }
                buffer.Append(condition.ToString(_surroundFields, _surroundFieldLeftChar, _surroundFieldRightChar) + " ");
            }
            if(includeWhere)
                return " where " + buffer.ToString();

            return buffer.ToString();
        }


        public string SqlOrderBy()
        {
            return SqlOrderBy(true);
        }


        public virtual string SqlOrderBy(bool includeOrderBy)
        {
            this.End();
            if (_orderings == null || _orderings.Count == 0)
                return string.Empty;

            var buffer = new StringBuilder();
            // Build the order by clauses. 
            // e.g. order by createdate desc, username asc
            // Add first.
            buffer.Append(string.Format("{0} {1}", _orderings[0].Field, _orderings[0].Ordering.ToString()));
            if (_orderings.Count > 1)
            {
                for (int ndx = 1; ndx < _orderings.Count; ndx++)
                    buffer.Append(string.Format(", {0} {1}", _orderings[ndx].Field, _orderings[ndx].Ordering.ToString()));
            }
            if(includeOrderBy)
                return " order by " + buffer.ToString();

            return buffer.ToString();
        }
        #endregion


        #region Private Helper methods
        protected ICriteria<T> BuildCondition<T>(string comparison, params object[] vals)
        {
            var buffer = new StringBuilder();
            // String
            if (typeof(T) == typeof(string))
            {
                buffer.Append("'" + DataUtils.Encode((string)vals[0]) + "'");
                for (int ndx = 1; ndx < vals.Length; ndx++)
                    buffer.Append(", '" + DataUtils.Encode((string)vals[ndx]) + "'");
            }
            // DateTime
            else if (typeof(T) == typeof(DateTime))
            {
                buffer.Append("'" + DataUtils.Encode(((DateTime)vals[0]).ToShortDateString()) + "'");
                for (int ndx = 1; ndx < vals.Length; ndx++)
                    buffer.Append(", '" + DataUtils.Encode(((DateTime)vals[ndx]).ToShortDateString()) + "'");
            }
            // int, long, float, double.
            else if (Types.TypeHelper.IsNumeric(typeof(T)))
            {
                buffer.Append(Types.TypeHelper.Join(vals));
            }

            _lastCondition.Comparison = comparison;
            _lastCondition.Value =  "( " + buffer.ToString() + " )";
            return this;
        }


        protected ICriteria<T> BuildCondition(string comparison, object val)
        {
            var valToUse = ConvertVal(val);
            _lastCondition.Comparison = comparison;
            _lastCondition.Value = valToUse;
            return this;
        }


        protected ICriteria<T> StartOrderBy(string field)
        {
            if (_lastOrderBy != null)
            {
                _orderings.Add(_lastOrderBy);
            }
            _lastOrderBy = new OrderByClause() { Field = field };
            return this;
        }


        protected ICriteria<T> BuildOrderBy(OrderByType type)
        {
            _lastOrderBy.Ordering = type;
            return this;
        }


        protected string ConvertVal(object val)
        {
            string valToUse = "";

            // String
            if (val.GetType() == typeof(string) && string.IsNullOrEmpty((string)val))
                return "''";

            if (val.GetType() == typeof(string))
                valToUse = "'" + DataUtils.Encode((string)val) + "'";

            // Bool
            else if (val.GetType() == typeof(bool))
            {
                bool bval = (bool)val;
                valToUse = bval ? "1" : "0";
            }

            // DateTime
            else if (val.GetType() == typeof(DateTime))
            {
                DateTime date = (DateTime)val;
                valToUse = "'" + date.ToShortDateString() + "'";
            }

            // Int / Long / float / double
            else if( Types.TypeHelper.IsNumeric(val))
                valToUse = val.ToString();

            return valToUse;
        }


        protected ICriteria<T> StartNewCondition(ConditionType condition, string fieldName)
        {
            if (_lastCondition != null)
            {
                _conditions.Add(_lastCondition);
            }
            _lastCondition = new Condition(condition);
            _lastCondition.Field = fieldName;
            return this;
        }


        
        #endregion
    }


    public class OrderByClause
    {
        public string Field;
        public OrderByType Ordering;


        public override string ToString()
        {
            return ToString(false, "[", "]");
        }


        public virtual string ToString(bool surround, string left, string right)
        {
            if (!surround)
                return Field + " " + Ordering.ToString();

            return left + Field + right + " " + Ordering.ToString();
        }
    }


    public class SelectField
    {
        public string Field;
        public string Alias;

        public override string ToString()
        {
            return ToString(false, "[", "]");
        }


        public virtual string ToString(bool surround, string left, string right)
        {
            string aliasText = !string.IsNullOrEmpty(Alias) ? " as " + Alias : string.Empty;
            if (!surround)
                return Field + aliasText;

            return left + Field + right + aliasText;
        }
    }  


    public class Condition
    {
        /// <summary>
        /// And | Or
        /// </summary>
        public ConditionType ConditionType;
        public string Field;
        public string Comparison;
        public string Value;


        public Condition()
        {
        }


        public Condition(ConditionType condition)
        {
            ConditionType = condition;
        }


        public Condition(ConditionType condition, string key, string assignment, string val)
        {
            ConditionType = condition;
            Field = key;
            Comparison = assignment;
            Value = val;
        }


        public override string ToString()
        {
            return ToString(false, "[", "]");
        }


        public virtual string ToString(bool surround, string left, string right)
        {
            string col = surround ? left + Field + right : Field;
            string val = string.IsNullOrEmpty(Value) ? "''" : Value;

            return string.Format("{0} {1} {2}", col, Comparison, val);
        }
    }


    public enum ConditionType { And, Or, None };
    public enum OrderByType { Asc, Desc };   
}
