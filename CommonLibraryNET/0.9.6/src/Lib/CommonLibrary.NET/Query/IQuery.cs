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
    /// <summary>
    /// Interface for query
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public interface IQuery<T> : IQuery
    {
        IQuery<T> AddCondition(Expression<Func<T, object>> exp, ExpressionType comparison, object val);
        IQuery<T> Select(params Expression<Func<T, object>>[] colExpressions);
        IQuery<T> Where(Expression<Func<T, object>> exp);
        IQuery<T> And(Expression<Func<T, object>> exp);
        IQuery<T> Or(Expression<Func<T, object>> exp);
        IQuery<T> OrderBy(Expression<Func<T, object>> exp);
        IQuery<T> OrderByDescending(Expression<Func<T, object>> exp);
        IQuery<T> Select(params string[] cols);
        IQuery<T> Select(string field);
        IQuery<T> As(string alias);
        IQuery<T> From(string tableName);
        IQuery<T> Where(string field);
        IQuery<T> And(string field);
        IQuery<T> Or(string field);
        IQuery<T> In<TParam>(params object[] vals);
        IQuery<T> NotIn<TParam>(params object[] vals);
        IQuery<T> Null();
        IQuery<T> NotNull();
        IQuery<T> Like(string val);
        IQuery<T> Like(string val, bool addWildcardPrefix, bool addWildcardSuffix);
        IQuery<T> Is(object val);
        IQuery<T> Not(object val);
        IQuery<T> LessThan(object val);
        IQuery<T> LessEqual(object val);
        IQuery<T> MoreThan(object val);
        IQuery<T> MoreEqual(object val);
        IQuery<T> OrderBy(string field);
        IQuery<T> OrderByDescending(string field);
        IQuery<T> Limit(int maxRecords);
        IQuery<T> End();
    }



    public interface IQuery
    {
        QueryData Data { get; set; }
        IQueryBuilder Builder { get; set; }

        void AddCondition(string field, ExpressionType comparison, object val);
        void Complete();
    }



    public interface IQueryBuilder
    {
        IQuery Criteria { get; set; }

        string Build();
        string Build(string from);
        string BuildSelect();
        string BuildSelect(bool includeSelect);
        string BuildLimit();
        string BuildConditions();
        string BuildConditions(bool includeWhere);
        string BuildConditionsAndOrdering(bool includeWhere, bool includeOrderBy);
        string BuildOrderBy();
        string BuildOrderBy(bool includeOrderBy);
    }
}