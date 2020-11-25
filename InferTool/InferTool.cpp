#include "pch.h"
#include "InferTool.h"
#include <iostream>
#include <fstream>

class Logger :public ILogger
{
    void log(Severity severity, const char* msg) override
    {
        if (severity == ILogger::Severity::kERROR)
        {
            cout << msg << endl;
        }
    }
} mLogger;


InferTool::InferTool()
{
}

InferTool::~InferTool()
{
}

bool InferTool::InitializeTRTModel(string path)
{
    string buffer;
    ifstream ifs(path.c_str(), ios::binary);
    ifs >> noskipws;
    copy(istream_iterator<char>(ifs), istream_iterator<char>(), back_inserter(buffer));
    IRuntime* runtime = createInferRuntime(mLogger);
    mEngine = runtime->deserializeCudaEngine(buffer.data(), buffer.size(), nullptr);
    mContext = mEngine->createExecutionContext();
    runtime->destroy();
    return true;
}

bool InferTool::onnxToTRTModel( string inputModelFile, string outputModelFile)
{
    IBuilder* builder = createInferBuilder(mLogger);
    nvinfer1::INetworkDefinition* network = builder->createNetwork();

    auto parser = nvonnxparser::createParser(*network, mLogger);

    string modelPath = inputModelFile;
    parser->parseFromFile(modelPath.c_str(), 0);

    builder->setMaxBatchSize(1);
    builder->setMaxWorkspaceSize(1 << 20);
    builder->setFp16Mode(false);
    builder->setInt8Mode(false);


    ICudaEngine* engine = builder->buildCudaEngine(*network);
    IHostMemory* trtModelStream = engine->serialize();
    ofstream ofs(outputModelFile, ios::out | ios::binary);
    ofs.write((char*)trtModelStream->data(), trtModelStream->size());
    ofs.close();

    parser->destroy();
    trtModelStream->destroy();
    engine->destroy();
    network->destroy();
    builder->destroy();

    return true;
}


bool InferTool::DoInference(Mat srcMat, Mat& dstMat, int batchSize)
{
    float* input= Mat2FloatArr(srcMat);
    int inputSize = srcMat.cols * srcMat.rows;
    int outputSize = srcMat.cols * srcMat.rows;
    float* output = new float[outputSize];

    const ICudaEngine& engine = mContext->getEngine();
    void* buffers[2];
    int inputIndex{}, outputIndex{};
    for (int i = 0; i < engine.getNbBindings(); i++)
    {
        if (engine.bindingIsInput(i))
            inputIndex = i;
        else
            outputIndex = i;
    }

    cudaMalloc(&buffers[inputIndex], batchSize * inputSize * sizeof(float));
    cudaMalloc(&buffers[outputIndex], batchSize * outputSize * sizeof(float));

    cudaStream_t stream;
    cudaStreamCreate(&stream);

    cudaMemcpyAsync(buffers[inputIndex], input, batchSize * inputSize * sizeof(float), cudaMemcpyHostToDevice, stream);
    mContext->enqueue(batchSize, buffers, stream, nullptr);
    cudaMemcpyAsync(output, buffers[outputIndex], batchSize * outputSize * sizeof(float), cudaMemcpyDeviceToHost, stream);
    cudaStreamSynchronize(stream);

    dstMat = FloatArr2Mat(output, srcMat.cols, srcMat.rows);

    cudaStreamDestroy(stream);
    cudaFree(buffers[inputIndex]);
    cudaFree(buffers[outputIndex]);

    return true;
}


float* InferTool::Mat2FloatArr(Mat img)
{
    uchar* imgByte = img.data;
    int width = img.cols;
    int height = img.rows;
    float* imgFloat = new float[width * height];
    for (int i = 0; i < width * height; i++)
        imgFloat[i] = imgByte[i * img.channels()] / 255.0;

    return imgFloat;
}

Mat InferTool::FloatArr2Mat(float* imgF, int width, int height)
{
    uchar* imgB = new uchar[width * height];
    for (int i = 0; i < width * height; i++)
        imgB[i] = uchar(imgF[i] * 255);
    Mat imgM = Mat(height, width, CV_8U, imgB);

    return imgM;
}
