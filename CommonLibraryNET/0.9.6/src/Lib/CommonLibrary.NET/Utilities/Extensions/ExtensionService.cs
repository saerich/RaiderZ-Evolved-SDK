using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using ComLib.Collections;
using ComLib.Configuration;
using ComLib.Reflection;

namespace ComLib
{
    
    /// <summary>
    /// Extension Service for dynamically loading types from an assembly(s).
    /// </summary>
    public class ExtensionService<TAttrib, TExtension> where TAttrib : ExtensionAttribute
    {
        protected string _defaultAssemblyForExtensions = "CommonLibrary.Web.Modules";
        protected IDictionary<string, ExtensionMetaData> _lookup = new DictionaryOrdered<string, ExtensionMetaData>();
        protected Action<KeyValuePair<Type, TAttrib>> _onLoadCallback;
        protected Action _onLoadCompleteCallback;


        /// <summary>
        /// Lookup all the availably dynamically loaded extensions
        /// </summary>
        public IDictionary<string, ExtensionMetaData> Lookup { get { return _lookup; } }


        /// <summary>
        /// Load all the information tasks available.
        /// </summary>
        /// <param name="assembliesDelimited"></param>
        public virtual void Load(string assembliesDelimited)
        {
            if (string.IsNullOrEmpty(assembliesDelimited))
                assembliesDelimited = _defaultAssemblyForExtensions;
            var assemblies = assembliesDelimited.Split(',');
            foreach (var assemblyname in assemblies)
            {
                // This dynamically gets all the classes that have the InfoAttribute on them,
                // which means they return some information.
                var extensions = AttributeHelper.GetClassAttributesFromAssembly<TAttrib>(assemblyname, (pair) =>
                {
                    pair.Value.DeclaringType = pair.Key.FullName;
                    pair.Value.DeclaringDataType = pair.Key;
                    pair.Value.DeclaringAssembly = assemblyname;
                    if (_onLoadCallback != null)
                    {
                        _onLoadCallback(pair);
                    }
                    Register(pair.Value);
                });
            }
            if (_onLoadCompleteCallback != null)
                _onLoadCompleteCallback();
        }


        /// <summary>
        /// Create the instance of the extension.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public virtual TExtension Create(string name)
        {
            var metadata = Lookup[name];
            Assembly assembly = null;
            Type type = null;
            if (metadata.Attribute.IsReusable)
            {
                if (metadata.Instance == null)
                {
                    assembly = Assembly.Load(metadata.Attribute.DeclaringAssembly);
                    type = assembly.GetType(metadata.Attribute.DeclaringType);
                    metadata.Instance = Activator.CreateInstance(type);
                }
                return (TExtension)metadata.Instance;
            }

            assembly = Assembly.Load(metadata.Attribute.DeclaringAssembly);
            type = assembly.GetType(metadata.Attribute.DeclaringType);
            var instance = Activator.CreateInstance(type);
            return (TExtension)instance;
        }

        
        /// <summary>
        /// Register the attribute.
        /// </summary>
        /// <param name="attrib"></param>
        public virtual void Register(TAttrib attrib)
        {
            var metadata = new ExtensionMetaData
            {
                Id = attrib.Name,
                Attribute = attrib,
                Instance = null,
                DataType = attrib.DeclaringDataType
            };
            Register(attrib.Name, metadata);
        }


        /// <summary>
        /// Register the id.
        /// </summary>
        /// <param name="extension"></param>
        public virtual void Register(string id, ExtensionMetaData metadata)
        {
            Lookup[id] = metadata;
        }


        /// <summary>
        /// Register the attributes.
        /// </summary>
        /// <param name="attributes"></param>
        public virtual void Register(IList<TAttrib> attributes)
        {
            foreach (var attrib in attributes)
                Register(attrib);
        }


        /// <summary>
        /// Register the attributes.
        /// </summary>
        /// <param name="attributes"></param>
        public virtual void Register(IList<KeyValuePair<Type, TAttrib>> pairs)
        {
            foreach (var pair in pairs)
                Register(pair.Value);
        }
    }
}
