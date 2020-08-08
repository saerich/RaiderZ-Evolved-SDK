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
using System.Linq.Expressions;




namespace ComLib.Data
{

    public class Query<T> : Query, IQuery<T>
    {
        /// <summary>
        /// Creates a new instance.
        /// </summary>
        /// <returns></returns>
        public static IQuery<T> New()
        {
            return New(string.Empty);
        }


        // Static factory
        /// <summary>
        /// Creates new Query object for the specified db provider name.
        /// </summary>
        /// <param name="dbProviderName">Name of the db provider.</param>
        /// <returns></returns>
        public static IQuery<T> New(string dbProviderName)
        {
            if (string.IsNullOrEmpty(dbProviderName)) return new Query<T>();
            
            // No support for other databases yet.
            // Not sure it's necessay but just in case.
            return new Query<T>();
        }


        /// <summary>
        /// Initializes a new instance of the <see cref="Query&lt;T&gt;"/> class.
        /// </summary>
        public Query()
        {
            _builder = new QueryBuilderSql(this);
        }


        public IQuery<T> AddCondition(Expression<Func<T, object>> exp, ExpressionType comparison, object val)
        {
            string field = ExpressionHelper.GetPropertyName<T>(exp);
            AddCondition(field, comparison, val);
            return this;
        }


        public IQuery<T> Select(params Expression<Func<T, object>>[] colExpressions)
        {
            if (colExpressions == null || colExpressions.Length == 0)
                return this;

            string[] cols = new string[colExpressions.Length];
            for (int ndx = 0; ndx < colExpressions.Length; ndx++)
                cols[ndx] = ExpressionHelper.GetPropertyName<T>(colExpressions[ndx]);

            return Select(cols);
        }


        public IQuery<T> Where(Expression<Func<T, object>> exp)
        {
            string field = ExpressionHelper.GetPropertyName<T>(exp);
            return StartNewCondition(ConditionType.None, field) as IQuery<T>;
        }


        public IQuery<T> And(Expression<Func<T, object>> exp)
        {
            string field = ExpressionHelper.GetPropertyName<T>(exp);
            return StartNewCondition(ConditionType.And, field) as IQuery<T>;
        }


        public IQuery<T> Or(Expression<Func<T, object>> exp)
        {
            string field = ExpressionHelper.GetPropertyName<T>(exp);
            return StartNewCondition(ConditionType.Or, field) as IQuery<T>;
        }


        public IQuery<T> OrderBy(Expression<Func<T, object>> exp)
        {
            string field = ExpressionHelper.GetPropertyName<T>(exp);
            return OrderByInternal(field, OrderByType.Asc);
        }


        public IQuery<T> OrderByDescending(Expression<Func<T, object>> exp)
        {
            string field = ExpressionHelper.GetPropertyName<T>(exp);
            return OrderByInternal(field, OrderByType.Desc);
        }


        public IQuery<T> Select(params string[] cols)
        {
            if( cols == null || cols.Length == 0)
                return this;
            
            foreach (string col in cols)
                _data.SelectFields.Add(new SelectField() { Field = col });

            return this;
        }


        public IQuery<T> Select(string col)
        {
            _data.SelectFields.Add(new SelectField() { Field = col });
            return this;
        }


        public IQuery<T> As(string columnAlias)
        {
            if (_data.SelectFields == null || _data.SelectFields.Count == 0)
                return this;

            _data.SelectFields[_data.SelectFields.Count - 1].Alias = columnAlias;
            return this;
        }


        public IQuery<T> From(string tableName)
        {
            _data.From = tableName;
            return this;
        }


        public IQuery<T> Where(string field)
        {
            return StartNewCondition(ConditionType.None, field) as IQuery<T>;
        }     


        public IQuery<T> And(string field)
        {
            return StartNewCondition(ConditionType.And, field) as IQuery<T>;
        }


        public IQuery<T> Or(string field)
        {
            return StartNewCondition(ConditionType.Or, field) as IQuery<T>;
        }


        public IQuery<T> Is(object val)
        {
            return BuildCondition("=", val);
        }


        public IQuery<T> Not(object val)
        {
            return BuildCondition("<>", val);
        }


        public IQuery<T> Null()
        {
            _lastCondition.Comparison = "is";
            _lastCondition.Value = "null";
            return this;
        }


        public IQuery<T> NotNull()
        {
            _lastCondition.Comparison = "is not";
            _lastCondition.Value = "null";
            return this;
        }


        public IQuery<T> In<TParam>(params object[] vals)
        {
            return BuildCondition<TParam>("in", vals);
        }


        public IQuery<T> NotIn<TParam>(params object[] vals)
        {
            return BuildCondition<TParam>("not in", vals);
        }


        public IQuery<T> Like(string val)
        {
            return BuildCondition("like", val);
        }


        public IQuery<T> Like(string val, bool addWildcardPrefix, bool addWildcardSuffix)
        {
            if (addWildcardPrefix) val = "%" + val;
            if (addWildcardSuffix) val = val + "%";
            return BuildCondition("like", val);
        }


        public IQuery<T> MoreThan(object val)
        {
            return BuildCondition(">", val);
        }


        public IQuery<T> MoreEqual(object val)
        {
            return BuildCondition(">=", val);             
        }
        
        
        public IQuery<T> LessThan(object val)
        {
            return BuildCondition("<", val);
        }


        public IQuery<T> LessEqual(object val)
        {
            return BuildCondition("<=", val);
        }


        public IQuery<T> OrderBy(string field)
        {
            return OrderByInternal(field, OrderByType.Asc);
        }


