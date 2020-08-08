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
    public interface IExtensionService<TAttrib, TExtension> where TAttrib : ExtensionAttribute
    {
        /// <summary>
        /// Lookup all the availably dynamically loaded extensions
        /// </summary>
        IDictionary<string, ExtensionMetaData> Lookup { get; }


        /// <summary>
        /// Load all the information tasks available.
        /// </summary>
        /// <param name="assembliesDelimited"></param>
        void Load(string assembliesDelimited);

        /// <summary>
        /// Create the instance of the extension.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        TExtension Create(string name);


        /// <summary>
        /// Register the attribute.
        /// </summary>
        /// <param name="attrib"></param>
        void Register(TAttrib attrib);


        /// <summary>
        /// Register the id.
        /// </summary>
        /// <param name="extension"></param>
        void Register(string id, ExtensionMetaData metadata);


        /// <summary>
        /// Register the attributes.
        /// </summary>
        /// <param name="attributes"></param>
        void Register(IList<TAttrib> attributes);


        /// <summary>
        /// Register the attributes.
        /// </summary>
        /// <param name="attributes"></param>
        void Register(IList<KeyValuePair<Type, TAttrib>> pairs);
    }
}
