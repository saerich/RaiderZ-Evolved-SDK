using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace NUnit.UiKit
{
	public class TabbedOptionsDialog : NUnit.UiKit.OptionsDialogBase
	{
		private System.Windows.Forms.TabControl tabControl1;
		private System.ComponentModel.IContainer components = null;

		public TabbedOptionsDialog()
		{
			// This call is required by the Windows Form Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitializeComponent call
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.tabControl1 = new System.Windows.Forms.TabControl();
			this.SuspendLayout();
			// 
			// tabControl1
			// 
			this.tabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.tabControl1.ItemSize = new System.Drawing.Size(46, 18);
			this.tabControl1.Location = new System.Drawing.Point(10, 9);
			this.tabControl1.Name = "tabControl1";
			this.tabControl1.SelectedIndex = 0;
			this.tabControl1.Size = new System.Drawing.Size(310, 420);
			this.tabControl1.TabIndex = 2;
			// 
			// TabbedOptionsDialog
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(6, 15);
			this.ClientSize = new System.Drawing.Size(320, 456);
			this.Controls.Add(this.tabControl1);
			this.Name = "TabbedOptionsDialog";
			this.Controls.SetChildIndex(this.tabControl1, 0);
			this.ResumeLayout(false);

		}
		#endregion

		protected void LoadTabPage( TabPage tabPage )
		{
			tabControl1.Controls.Add( tabPage );
		}
	}
}

