using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using NUnit.Framework;

using ComLib;
using ComLib.Extensions;
using ComLib.Macros;



namespace CommonLibrary.Tests.MacroTests
{

    [Macro(Name = "hello", DisplayName = "Widget", Description = "Renders a hello world macro", IsReusable = true)]
    public class HelloWorldMacro : IMacro
    {
        /// <summary>
        /// Process the tag.
        /// </summary>
        /// <param name="tag"></param>
        /// <returns></returns>
        public string Process(Tag tag)
        {
            return "hello name: " + tag.Attributes["name"].ToString() + ", message: " + tag.InnerContent;
        }
    }

    
    [TestFixture]
    public class MacroServiceTests
    {

        [Test]
        public void CanLoad()
        {
            var service = new MacroService();
            service.Load("CommonLibrary.Tests");
            var macro = service.Create("hello");

            Assert.IsTrue(service.Lookup.Count == 1);
            Assert.IsNotNull(macro);
        }


        [Test]
        public void CanProcess()
        {
            var service = new MacroService("$", "[", "]");
            service.Load("CommonLibrary.Tests");

            var result = service.BuildContent(@"testing $[hello name=""kishore""]commonlibrary.net.cms[/hello]");
            Assert.AreEqual(result, "hello name: kishore, message: commonlibrary.net.cms");
        }
    }
}