        public IQuery<T> OrderByDescending(string field)
        {
            return OrderByInternal(field, OrderByType.Desc);
        }


        public IQuery<T> Limit(int maxRecords)
        {
            _data.RecordLimit = maxRecords;
            _data.IsRecordLimitEnabled = true;
            return this;
        }


        public IQuery<T> End()
        {
            Complete();
            return this;
        }


        #region Private Helper methods
        protected IQuery<T> BuildCondition<TParam>(string comparison, params object[] vals)
        {
            var buffer = new StringBuilder();
            // String
            if (typeof(TParam) == typeof(string))
            {
                buffer.Append("'" + Encode((string)vals[0]) + "'");
                for (int ndx = 1; ndx < vals.Length; ndx++)
                    buffer.Append(", '" + Encode((string)vals[ndx]) + "'");
            }
            // DateTime
            else if (typeof(TParam) == typeof(DateTime))
            {
                buffer.Append("'" + Encode(((DateTime)vals[0]).ToShortDateString()) + "'");
                for (int ndx = 1; ndx < vals.Length; ndx++)
                    buffer.Append(", '" + Encode(((DateTime)vals[ndx]).ToShortDateString()) + "'");
            }
            // int, long, float, double.
            else if (TypeHelper.IsNumeric(typeof(TParam)))
            {
                buffer.Append(TypeHelper.Join(vals));
            }
            
            _lastCondition.Comparison = comparison;
            _lastCondition.Value =  "( " + buffer.ToString() + " )";
            return this;
        }


        protected IQuery<T> BuildCondition(string comparison, object val)
        {
            var valToUse = ConvertVal(val);
            _lastCondition.Comparison = comparison;
            _lastCondition.Value = valToUse;
            return this;
        }


        protected IQuery<T> OrderByInternal(string field, OrderByType order)
        {
            if (_data.LastOrderBy != null)
            {
                _data.Orderings.Add(_data.LastOrderBy);
            }
            _data.LastOrderBy = new OrderByClause() { Field = field, Ordering = order };
            return this;
        }
        #endregion
    }



    /// <summary>
    /// Base class for Query.
    /// </summary>
    public class Query : IQuery
    {
        protected QueryData _data = new QueryData();
        protected IQueryBuilder _builder;
        protected Condition _lastCondition;


        public static IQuery<object> New()
        {
            return Query<object>.New();
        }


        /// <summary>
        /// The data for the criteria, including select field, conditions, orderby etc.
        /// </summary>
        public QueryData Data
        {
            get { return _data; }
            set { _data = value; }
        }


        /// <summary>
        /// The data for the criteria, including select field, conditions, orderby etc.
        /// </summary>
        public IQueryBuilder Builder
        {
            get { return _builder; }
            set { _builder = value; }
        }


        public void AddCondition(string field, ExpressionType comparison, object val)
        {
            StartNewCondition(ConditionType.And, field);
            string sign = RepositoryExpressionTypeHelper.GetText(comparison);
            BuildCondition(sign, val);
        }


        /// <summary>
        /// Completes this condition construction.
        /// </summary>
        public void Complete()
        {
            if (_lastCondition != null)
            {
                _data.Conditions.Add(_lastCondition);
                _lastCondition = null;
            }
            if (_data.LastOrderBy != null)
            {
                _data.Orderings.Add(_data.LastOrderBy);
                _data.LastOrderBy = null;
            }
        }


        protected IQuery BuildCondition<TParam>(string comparison, params object[] vals)
        {
            var buffer = new StringBuilder();
            // String
            if (typeof(TParam) == typeof(string))
            {
                buffer.Append("'" + Encode((string)vals[0]) + "'");
                for (int ndx = 1; ndx < vals.Length; ndx++)
                    buffer.Append(", '" + Encode((string)vals[ndx]) + "'");
            }
            // DateTime
            else if (typeof(TParam) == typeof(DateTime))
            {
                buffer.Append("'" + Encode(((DateTime)vals[0]).ToShortDateString()) + "'");
                for (int ndx = 1; ndx < vals.Length; ndx++)
                    buffer.Append(", '" + Encode(((DateTime)vals[ndx]).ToShortDateString()) + "'");
            }
            // int, long, float, double.
            else if (TypeHelper.IsNumeric(typeof(TParam)))
            {
                buffer.Append(TypeHelper.Join(vals));
            }

            _lastCondition.Comparison = comparison;
            _lastCondition.Value = "( " + buffer.ToString() + " )";
            return this;
        }


        protected IQuery BuildCondition(string comparison, object val)
        {
            var valToUse = ConvertVal(val);
            _lastCondition.Comparison = comparison;
            _lastCondition.Value = valToUse;
            return this;
        }


        protected string ConvertVal(object val)
        {
            string valToUse = "";

            // String
            if (val == null || (val.GetType() == typeof(string) && string.IsNullOrEmpty((string)val)))
                return "''";

            if (val.GetType() == typeof(string))
                valToUse = "'" + Encode((string)val) + "'";

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
            else if (TypeHelper.IsNumeric(val))
                valToUse = val.ToString();

            return valToUse;
        }


        protected IQuery StartNewCondition(ConditionType condition, string fieldName)
        {
            if (_lastCondition != null)
            {
                _data.Conditions.Add(_lastCondition);
            }
            _lastCondition = new Condition(condition);
            _lastCondition.Field = fieldName;
            return this;
        }


        /// <summary>
        /// Encode the text for single quotes.
        /// </summary>
        /// <param name="text"></param>
        /// <returns></returns>
        public static string Encode(string text)
        {
            if (string.IsNullOrEmpty(text)) return text;

            return text.Replace("'", "''");
        }
    }
}
