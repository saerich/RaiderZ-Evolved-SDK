using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib
{
    public class ExtensionMetaData
    {
        public string Id;
        public object Instance;       
        public ExtensionAttribute Attribute;
        public List<object> AdditionalAttributes = new List<object>();
        public Type DataType;
    }
}
