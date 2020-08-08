using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Web;
using System.Drawing;
using System.Drawing.Imaging;


namespace ComLib.CaptchaSupport
{
    /// <summary>
    /// Captcha Image Handler
    /// </summary>
    public class CaptchaImageHandler : IHttpHandler
    {
        /// <summary>
        /// IsReusable handler.
        /// </summary>
        public bool IsReusable
        {
            get { return false; }
        }


        /// <summary>
        /// Generate a captcha image
        /// </summary>
        /// <param name="context"></param>
        public void ProcessRequest(HttpContext context)
        {
            if (context.Request.Params["CaptchaText"] != null)
            {
                // string name = context.Request.Params["CaptchaText"];
                Bitmap bmp = Captcha.GenerateFromUrl();
                context.Response.Clear();
                context.Response.ContentType = "image/jpeg";
                bmp.Save(context.Response.OutputStream, ImageFormat.Jpeg);
                bmp.Dispose();
            }
        }
    }
}
