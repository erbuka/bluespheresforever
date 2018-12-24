using System;
using System.Windows.Forms;

namespace LevelEditor
{
    public partial class AboutForm : Form
    {
        public AboutForm()
        {
            InitializeComponent();

            this.VersionLabel.Text = "Version " + Application.ProductVersion;
        }

        private void Close(object sender, EventArgs e)
        {
            this.Dispose();
        }

    }
}
