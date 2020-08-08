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
using ComLib.Cryptography;
using ComLib.Application;


namespace ComLib.Samples
{
    /// <summary>
    /// Example of ActiveRecord Initialization/Configuration.
    /// </summary>
    public class Example_Cryptography : App
    {

        /// <summary>
        /// Run the application.
        /// </summary>
        public override BoolMessageItem Execute()
        {
            // 1. Encrypt using default provider. ( Symmetric TripleDes )
            string plainText = "www.knowledgedrink.com";
            string encrypted = Crypto.Encrypt(plainText);
            string decrypted = Crypto.Decrypt(encrypted);

            Console.WriteLine("====================================================");
            Console.WriteLine("CRYPTOGRAPHY ");
            Console.WriteLine("Encrypted : " + plainText + " to " + encrypted);
            Console.WriteLine("Decrypted : " + encrypted + " to " + decrypted);
            Console.WriteLine(Environment.NewLine);

            // 2. Use non-static encryption provider.
            ICrypto crypto = new CryptoHash("commonlib.net", new MD5CryptoServiceProvider());
            string hashed = crypto.Encrypt("my baby - 2002 honda accord ex coupe");
            Console.WriteLine(hashed);

            // 3. Change the crypto provider on the static helper.
            ICrypto crypto2 = new CryptoSym("new key", new TripleDESCryptoServiceProvider());
            Crypto.Init(crypto2);
            string encryptedWithNewKey = Crypto.Encrypt("www.knowledgedrink.com");
            Console.WriteLine(string.Format("Encrypted text : using old key - {0}, using new key - {1}", encrypted, encryptedWithNewKey));
            return BoolMessageItem.True;
        }
    }
}
