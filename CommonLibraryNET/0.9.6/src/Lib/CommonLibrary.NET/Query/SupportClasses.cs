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
    /// Represents a field in a select clause.
    /// </summary>
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



    /// <summary>
    /// Represents one entry to order by
    /// </summary>
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


    public enum Eq { Less, LessEqual, Equals, More, MoreEqual, NotEqual, In, NotIn };
}
