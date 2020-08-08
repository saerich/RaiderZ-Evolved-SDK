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
using System.Collections;



namespace ComLib.UI
{

    public interface IDataTransferUI
    {
        /// <summary>
        /// Transfers data from the control's textboxes' etc.
        /// over to the binding object.
        /// </summary>
        void TransferToDTO();

        /// <summary>
        /// Puts data from the binding object into the 
        /// control's textbox's.
        /// </summary>
        void TransferToUI();
    }



    public enum BindingDataTransferType { TransferToDTO, TransferToUI, None }



	/// <summary>
	/// Interface to enforce binding functionality.
	/// 
	/// This used as a manual way to bind and unbind the
	/// fields in user control to an object which contains
	/// values associated with those controls.
	/// 
	/// </summary>
    public interface IBindableUI<T> : IDataTransferUI
	{	
		/// <summary>
		/// Binds the control to an data structure.
		/// </summary>
		/// <param name="bindingObject"></param>
		void Bind( T bindingObject, BindingDataTransferType transferType );

		
        /// <summary>
		/// Unbinds the control from it's binding object.
		/// </summary>
		/// <returns></returns>
		void UnBind( );


		/// <summary>
		/// Gets the binding object associated with this control.
		/// </summary>
        T GetBindingObject( BindingDataTransferType transferType );

		
        /// <summary>
		/// Determine if this control is bound to a bindingObject.
		/// </summary>
		bool IsBound { get; }        
	}
}
