using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;
using System.Reflection;


namespace ComLib.Entities
{
    class RepositoryExpressionHelper
    {
        /// <summary>
        /// Get the property name from the expression.
        /// e.g. GetPropertyName(Person)( p => p.FirstName);
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="exp"></param>
        /// <returns></returns>
        public static string GetPropertyName<T>(Expression<Func<T, object>> exp)
        {
            MemberExpression memberExpression = null;

            // Get memberexpression.
            if (exp.Body is MemberExpression) 
                memberExpression = exp.Body as MemberExpression;

            if (exp.Body is UnaryExpression)
            {
                var unaryExpression = exp.Body as UnaryExpression;
                if (unaryExpression.Operand is MemberExpression)
                    memberExpression = unaryExpression.Operand as MemberExpression;
            }
            
            if( memberExpression == null)
                throw new InvalidOperationException("Not a member access.");

            var info = memberExpression.Member as PropertyInfo;
            return info.Name;
        }


        /// <summary>
        /// Delete using the expression.
        /// e.g. entity.LogLevel == 1
        /// </summary>
        /// <param name="expression"></param>
        public static string BuildSinglePropertyCondition<T>(Expression<Func<T, bool>> predicate)
        {
            string format = "{0} {1} {2}";
            BinaryExpression exp = (BinaryExpression)predicate.Body;
            MemberExpression left = null;
            if (exp.Left is MemberExpression)
            {
                left = (MemberExpression)exp.Left;
            }
            else if (exp.Left is UnaryExpression)
            {
                left = ((UnaryExpression)exp.Left).Operand as MemberExpression;
            }

            string fieldName = left.Member.Name;
            string operand = RepositoryExpressionTypeHelper.GetText(exp.NodeType);
            string fieldVal = "";
            object rval = null;

            if (exp.Right is ConstantExpression)
            {
                ConstantExpression right = (ConstantExpression)exp.Right;
                fieldVal = GetVal(right.Value);
            }
            else if (exp.Right is MemberExpression)
            {
                MemberExpression right = (MemberExpression)exp.Right;
                rval = Expression.Lambda(exp.Right).Compile().DynamicInvoke();
            }
            else
            {
                rval = Expression.Lambda(exp.Right).Compile().DynamicInvoke();
                fieldVal = rval.ToString();
            }

            // Check for data types.
            PropertyInfo propInfo = left.Member as PropertyInfo;

            // String ? Encode single quotes. ' = ''
            if (propInfo.PropertyType == typeof(string))
            {
                fieldVal = string.Format("'{0}'", fieldVal.Replace("'", "''"));
            }
            else if (propInfo.PropertyType == typeof(DateTime))
            {
                fieldVal = "'" + ((DateTime)rval).ToShortDateString() + "'";
            }

            string val = string.Format(format, fieldName, operand, fieldVal);
            return val;
        }


        public static string GetVal(object val)
        {
            if (val.GetType() == typeof(bool) || val.GetType() == typeof(Boolean))
            {
                bool boolVal = (bool)val;
                return boolVal == true ? "1" : "0";
            }
            return val.ToString();
        }
    }


    class RepositoryExpressionTypeHelper
    {
        private static IDictionary<ExpressionType, string> _map = new Dictionary<ExpressionType, string>();

        static RepositoryExpressionTypeHelper()
        {
            _map[ExpressionType.Equal] = "=";
            _map[ExpressionType.NotEqual] = "<>";
            _map[ExpressionType.GreaterThanOrEqual] = ">=";
            _map[ExpressionType.LessThanOrEqual] = "<=";
            _map[ExpressionType.LessThan] = "<";
            _map[ExpressionType.GreaterThan] = ">";
        }


        /// <summary>
        /// Get the sql text equivalent of the expression type.
        /// </summary>
        /// <param name="expType"></param>
        /// <returns></returns>
        public static string GetText(ExpressionType expType)
        {
            if (!_map.ContainsKey(expType))
                throw new ArgumentException("expresion type :" + expType.ToString() + "not supported.");

            return _map[expType];
        }
    }
}
