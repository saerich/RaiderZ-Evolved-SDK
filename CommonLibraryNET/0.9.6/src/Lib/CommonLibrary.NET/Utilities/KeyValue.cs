using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib
{
    /// <summary>
    /// KeyValue pair. This is a class not a struct like the KeyValuePair.
    /// Unlike Tuple, this has Key/Value fields instead of properties that can be modified.
    /// Also,
    /// </summary>
    public class KeyValue<TKey, TValue>
    {
        /// <summary>
        /// The key.
        /// </summary>
        public TKey Key { get; set;}


        /// <summary>
        /// The value.
        /// </summary>
        public TValue Value { get; set; }


        /// <summary>
        /// Initialize
        /// </summary>
        public KeyValue()
        {
        }


        /// <summary>
        /// Initialize with supplied values.
        /// </summary>
        /// <param name="key"></param>
        /// <param name="val"></param>
        public KeyValue(TKey key, TValue val)
        {
            Key = key;
            Value = val;
        }


        /// <summary>
        /// Get string representation.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return string.Format("{0}:{1}", Key, Value);
        }
    }
}
