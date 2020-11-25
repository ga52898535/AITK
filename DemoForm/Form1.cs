using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Collections;
using AITK;
using System.Drawing.Imaging;

namespace DemoForm
{
    public partial class Form1 : Form
    {
        InferToolNET mInfer = new InferToolNET();
        Bitmap mImg = null;
        
        public Form1()
        {
            InitializeComponent();
        }

        private void btnImportImage_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            if (ofd.ShowDialog() == DialogResult.OK)
            {
                mImg = new Bitmap(ofd.FileName);
                picOrigin.Image = mImg;
            }
        }

        private void LOG(string message)
        {
            string zzz = "";
            txtCommand.AppendText(message + "\r\n");
        }

        private void btnInitTRTModel_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            if(ofd.ShowDialog()==DialogResult.OK)
            {
                LOG("Initial Model: " + ofd.FileName);
                mInfer.InitializeTRTModel(ofd.FileName);
                LOG("Initial successful!");

            }
        }

        private void btnDoInfer_Click(object sender, EventArgs e)
        {
            Bitmap dstBmp = null;
            mInfer.DoInference(mImg,ref dstBmp, 1);
            picResult.Image = dstBmp;
        }
    }
}
