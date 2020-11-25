#pragma once

using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System;

namespace AITK {
	public ref class InferToolNET
	{
	public:
		InferToolNET();
		~InferToolNET();
		bool InitializeTRTModel(System::String^ path);
		bool onnxToTRTModel(System::String^ inputModelFile, System::String^ outputModelFile);
		bool DoInference(Bitmap^ srcBmp, Bitmap^% dstBmp, int batchSize);

	private:
		InferTool* mInfer;
		static cv::Mat BitmapToMat(System::Drawing::Bitmap^ bitmap);
		static Bitmap^ MatToBitmap(cv::Mat img);
	};
}
