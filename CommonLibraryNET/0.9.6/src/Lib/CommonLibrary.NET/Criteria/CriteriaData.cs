using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib.Data
{
    public class QueryData
    {
        public List<Condition> Conditions;
        public List<OrderByClause> Orderings;
        public OrderByClause LastOrderBy;
        public List<SelectField> SelectFields;
        public string From;
        public int RecordLimit;
        public bool IsRecordLimitEnabled;


        public QueryData()
        {
            Conditions = new List<Condition>();
            SelectFields = new List<SelectField>();
            Orderings = new List<OrderByClause>();
        }
    }
}
