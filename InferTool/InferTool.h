#pragma once

#include "cuda_runtime_api.h"
#include "NvInfer.h"
#include "NvOnnxParser.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace nvinfer1;
using namespace cv;
using namespace std;

class EXPORT_DLL InferTool
{
public:
    InferTool();
    ~InferTool();
    bool InitializeTRTModel(string path);
    bool onnxToTRTModel( string inputModelFile, string outputModelFile);
    bool DoInference(Mat srcMat, Mat& dstMat, int batchSize);

private:
    ICudaEngine* mEngine;
    IExecutionContext* mContext;
    float* Mat2FloatArr(Mat img);
    Mat FloatArr2Mat(float* imgF, int width, int height);
};

