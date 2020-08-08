using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Reflection;
using System.Resources;
using NUnit.Framework;

using ComLib;
using ComLib.Extensions;

namespace CommonLibrary.Tests
{


    [TestFixture]
    public class ExtensionsTests
    {
        [Test]
        public void CanParseStringAsBool()
        {
            Assert.AreEqual(StringExtensions.ToBool("yes"), true);
            Assert.AreEqual(StringExtensions.ToBool("true"), true);
            Assert.AreEqual(StringExtensions.ToBool("YES"), true);
            Assert.AreEqual(StringExtensions.ToBool("TrUe"), true);
            Assert.AreEqual(StringExtensions.ToBool("1"), true);
            Assert.AreEqual(StringExtensions.ToBool("no"), false);
            Assert.AreEqual(StringExtensions.ToBool("false"), false);
            Assert.AreEqual(StringExtensions.ToBool("0"), false);
        }


        [Test]
        public void CanParseStringAsIntWithMoney()
        {
            Assert.AreEqual(StringExtensions.ToInt("100"), 100);
            Assert.AreEqual(StringExtensions.ToInt("$100"), 100);
            Assert.AreEqual(StringExtensions.ToInt(" 100"), 100);
            Assert.AreEqual(StringExtensions.ToInt(" $100 "), 100);
            Assert.AreEqual(StringExtensions.ToInt(" 100.55"), 101);
            Assert.AreEqual(StringExtensions.ToInt(" $100.55 "), 101);
            Assert.AreEqual(StringExtensions.ToInt(" $100.35 "), 100);
        }


        [Test]
        public void CanParseStringAsDoubleWithMoney()
        {
            Assert.AreEqual(StringExtensions.ToDouble("101.5"), 101.5);
            Assert.AreEqual(StringExtensions.ToDouble("$101"), 101);
            Assert.AreEqual(StringExtensions.ToDouble(" 101"), 101);
            Assert.AreEqual(StringExtensions.ToDouble(" $101 "), 101);
            Assert.AreEqual(StringExtensions.ToDouble("101"), 101);
            Assert.AreEqual(StringExtensions.ToDouble("$101"), 101);
            Assert.AreEqual(StringExtensions.ToDouble(" 101.55"), 101.55);
            Assert.AreEqual(StringExtensions.ToDouble(" $101.55 "), 101.55);
            Assert.AreEqual(StringExtensions.ToDouble(" $101.35 "), 101.35);
        }

        
        [Test]
        public void CanParseStringAsDate()
        {
            Assert.AreEqual(StringExtensions.ToDateTime("${t}").Date, DateTime.Today.Date);
            Assert.AreEqual(StringExtensions.ToDateTime("${t-1}").Date, DateTime.Today.AddDays(-1).Date);
            Assert.AreEqual(StringExtensions.ToDateTime("${t-4}").Date, DateTime.Today.AddDays(-4).Date);
            Assert.AreEqual(StringExtensions.ToDateTime("${t+1}").Date, DateTime.Today.AddDays(1).Date);
            Assert.AreEqual(StringExtensions.ToDateTime("${t+4}").Date, DateTime.Today.AddDays(4).Date);
            Assert.AreEqual(StringExtensions.ToDateTime("${today}").Date, DateTime.Today.Date);
            Assert.AreEqual(StringExtensions.ToDateTime("${yesterday}").Date, DateTime.Today.AddDays(-1).Date);
            Assert.AreEqual(StringExtensions.ToDateTime("${tommorrow}").Date, DateTime.Today.AddDays(1).Date);
        }


        [Test]
        public void CanParseStringAsTime()
        {
            Assert.AreEqual(StringExtensions.ToTime("9"), new TimeSpan(9, 0, 0)); 
            Assert.AreEqual(StringExtensions.ToTime("9am"), new TimeSpan(9, 0, 0));
            Assert.AreEqual(StringExtensions.ToTime("9pm"), new TimeSpan(21, 0, 0));
            Assert.AreEqual(StringExtensions.ToTime("9 am"), new TimeSpan(9, 0, 0));
            Assert.AreEqual(StringExtensions.ToTime("9 pm"), new TimeSpan(21, 0, 0));
            Assert.AreEqual(StringExtensions.ToTime("9:35"), new TimeSpan(9, 35, 0));
            Assert.AreEqual(StringExtensions.ToTime("9:35am"), new TimeSpan(9, 35, 0));
            Assert.AreEqual(StringExtensions.ToTime("9:35pm"), new TimeSpan(21, 35, 0));
            Assert.AreEqual(StringExtensions.ToTime("9:35 am"), new TimeSpan(9, 35, 0));
            Assert.AreEqual(StringExtensions.ToTime("9:35 pm"), new TimeSpan(21, 35, 0));
            Assert.AreEqual(StringExtensions.ToTime("12am"), new TimeSpan(0, 0, 0));
            Assert.AreEqual(StringExtensions.ToTime("12pm"), new TimeSpan(12, 0, 0));
        }

        [Test]
        public void Levenshtein()
        {
            // Example with the same string
            var same = "I am a string";
            var sameResult = StringExtensions.Levenshtein(same, same);
            Console.WriteLine("Difference for same string: {0}", sameResult);
            Assert.AreEqual(0, sameResult, "Both strings are equal - " +
                "the Levenshtein algorithm should return 0");

            // Wikipedia example
            var result = StringExtensions.Levenshtein("kitten", "sitting");
            Console.WriteLine("Difference 'for kitten' and 'sitting': {0}", result);
            Assert.AreEqual(3, result, "Unexpected Levenshtein value for strings");
        }

        [Test]
        public void SimplifiedSoundex()
        {
            /* Test to see if soundex values correspond to known trouble values
             * as published on the web. */
            Assert.AreEqual("R163", "Robert".SimplifiedSoundex(4));
            Assert.AreEqual("B625", "Baragwanath".SimplifiedSoundex(4));
            Assert.AreEqual("D540", "Donnell".SimplifiedSoundex(4));
            Assert.AreEqual("L300", "Lloyd".SimplifiedSoundex(4));
            Assert.AreEqual("W422", "Woolcock".SimplifiedSoundex(4));
        }

        #region DecimalExtensions
        [Test]
        public void DigitAtPosition()
        {
            const decimal number = 328.24568015823m;
            var decimalArray = "24568015823".ToCharArray();

            for (var i = 0; i < decimalArray.Length; i++)
            {
                var expected = decimalArray[i].ToString();
                var actual = number.DigitAtPosition(i + 1).ToString();
                Assert.AreEqual(expected, actual,
                    "Decimal value at position was incorrect");
            }
        }

        [Test]
        public void Pow()
        {
            Assert.AreEqual(16m, 4m.Pow(2), "4^2 should equal 16");
            Assert.AreEqual(0.25m, 4m.Pow(-1), "4^-1 should equal 1/4");
            Assert.AreEqual(1237940039285380274899124224m, 1024m.Pow(9),
                "1024^9 should equal 1237940039285380274899124224");
            Assert.AreEqual(2m, 16m.Pow(0.25m), "16^0.25 should equal 2");
        }
        #endregion
    }
}
