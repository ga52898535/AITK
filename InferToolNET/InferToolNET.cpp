#include "pch.h"
#include "InferTool.h"
#include "InferToolNET.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <math.h>


using namespace cv;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
//using namespace std;

AITK::InferToolNET::InferToolNET()
{
	mInfer = new InferTool();
}

AITK::InferToolNET::~InferToolNET()
{
	//throw gcnew System::NotImplementedException();
}

bool AITK::InferToolNET::InitializeTRTModel(System::String^ path)
{
	const char* chars = (const char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(path)).ToPointer();	
	return mInfer->InitializeTRTModel(chars);
}

bool AITK::InferToolNET::onnxToTRTModel(System::String^ inputModelFile, System::String^ outputModelFile)
{
	const char* inputChars = (const char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(inputModelFile)).ToPointer();
	const char* outputChars = (const char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(outputModelFile)).ToPointer();	
	return mInfer->onnxToTRTModel(inputChars, outputChars);
}

bool AITK::InferToolNET::DoInference(Bitmap^ srcBmp, Bitmap^% dstBmp, int batchSize)
{
	Mat resMat, resInferMat;
	Mat srcMat = BitmapToMat(srcBmp);
	cv::Size dSize = cv::Size(128, 128);
	resize(srcMat, resMat, dSize, 0, 0);
	Mat dstMat;
	bool ret= mInfer->DoInference(resMat, dstMat, batchSize);
	dSize = cv::Size(512, 512);
	resize(dstMat, resInferMat, dSize, 0, 0);
	dstBmp = MatToBitmap(resInferMat);
	return ret;
}

Mat AITK::InferToolNET::BitmapToMat(System::Drawing::Bitmap^ bitmap)
{
	Rectangle blank = System::Drawing::Rectangle(0, 0, bitmap->Width, bitmap->Height);
	BitmapData^ bmpdata = bitmap->LockBits(blank, ImageLockMode::ReadWrite, bitmap->PixelFormat);
	Mat cv_img;
	if (bitmap->PixelFormat == PixelFormat::Format24bppRgb)
		cv_img = Mat(cv::Size(bitmap->Width, bitmap->Height), CV_8UC3, bmpdata->Scan0.ToPointer(), cv::Mat::AUTO_STEP);
	else
		cv_img = Mat(cv::Size(bitmap->Width, bitmap->Height), CV_8UC1, bmpdata->Scan0.ToPointer(), cv::Mat::AUTO_STEP);

	bitmap->UnlockBits(bmpdata);

	return cv_img;
}

Bitmap^ AITK::InferToolNET::MatToBitmap(Mat srcImg)
{
	int stride = srcImg.size().width * srcImg.channels();//calc the srtide
	int hDataCount = srcImg.size().height;

	System::Drawing::Bitmap^ retImg;

	System::IntPtr ptr(srcImg.data);

	//create a pointer with Stride
	if (stride % 4 != 0) {//is not stride a multiple of 4?
		//make it a multiple of 4 by fiiling an offset to the end of each row

		uchar* dataPro = new uchar[((srcImg.size().width * srcImg.channels() + 3) & -4) * hDataCount];//to hold processed data

		uchar* data = srcImg.ptr();

		//current position on the data array
		int curPosition = 0;
		//current offset
		int curOffset = 0;

		int offsetCounter = 0;

		//itterate through all the bytes on the structure
		for (int r = 0; r < hDataCount; r++) {
			//fill the data
			for (int c = 0; c < stride; c++) {
				curPosition = (r * stride) + c;

				dataPro[curPosition + curOffset] = data[curPosition];
			}

			//reset offset counter
			offsetCounter = stride;

			//fill the offset
			do {
				curOffset += 1;
				dataPro[curPosition + curOffset] = 0;

				offsetCounter += 1;
			} while (offsetCounter % 4 != 0);
		}

		ptr = (System::IntPtr)dataPro;//set the data pointer to new/modified data array

		//calc the stride to nearest number which is a multiply of 4
		stride = (srcImg.size().width * srcImg.channels() + 3) & -4;

		retImg = gcnew System::Drawing::Bitmap(srcImg.size().width, srcImg.size().height,
			stride,
			System::Drawing::Imaging::PixelFormat::Format24bppRgb,
			ptr);
	}
	else {

		//no need to add a padding or recalculate the stride
		if (srcImg.channels() == 3)
		{
			retImg = gcnew System::Drawing::Bitmap(srcImg.size().width, srcImg.size().height,
				stride,
				System::Drawing::Imaging::PixelFormat::Format24bppRgb,
				ptr);
		}
		else
		{
			retImg = gcnew System::Drawing::Bitmap(srcImg.size().width, srcImg.size().height,
				stride,
				System::Drawing::Imaging::PixelFormat::Format8bppIndexed,
				ptr);
			//// Set gray palette
			ColorPalette^ pal = retImg->Palette;
			int i;
			for (i = 0; i <= 255; i++)
				pal->Entries[i] = System::Drawing::Color::FromArgb(255, i, i, i);
			retImg->Palette = pal;
		}
	}

	cli::array<System::Byte>^ imageData;
	System::Drawing::Bitmap^ output;

	// Create the byte array.
	{
		System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream();
		retImg->Save(ms, System::Drawing::Imaging::ImageFormat::Png);
		imageData = ms->ToArray();
		delete ms;
	}

	// Convert back to bitmap
	{
		System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream(imageData);
		output = (System::Drawing::Bitmap^)System::Drawing::Bitmap::FromStream(ms);
		delete ms;
	}

	return output;
}
