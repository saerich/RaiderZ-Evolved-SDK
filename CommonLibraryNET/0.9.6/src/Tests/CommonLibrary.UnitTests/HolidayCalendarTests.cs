using System;
using System.Collections.Generic;
using System.Text;

using NUnit.Framework;

using ComLib;
using ComLib.Calendars;


namespace CommonLibrary.Tests
{
    [TestFixture]
    public class HolidayCalendarTests
    {
        [TestFixtureSetUp]
        public void Setup()
        {
            CalanderDao dao = new CalanderDao();

            // For testing, jan1, july4, christmas.
            List<Holiday> holidays = new List<Holiday>()
            {
                new Holiday(1, 1, true, DayOfWeek.Monday, -1, "New Years"),
                new Holiday(7, 4, true, DayOfWeek.Monday, -1, "Independence Day"),
                new Holiday(12, 25, true, DayOfWeek.Monday, -1, "Christmas Day")
            };
            dao.Load("usa-bronx-holidays", holidays);            
            Calendar.Init("usa-bronx-holidays", dao, DateTime.Today.Year, DateTime.Today.Year + 1);
        }


        [Test]
        public void CanGetNextBusinessDate()
        {
            DateTime busDay = Calendar.NextBusinessDate(new DateTime(DateTime.Today.Year, 1, 1));

            Assert.AreEqual(Calendar.CalendarCode, "usa-bronx-holidays");
            Assert.AreEqual(busDay, new DateTime(DateTime.Today.Year, 1, 3));
        }


        [Test]
        public void CanGetFirstBusinessDateOfYear()
        {
            DateTime busDay = Calendar.FirstBusinessDateOfYear(DateTime.Today.Year);

            Assert.AreEqual(busDay, new DateTime(DateTime.Today.Year, 1, 3));
        }


        [Test]
        public void CanGetFirsttBusinessDateOfMonth()
        {
            DateTime busDay = Calendar.FirstBusinessDateOfMonth(1, DateTime.Today.Year);

            Assert.AreEqual(busDay, new DateTime(DateTime.Today.Year, 1, 3));
        }
    }
}