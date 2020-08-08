using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;

using ComLib.Reflection;


namespace ComLib.Information
{
    /// <summary>
    /// Information Service
    /// </summary>
    public interface IInformationService : IExtensionService<InfoAttribute, IInformation>
    {
        /// <summary>
        /// Get the information task with the specified name.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        IInformation Create(string name);


        /// <summary>
        /// Gets the information task after validating that the user has access to it.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="authenticate"></param>
        /// <returns></returns>
        BoolMessageItem<KeyValue<InfoAttribute, IInformation>> GetInfoTask(string name, bool authenticate, Func<string, bool> authenticator);
    }
}
