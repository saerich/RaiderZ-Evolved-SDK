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

namespace ComLib.Data
{
    public class QueryBuilderSql : IQueryBuilder
    {
        protected bool _surroundFields;
        protected string _surroundFieldLeftChar = "[";
        protected string _surroundFieldRightChar = "]";
        private IQuery _criteria;
        private static IDictionary<string, bool> _reservedWords = new Dictionary<string, bool>();


        static QueryBuilderSql()
        {
            _reservedWords["name"] = true;
            _reservedWords["group"] = true;
            _reservedWords["year"] = true;
            _reservedWords["month"] = true;
            _reservedWords["key"] = true;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="QueryBuilderSql"/> class.
        /// </summary>
        public QueryBuilderSql(IQuery criteria)
        {
            _criteria = criteria;
        }


        /// <summary>
        /// The criteria API.
        /// </summary>
        public IQuery Criteria
        {
            get { return _criteria; }
            set { _criteria = value; }
        }


        #region IQueryBuilder

        /// <summary>
        /// Builds the full markup for the criteria provided.
        /// </summary>
        /// <returns></returns>
        public string Build()
        {
            return Build(_criteria.Data.From);
        }


        /// <summary>
        /// Builds the full markup fro the criteria provided using the from source supplied.
        /// </summary>
        /// <param name="from">From.</param>
        /// <returns></returns>
        public virtual string Build(string from)
        {
            _criteria.Complete();
            var buffer = new StringBuilder();
            string select = BuildSelect(false);
            string limit = BuildLimit();
            string conditions = BuildConditions(false);
            string orderby = BuildOrderBy(false);

            buffer.Append("select ");
            if (!string.IsNullOrEmpty(limit)) buffer.Append(limit + " ");
            if (!string.IsNullOrEmpty(select)) buffer.Append(select);
            buffer.Append(" from " + from + " ");
            if (!string.IsNullOrEmpty(conditions)) buffer.Append("where " + conditions);
            if (!string.IsNullOrEmpty(orderby)) buffer.Append(" order by " + orderby);

            return buffer.ToString();
        }


        /// <summary>
        /// Builds only the "select fields" markup.
        /// </summary>
        /// <returns></returns>
        public virtual string BuildSelect()
        {
            return BuildSelect(true);
        }


        /// <summary>
        /// Builds only the select fields marketup with conditional flag indicating whether or not
        /// to include the markup's select statement.
        /// </summary>
        /// <param name="includeSelectStatement">if set to <c>true</c> [include select statement].</param>
        /// <returns></returns>
        public virtual string BuildSelect(bool includeSelectStatement)
        {
            QueryData data = _criteria.Data;
            if (data.SelectFields == null || data.SelectFields.Count == 0)
                return includeSelectStatement ? "select *" : "*";

            var buffer = new StringBuilder();

            bool encloseField = EncloseField(data.SelectFields[0].Field);
            buffer.Append(data.SelectFields[0].ToString(encloseField, _surroundFieldLeftChar, _surroundFieldRightChar));
            if (data.SelectFields.Count > 1)
            {
                for (int ndx = 1; ndx < data.SelectFields.Count; ndx++)
                {
                    encloseField = EncloseField(data.SelectFields[ndx].Field);
                    buffer.Append(", " + data.SelectFields[ndx].ToString(encloseField, _surroundFieldLeftChar, _surroundFieldRightChar));
                }
            }
            if (includeSelectStatement)
                return "select " + buffer.ToString();

            return buffer.ToString();
        }


        /// <summary>
        /// Builds the limit.
        /// </summary>
        /// <returns></returns>
        public virtual string BuildLimit()
        {
            QueryData data = _criteria.Data;
            if (!data.IsRecordLimitEnabled) return string.Empty;

            return "top " + data.RecordLimit.ToString();
        }


        /// <summary>
        /// Buildses the conditions.
        /// </summary>
        /// <returns></returns>
        public string BuildConditions()
        {
            return BuildConditions(true);
        }


        /// <summary>
        /// Buildses the conditions.
        /// </summary>
        /// <returns></returns>
        public string BuildConditionsAndOrdering(bool includeWhere, bool includeOrderBy)
        {
            StringBuilder buffer = new StringBuilder();
            string conditions = BuildConditions(includeWhere);
            string orderby = BuildOrderBy(includeOrderBy);

            if (!string.IsNullOrEmpty(conditions)) buffer.Append(" " + conditions);
            if (!string.IsNullOrEmpty(orderby)) buffer.Append("  " + orderby);
            string sql = buffer.ToString();
            return sql;
        }


        /// <summary>
        /// Whether or not to enclose fields.
        /// </summary>
        /// <param name="fieldName"></param>
        /// <returns></returns>
        public bool EncloseField(string fieldName)
        {            
            if (_reservedWords.ContainsKey(fieldName.ToLower()))
                return true;

            return false;
        }


        /// <summary>
        /// Whether or not to enclose fields.
        /// </summary>
        /// <param name="fieldName"></param>
        /// <returns></returns>
        protected string HandleEncloseField(string fieldName)
        {
            if (_reservedWords.ContainsKey(fieldName.ToLower()))
            {
                return _surroundFieldLeftChar + fieldName + _surroundFieldRightChar;
            }
            return fieldName;
        }


        /// <summary>
        /// Builds the conditions.
        /// </summary>
        /// <param name="includeWhere">if set to <c>true</c> [include where].</param>
        /// <returns></returns>
        public virtual string BuildConditions(bool includeWhere)
        {
            _criteria.Complete();
            QueryData data = _criteria.Data;
            if (data.Conditions == null || data.Conditions.Count == 0)
                return string.Empty;

            var buffer = new StringBuilder();
            // Build the conditions.
            // id > 5 or name = 'kishore'
            for(int ndx = 0; ndx < data.Conditions.Count; ndx++)
            {
                var condition = data.Conditions[ndx];
                if (condition.ConditionType != ConditionType.None && ndx > 0)
                {
                    buffer.Append(condition.ConditionType.ToString() + " ");
                }
                bool encloseField = EncloseField(condition.Field);
                buffer.Append(condition.ToString(encloseField, _surroundFieldLeftChar, _surroundFieldRightChar) + " ");
            }
            if (includeWhere)
                return " where " + buffer.ToString();

            return buffer.ToString();
        }


        /// <summary>
        /// Builds the order by.
        /// </summary>
        /// <returns></returns>
        public string BuildOrderBy()
        {
            return BuildOrderBy(true);
        }


        /// <summary>
        /// Builds the order by.
        /// </summary>
        /// <param name="includeOrderBy">if set to <c>true</c> [include order by].</param>
        /// <returns></returns>
        public virtual string BuildOrderBy(bool includeOrderBy)
        {
            _criteria.Complete();
            QueryData data = _criteria.Data;
            if (data.Orderings == null || data.Orderings.Count == 0)
                return string.Empty;

            var buffer = new StringBuilder();
            // Build the order by clauses. 
            // e.g. order by createdate desc, username asc
            // Add first.
            string fieldName = HandleEncloseField(data.Orderings[0].Field);
            buffer.Append(string.Format("{0} {1}", fieldName, data.Orderings[0].Ordering.ToString()));
            if (data.Orderings.Count > 1)
            {
                for (int ndx = 1; ndx < data.Orderings.Count; ndx++)
                {
                    fieldName = HandleEncloseField(data.Orderings[ndx].Field);
                    buffer.Append(string.Format(", {0} {1}", fieldName, data.Orderings[ndx].Ordering.ToString()));
                }
            }
            if (includeOrderBy)
                return " order by " + buffer.ToString();

            return buffer.ToString();
        }
        #endregion
    }
}
