using System;
using System.Collections.Generic;
using System.Text;
using System.Security.Cryptography;

using ComLib;
using ComLib.Cryptography;
using NUnit.Framework;


namespace CommonLibrary.Tests
{
    [TestFixture]
    public class Cryptography
    {        
        [Test]
        public void CanEncryptSymmetricTripleDes()
        {
            ICrypto crypto = new CryptoSym("commonlib.net", new TripleDESCryptoServiceProvider());
            string encrypted = crypto.Encrypt("horizonguy");
            Assert.AreNotEqual("horizonguy", encrypted);
        }


        [Test]
        public void IsEncryptSymmetricTripleDesDifferentByInput()
        {
            ICrypto crypto = new CryptoSym("commonlib.net", new TripleDESCryptoServiceProvider());
            string encrypted = crypto.Encrypt("horizonguy");
            string encrypted2 = crypto.Encrypt("bourneIdentity");
            Assert.AreNotEqual("horizonguy", encrypted);
            Assert.AreNotEqual(encrypted, encrypted2);
        }


        [Test]
        public void CanDecryptSymmetricTripleDes()
        {
            string encrypted = "OTrZQMzbEM2QTfH7vJyaDg==";
            ICrypto crypto = new CryptoSym("commonlib.net", new TripleDESCryptoServiceProvider());
            string decrypted = crypto.Decrypt(encrypted);
            Assert.AreEqual("horizonguy", decrypted);
        }


        [Test]
        public void CanEncryptHash()
        {
            string plainText = "horizonguy";
            ICrypto crypto = new CryptoHash("commonlib.net", new MD5CryptoServiceProvider());
            string encrypted = crypto.Encrypt(plainText);
            Assert.AreNotEqual("horizonguy", encrypted);
        }


        [Test]
        public void CanDecryptHash()
        {
            string encrypted = "44bf9ba3479261e365c6389bc03bf497";
            ICrypto crypto = new CryptoHash("commonlib.net", new MD5CryptoServiceProvider());
            bool ismatch = crypto.IsMatch(encrypted, "horizonguy");
            Assert.AreEqual(ismatch, true);
        }


        [Test]
        public void CanEncryptWithSpecialCharsTripleDes()
        {
            string plainText = "~`!@#$%^&*()_+{}|:\"<>?[]\\,./;'-=";
            ICrypto crypto = new CryptoSym("commonlib.net", new TripleDESCryptoServiceProvider());
            string encrypted = crypto.Encrypt(plainText);

            Assert.AreNotEqual(plainText, encrypted);

            // Now decrypt.
            string decrypted = crypto.Decrypt(encrypted);
            Assert.AreEqual("~`!@#$%^&*()_+{}|:\"<>?[]\\,./;'-=", decrypted);
        }
    }
}
