using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ComLib
{
    public class EnvironmentVariables
    {
        /// <summary>
        /// Get environment variable from current process, user variable, machine variable.
        /// </summary>
        /// <param name="name">Environment variable name.</param>
        /// <returns></returns>
        public static string GetAny(string name)
        {
            string env = Environment.GetEnvironmentVariable(name, EnvironmentVariableTarget.Process);
            if (string.IsNullOrEmpty(env))
            {
                env = Environment.GetEnvironmentVariable(name, EnvironmentVariableTarget.User);
                if (string.IsNullOrEmpty(env))
                {
                    env = Environment.GetEnvironmentVariable(name, EnvironmentVariableTarget.Machine);
                }
            }
            return env;
        }
    }
}
