using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;


namespace ComLib.CaptchaSupport
{
    public interface ICaptchaGenerator
    {
        System.Drawing.Bitmap Generate(string randomText);
        CaptchaSettings Settings { get; set; }
    }


    public interface ICaptcha : ICaptchaGenerator
    {
        bool IsCorrect();
        bool IsCorrect(string userInput);
        bool IsCorrect(string userInput, string encodedText);

        Bitmap Generate();
        Bitmap GenerateFromUrl();

        string GetRandomText();
        string GetRandomTextEncoded();
    }



    public class Captcha
    {
        private static ICaptcha _captcha = new CaptchaWeb();


        /// <summary>
        /// Initialize captcha provider.
        /// </summary>
        /// <param name="captcha"></param>
        public static void Init(ICaptcha captcha)
        {
            _captcha = captcha;
        }


        /// <summary>
        /// Get random text.
        /// </summary>
        /// <returns></returns>
        public static string GetRandomText()
        {
            return _captcha.GetRandomText();
        }


        /// <summary>
        /// Get encoded random text.
        /// </summary>
        /// <returns></returns>
        public static string GetRandomTextEncoded()
        {
            return _captcha.GetRandomTextEncoded();
        }


        /// <summary>
        /// Determine if the current captcha is correct.
        /// </summary>
        /// <returns></returns>
        public static bool IsCorrect()
        {
            return _captcha.IsCorrect();
        }


        /// <summary>
        /// Determine if the current captcha is correct.
        /// </summary>
        /// <returns></returns>
        public static bool IsCorrect(string userInput)
        {
            return _captcha.IsCorrect(userInput);
        }


        /// <summary>
        /// Determine if the current captcha is correct.
        /// </summary>
        /// <returns></returns>
        public static bool IsCorrect(string userInput, string encodedInput)
        {
            return _captcha.IsCorrect(userInput, encodedInput);
        }


        /// <summary>
        /// Create a new Captcha image.
        /// </summary>
        /// <returns></returns>
        public static System.Drawing.Bitmap Generate()
        {
            return _captcha.Generate();
        }


        /// <summary>
        /// Create a new Captcha image.
        /// </summary>
        /// <returns></returns>
        public static System.Drawing.Bitmap GenerateFromUrl()
        {
            return _captcha.GenerateFromUrl();
        }

        
        /// <summary>
        /// Create a new captcha image using the random text supplied.
        /// </summary>
        /// <param name="randomText"></param>
        /// <returns></returns>
        public static System.Drawing.Bitmap Generate(string randomText)
        {
            return _captcha.Generate(randomText);
        }
    }
}
