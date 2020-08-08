using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Web.Security;


namespace ComLib.Account
{
    public interface IAccountService
    {
        BoolMessage Approve(string userName);
        BoolMessage Create(User user, string password, ref MembershipCreateStatus status);
        BoolMessageItem<User> Create(string userName, string email, string password, string roles);
        BoolMessageItem<User> Create(string userName, string email, string password, string roles, ref MembershipCreateStatus status);
        BoolMessage LogOn(string userName, string password, bool rememberUser);
        BoolMessageItem<User> VerifyUser(string userName, string password);
        User Get(string userName);
        BoolMessage ChangePassword(string userName, string currentPassword, string newPassword);
        void LockOut(string userName, string lockOutReason);
        void UndoLockOut(string userName, string comment);
    }
}
