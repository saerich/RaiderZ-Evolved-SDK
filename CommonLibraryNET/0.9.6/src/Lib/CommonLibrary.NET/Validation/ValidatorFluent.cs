using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;
using System.Text.RegularExpressions;


namespace ComLib.ValidationSupport
{
    public class ValidatorFluent
    {
        private object _target;
        private string _objectName;
        private bool _appendObjectNameToError;
        private string _propertyName;
        private bool _checkCondition;
        private IValidationResults _errors;
        private int _initialErrorCount;


        public ValidatorFluent(Type typeToCheck) : this(typeToCheck, false, null)
        {
        }


        public ValidatorFluent(Type typeToCheck, IValidationResults errors)
            : this(typeToCheck, false, errors)
        {
        }


        public ValidatorFluent(Type typeToCheck, bool appendTypeToError, IValidationResults errors)
        {
            _objectName = typeToCheck.Name;
            _appendObjectNameToError = appendTypeToError;
            _errors = errors == null ? new ValidationResults() : errors;
            _initialErrorCount = _errors.Count;
        }


        public bool HasErrors
        {
            get { return _errors.Count > _initialErrorCount; }
        }


        public IValidationResults Errors
        {
            get { return _errors; }
            set { _errors = value; }
        }


        public ValidatorFluent Check(object target)
        {
            // Reset the check condition flag.
            _checkCondition = true;
            _propertyName = string.Empty;
            _target = target;
            return this;
        }


        public ValidatorFluent Check(Expression<Func<object>> exp)
        {
            _target = ExpressionHelper.GetPropertyNameAndValue(exp, ref _propertyName);
            _checkCondition = true;
            return this;
        }


        public ValidatorFluent Check(string propName, object target)
        {
            // Reset the check condition flag.            
            _checkCondition = true;
            _propertyName = propName;
            _target = target;
            return this;
        }


        public ValidatorFluent If(bool isOkToCheckNext)
        {
            _checkCondition = isOkToCheckNext;
            return this;
        }


        public ValidatorFluent Is(object val)
        {
            if (!_checkCondition) return this;

            if(_target == null && val == null)
                return this;

            if(_target == null && val != null)
                return IsValid(false, "must equal : " + val.ToString());

            return IsValid(val.Equals(_target), "must equal : " + val.ToString());
        }


        public ValidatorFluent IsNot(object val)
        {
            if (!_checkCondition) return this;
            
            if (_target == null && val == null)
                return this;

            if (_target == null && val != null)
                return this;

            return IsValid(!val.Equals(_target), "must not equal : " + val.ToString());
        }


        public ValidatorFluent IsNull()
        {
            if (!_checkCondition) return this;
            
            return IsValid(_target == null, "must be null");
        }


        public ValidatorFluent IsNotNull()
        {
            if (!_checkCondition) return this;

            return IsValid(_target != null, "must be not null");
        }


        public ValidatorFluent In<T>(params object[] vals)
        {
            if (!_checkCondition) return this;

            if (vals == null || vals.Length == 0)
                return this;

            T checkVal = Converter.ConvertTo<T>(_target);
            bool isValid = false;
            foreach (object val in vals)
            {
                T validVal = Converter.ConvertTo<T>(val);
                if (checkVal.Equals(validVal))
                {
                    isValid = true;
                    break;
                }
            }
            return IsValid(isValid, "is not a valid value");
        }


        public ValidatorFluent NotIn<T>(params object[] vals)
        {
            if (!_checkCondition) return this;

            if (vals == null || vals.Length == 0)
                return this;

            T checkVal = Converter.ConvertTo<T>(_target);
            bool isValid = true;
            foreach (object val in vals)
            {
                T validVal = Converter.ConvertTo<T>(val);
                if (checkVal.Equals(validVal))
                {
                    isValid = false;
                    break;
                }
            }
            return IsValid(isValid, "is not a valid value");
        }


        public ValidatorFluent Matches(string regex)
        {
            if (!_checkCondition) return this;

            return IsValid(Regex.IsMatch((string)_target, regex), "does not match pattern : " + regex);
        }


