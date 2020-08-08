using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Data;
using System.Data.Common;
using System.Security.Cryptography;
using System.Security.Principal;

using ComLib.Entities;
using ComLib.Account;
using ComLib;
using ComLib.Application;
using ComLib.Authentication;


namespace ComLib.Samples
{
    /// <summary>
    /// Example of ActiveRecord Initialization/Configuration.
    /// </summary>
    public class Example_ToDo : App
    {

        /// <summary>
        /// Run the application.
        /// </summary>
        public override BoolMessageItem Execute()
        {   
            // The ToDo logs all the information
            // 1. priority
            // 2. author
            // 3. description
            // 4. STACK TRACE to the log.
            // 
            // NOTE: This is very useful alternative to using comments in the code.
            //       Plus this logs the stack trace(Class and Methodname) where the ToDo.MethodXXX was called from 
            //       so you know from the logs exactly where some work needs done.

            // Example 1: Log code review.
            ToDo.CodeReview(ToDo.Priority.Normal, "Kishore", "Get john to review this code", ()=>
            {
                Console.WriteLine("My code needs review");
            });

            // Example 2: Log some thing that needs to be implemented.
            ToDo.Implement(ToDo.Priority.High, "Kishore", "Have to finish up the Repository.Find to use dynamic objects.", () =>
            {
                Console.WriteLine("DynamicObject find code is incomplete.");
            });


            // Example 3: Log refactoring needed
            ToDo.Refactor(ToDo.Priority.Low, "Kishore", "What a mess in the Reflection utils.", () =>
            {
                Console.WriteLine("This code is a mess, needs some refactoring.");
            });


            // Example 4: Log optimzation needed.
            ToDo.Optimize(ToDo.Priority.Critical, "Kishore", "Multi-tenant code needs to queried from database only once.", () =>
            {
                Console.WriteLine("Too many queries here. Get all data in one call.");
            });

            // Example 5: Log bug found in code.
            ToDo.BugFix(ToDo.Priority.High, "Kishore", "This is a bug related to configuration data. ", () =>
            {
                Console.WriteLine("Fix this code");
            });

            return BoolMessageItem.True;
        }
    }
}
