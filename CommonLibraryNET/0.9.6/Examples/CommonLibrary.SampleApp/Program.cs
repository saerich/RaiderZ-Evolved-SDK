using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Data;
using System.Data.Common;
using ComLib;
using ComLib.Application;
using ComLib.Logging;
using ComLib.CsvParse;


namespace ComLib.Samples
{
    public class SampleAppRunner
    {        
        /// <summary>
        /// Sample application runner.
        /// </summary>
        /// <param name="args"></param>
        static void Main(string[] args)
        {
            IApp app = new Example_ActiveRecord();
            try
            {       
                if (!app.Accept(args)) return;

                app.Init();
                app.Execute();
                app.ShutDown();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error : " + ex.Message);
                Console.WriteLine("Error : " + ex.StackTrace);
            }

            Console.WriteLine("Finished... Press enter to exit.");
            Console.ReadKey();
        }


        /// <summary>
        /// Sample application runner.
        /// Does pretty much the same thing as the above.
        /// But the above is shown just to display the API/Usage of ApplicationTemplate.
        /// </summary>
        /// <param name="args"></param>
        static void RunWithTemplateCall(IApp app, string[] args)
        {
            App.Run(app, args);
        }
    }
}