        public ValidatorFluent IsBetween(int min, int max)
        {
            if (!_checkCondition) return this;

            bool isNumeric = TypeHelper.IsNumeric(_target);
            if( isNumeric )
            {
                double val = Convert.ToDouble(_target);
                return IsValid(min <= val && val <= max, "must be between : " + min + ", " + max);
            }
            else
            {
                // can only be string.
                string strVal = _target as string;
                if(min > 0 && string.IsNullOrEmpty(strVal))
                    return IsValid(false, "length must be between : " + min + ", " + max);

                return IsValid(min <= strVal.Length && strVal.Length <= max, "length must be between : " + min + ", " + max);
            } 
        }


        public ValidatorFluent Contains(string val)
        {
            if (!_checkCondition) return this;

            if (string.IsNullOrEmpty((string)_target))
                return IsValid(false, "does not contain : " + val);

            string valToCheck = (string)_target;
            return IsValid(valToCheck.Contains(val), "must contain : " + val);
        }


        public ValidatorFluent NotContain(string val)
        {
            if (!_checkCondition) return this;

            if (string.IsNullOrEmpty((string)_target))
                return this;

            string valToCheck = (string)_target;
            return IsValid(!valToCheck.Contains(val), "should not contain : " + val);
        }


        public ValidatorFluent Min(int min)
        {
            if (!_checkCondition) return this;

            bool isNumeric = TypeHelper.IsNumeric(_target);
            if (!isNumeric) return IsValid(false, "must be numeric value");

            double val = Convert.ToDouble(_target);
            return IsValid(val >= min, "must have minimum value of : " + min);
        }


        public ValidatorFluent Max(int max)
        {
            if (!_checkCondition) return this;

            bool isNumeric = TypeHelper.IsNumeric(_target);
            if (!isNumeric) return IsValid(false, "must be numeric value");

            double val = Convert.ToDouble(_target);
            return IsValid(val <= max, "must have maximum value of : " + max);
        }


        public ValidatorFluent IsTrue()
        {
            if (!_checkCondition) return this;

            bool isBool = _target is bool;
            if (!isBool) return IsValid(false, "must be bool(true/false)");

            return IsValid(((bool)_target) == true, "must be true");
        }


        public ValidatorFluent IsFalse()
        {
            if (!_checkCondition) return this;

            bool isBool = _target is bool;
            if (!isBool) return IsValid(false, "must be bool(true/false)");

            return IsValid(((bool)_target) == false, "must be false");
        }


        public ValidatorFluent IsAfterToday()
        {
            if (!_checkCondition) return this;

            IsAfter(DateTime.Today);
            _checkCondition = false;
            return this;
        }


        public ValidatorFluent IsBeforeToday()
        {
            if (!_checkCondition) return this;

            IsBefore(DateTime.Today);
            _checkCondition = false;
            return this;
        }


        public ValidatorFluent IsAfter(DateTime date)
        {
            if (!_checkCondition) return this;

            DateTime checkVal = (DateTime)_target;
            return IsValid(checkVal.Date.CompareTo(date.Date) > 0, "must be after date : " + date.ToShortDateString());
        }


        public ValidatorFluent IsBefore(DateTime date)
        {
            if (!_checkCondition) return this;

            DateTime checkVal = (DateTime)_target;
            return IsValid(checkVal.Date.CompareTo(date.Date) < 0, "must be before date : " + date.ToShortDateString());
        }


        public ValidatorFluent IsValidEmail()
        {
            if (!_checkCondition) return this;

            return IsValid(Validation.IsEmail((string)_target, false), "must be a valid email.");
        }


        public ValidatorFluent IsValidPhoneUS()
        {
            if (!_checkCondition) return this;

            return IsValid(Validation.IsPhoneUS((string)_target, false), "must be a valid U.S phone.");
        }


        public ValidatorFluent IsValidUrl()
        {
            if (!_checkCondition) return this;

            return IsValid(Validation.IsUrl((string)_target, false), "must be a valid url.");
        }


        public ValidatorFluent IsValidZip()
        {
            if (!_checkCondition) return this;

            return IsValid(Validation.IsZipCode((string)_target, false), "must be a valid zip.");
        }


        public ValidatorFluent End()
        {
            return this;
        }


        #region Check
        private ValidatorFluent IsValid(bool isValid, string error)
        {
            if (!isValid)
            {
                string prefix = string.IsNullOrEmpty(_propertyName) ? "Property " : _propertyName + " ";
                _errors.Add(prefix + error);
            }
            return this;
        }
        #endregion
    }
}
