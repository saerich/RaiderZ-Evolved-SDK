/*
 * Author: Kishore Reddy
 * Url: http://commonlibrarynet.codeplex.com/
 * Title: CommonLibrary.NET
 * Copyright: � 2009 Kishore Reddy
 * License: LGPL License
 * LicenseUrl: http://commonlibrarynet.codeplex.com/license
 * Description: A C# based .NET 3.5 Open-Source collection of reusable components.
 * Usage: Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using System;
using System.Collections.Generic;
using System.Text;
using System.Web.Security;
using System.Security.Principal;
using System.Collections.ObjectModel;


namespace ComLib.Authentication
{
    public class RoleHelper
    {
        /// <summary>
        /// Is User in the selected roles.
        /// </summary>
        /// <param name="rolesDelimited"></param>
        /// <returns></returns>
        public static bool IsUserInRoles(string rolesDelimited)
        {
            if (string.IsNullOrEmpty(rolesDelimited))
                return false;

            string[] roles = StringHelper.ToStringArray(rolesDelimited, ';');
            foreach (string role in roles)
            {
                if (Roles.IsUserInRole(role))
                    return true;
            }
            return false;
        }


        /// <summary>
        /// Is User in the selected roles.
        /// </summary>
        /// <param name="rolesDelimited"></param>
        /// <returns></returns>
        public static bool IsInRoles(string rolesDelimited, IPrincipal user)
        {
            if (string.IsNullOrEmpty(rolesDelimited))
                return false;

            string[] roles = StringHelper.ToStringArray(rolesDelimited, ';');
            foreach (string role in roles)
            {
                if (user.IsInRole(role))
                    return true;
            }
            return false;
        }
    }
}
