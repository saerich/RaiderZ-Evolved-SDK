using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace NUnit.UiKit
{
	/// <summary>
	/// Summary description for OptionsDialogBase.
	/// </summary>
	public class OptionsDialogBase : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Button cancelButton;
		private System.Windows.Forms.Button okButton;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public OptionsDialogBase()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.cancelButton = new System.Windows.Forms.Button();
			this.okButton = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// cancelButton
			// 
			this.cancelButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.cancelButton.CausesValidation = false;
			this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.cancelButton.Location = new System.Drawing.Point(174, 432);
			this.cancelButton.Name = "cancelButton";
			this.cancelButton.Size = new System.Drawing.Size(68, 23);
			this.cancelButton.TabIndex = 18;
			this.cancelButton.Text = "Cancel";
			// 
			// okButton
			// 
			this.okButton.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.okButton.Location = new System.Drawing.Point(78, 432);
			this.okButton.Name = "okButton";
			this.okButton.Size = new System.Drawing.Size(76, 23);
			this.okButton.TabIndex = 17;
			this.okButton.Text = "OK";
			// 
			// OptionsDialogBase
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(6, 15);
			this.ClientSize = new System.Drawing.Size(320, 456);
			this.Controls.Add(this.cancelButton);
			this.Controls.Add(this.okButton);
			this.Name = "OptionsDialogBase";
			this.Text = "OptionsDialogBase";
			this.ResumeLayout(false);

		}
		#endregion
	}
}
