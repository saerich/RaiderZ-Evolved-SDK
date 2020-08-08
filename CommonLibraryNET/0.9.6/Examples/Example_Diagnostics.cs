using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Data;
using System.Data.Common;
using System.Security.Cryptography;

using ComLib.Entities;
using ComLib.Account;
using ComLib;
using ComLib.Diagnostics;
using ComLib.Application;


namespace ComLib.Samples
{
    /// <summary>
    /// Example of ActiveRecord Initialization/Configuration.
    /// </summary>
    public class Example_Diagnostics : App
    {
        /// <summary>
        /// Initialize.
        /// </summary>
        /// <param name="args"></param>
        public Example_Diagnostics()
        {
        }


        /// <summary>
        /// Run the application.
        /// </summary>
        public override BoolMessageItem Execute()
        {
            // 1. Write out the machine information and loaded dlls.
            Diagnostics.Diagnostics.WriteInfo("MachineInfo,AppDomain", "Diagnostics_MachineInfo_DllsLoaded.txt");

            // 2. Write out the environment variables.
            Diagnostics.Diagnostics.WriteInfo("Env_System,Env_User", "Diagnostics_EnvironmentVars.txt");

            Console.WriteLine("Wrote diagnostic data to Diagnostics_MachineInfo_DllsLoaded.txt and Diagnostics_EnvironmentVars.txt");
            return BoolMessageItem.True;
        }
    }
}
