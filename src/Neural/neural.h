#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Utils/vectmat.h"

enum CostFunction : char
{
  MSE,
  XENT
};
enum ActivationFunction : char
{
  SIGMOID,
  RELU,
  RELUL,
  IDENTITY
};
enum PredictionType : char
{
  VALUE,
  CATEGORY
};

inline cv::Mat sigmoid(const cv::Mat& z)
{
  cv::Mat result = z.clone();
  for (int r = 0; r < z.rows; r++)
    result.at<float>(r, 0) = 1. / (1 + exp(-z.at<float>(r, 0)));
  return result;
}

inline cv::Mat sigmoidPrime(const cv::Mat& z)
{
  return sigmoid(z).mul(1. - sigmoid(z));
}

inline cv::Mat relu(const cv::Mat& z)
{
  cv::Mat result = z.clone();
  for (int r = 0; r < z.rows; r++)
    result.at<float>(r, 0) = z.at<float>(r, 0) > 0 ? z.at<float>(r, 0) : 0;
  return result;
}

inline cv::Mat reluPrime(const cv::Mat& z)
{
  cv::Mat result = z.clone();
  for (int r = 0; r < z.rows; r++)
    result.at<float>(r, 0) = z.at<float>(r, 0) > 0 ? 1 : 0;
  return result;
}

inline cv::Mat reluLeaky(const cv::Mat& z)
{
  cv::Mat result = z.clone();
  for (int r = 0; r < z.rows; r++)
    result.at<float>(r, 0) = z.at<float>(r, 0) > 0 ? z.at<float>(r, 0) : 0.01 * z.at<float>(r, 0);
  return result;
}

inline cv::Mat reluLeakyPrime(const cv::Mat& z)
{
  cv::Mat result = z.clone();
  for (int r = 0; r < z.rows; r++)
    result.at<float>(r, 0) = z.at<float>(r, 0) > 0 ? 1 : 0.01;
  return result;
}

inline cv::Mat activate(const cv::Mat& z, ActivationFunction& activationFunction)
{
  switch (activationFunction)
  {
  case SIGMOID:
    return sigmoid(z);
  case RELU:
    return relu(z);
  case RELUL:
    return reluLeaky(z);
  case IDENTITY:
    return (z);
  }
}

inline cv::Mat activatePrime(const cv::Mat& z, ActivationFunction& activationFunction)
{
  switch (activationFunction)
  {
  case SIGMOID:
    return sigmoidPrime(z);
  case RELU:
    return reluPrime(z);
  case RELUL:
    return reluLeakyPrime(z);
  case IDENTITY:
    return (z * 0. + 1);
  }
}

inline cv::Mat mse(const cv::Mat& a, const cv::Mat& y)
{
  cv::Mat result;
  pow(a - y, 2, result);
  return result * 0.5;
}

inline cv::Mat msePrime(const cv::Mat& a, const cv::Mat& y)
{
  return a - y;
}

inline cv::Mat crossentropy(const cv::Mat& a, const cv::Mat& y)
{
  cv::Mat loga = a.clone();
  cv::Mat log1a = a.clone();
  log(a, loga);
  log(1. - a, log1a);
  return -(y.mul(loga) + (1 - y).mul(log1a));
}

inline cv::Mat crossentropyPrime(const cv::Mat& a, const cv::Mat& y)
{
  return (1. - y) / (1 - a) - y / a;
}

inline cv::Mat cost(const cv::Mat& a, const cv::Mat& y, CostFunction& costFunction)
{
  switch (costFunction)
  {
  case MSE:
    return mse(a, y);
  case XENT:
    return crossentropy(a, y);
  }
}

inline cv::Mat costPrime(const cv::Mat& a, const cv::Mat& y, CostFunction& costFunction)
{
  switch (costFunction)
  {
  case MSE:
    return msePrime(a, y);
  case XENT:
    return crossentropyPrime(a, y);
  }
}

class NeuralNetwork
{
public:
  ActivationFunction activationFunction = SIGMOID;
  ActivationFunction activationFunctionLast = SIGMOID;
  CostFunction costFunction = MSE;
  PredictionType predictionType = VALUE;
  double batchSizeRatio = 1;
  int trainingEpochs = 1000;
  std::vector<std::vector<double>> trainingInputs;
  std::vector<std::vector<double>> trainingOutputs;
  double learningRate = 0.001;

  NeuralNetwork(int inputSize, std::vector<int> layerSizes) : inputSize(inputSize), layerSizes(layerSizes) { initialize(); }

  std::vector<double> predictValue(std::vector<double>& input)
  {
    cv::Mat input_ = vectToMat(input); // input vector to mat conversion
    auto z = copyVectorMat(b), a = copyVectorMat(b);
    feedForward(input_, z, a);
    return mat1ToVect(a.back()); // return last activation vector
  }

  int predictCategory(std::vector<double>& input)
  {
    cv::Mat input_ = vectToMat(input); // input vector to mat conversion
    auto z = copyVectorMat(b), a = copyVectorMat(b);
    feedForward(input_, z, a);

    cv::Point2i minloc, maxloc;
    double maxR, minR;
    minMaxLoc(a.back(), &minR, &maxR, &minloc, &maxloc); // find the neuron with maximum activation
    return maxloc.y;                                     // return max index
  }

  void train()
  {
    std::vector<cv::Mat> trainingInputs_ = vect2ToMats(trainingInputs);   // input vector to mat conversion
    std::vector<cv::Mat> trainingOutputs_ = vect2ToMats(trainingOutputs); // output vector to mat conversion
    int dataSize = trainingInputs_.size();
    double costFunVal = DBL_MAX; // cost function value

    for (int indexEpoch = 0; indexEpoch < trainingEpochs; indexEpoch++)
    {
      std::vector<cv::Mat> d = copyVectorMat(b);      // errors
      std::vector<cv::Mat> wPrime = copyVectorMat(w); // weight gradient
      std::vector<cv::Mat> bPrime = copyVectorMat(b); // bias gradient
      costFunVal = 0;

#pragma omp parallel for
      for (int indexData = 0; indexData < dataSize; indexData++) // loop over all data
      {
        auto z = copyVectorMat(b), a = copyVectorMat(b);
        feedForward(trainingInputs_[indexData], z, a);

        std::vector<cv::Mat> d_ = copyVectorMat(b);      // errors for 1 data
        std::vector<cv::Mat> wPrime_ = copyVectorMat(w); // weight gradient for 1 data
        std::vector<cv::Mat> bPrime_ = copyVectorMat(b); // bias gradient for 1 data

        for (int L = layerCount - 1; L >= 0; L--) // backpropagate
        {
          ActivationFunction actFun = L == layerCount - 1 ? activationFunctionLast : activationFunction;
          d_[L] = L < (layerCount - 1) ? (w[L + 1].t() * d_[L + 1]).mul(activatePrime(z[L], actFun)) : costPrime(a[L], trainingOutputs_[indexData], costFunction).mul(activatePrime(z[L], actFun));
          bPrime_[L] = d_[L].clone();
          wPrime_[L] = L > 0 ? d_[L] * a[L - 1].t() : d_[L] * trainingInputs_[indexData].t();
        }
#pragma omp critical // update gradient
        {
          d += d_;
          wPrime += wPrime_;
          bPrime += bPrime_;

          cv::Mat kost = cost(a.back(), trainingOutputs_[indexData], costFunction);
          if constexpr (0) // debug
          {
            std::cout << std::endl;
            std::cout << "<training epoch " << indexEpoch << "><data " << indexData << "> z" << indexData << " = " << z.back() << std::endl;
            std::cout << "<training epoch " << indexEpoch << "><data " << indexData << "> a" << indexData << " = " << a.back() << std::endl;
            std::cout << "<training epoch " << indexEpoch << "><data " << indexData << "> y" << indexData << " = " << trainingOutputs_[indexData] << std::endl;
            std::cout << "<training epoch " << indexEpoch << "><data " << indexData << "> kost" << indexData << " = " << kost << std::endl;
          }
          costFunVal += sum(kost)[0];
        }
      }

      if constexpr (0) // debug
      {
        for (int L = 0; L < layerCount; L++)
        {
          std::cout << std::endl;
          std::cout << "<training> "
                    << "weight matrix:" << w[L].size() << std::endl
                    << w[L] << std::endl;
          std::cout << "<training> "
                    << "bias matrix:" << b[L].size() << std::endl
                    << b[L] << std::endl;
        }
      }

      costFunVal /= dataSize;        // average cost per data
      costFunVal /= (w.back().rows); // average cost per neuron
      // perfrom the gradient descent step
      w -= (learningRate / dataSize) * wPrime;
      b -= (learningRate / dataSize) * bPrime;
      if (!((indexEpoch + 1) % 5000))
        learningRate *= 0.9;
      if (!(indexEpoch % 500))
        std::cout << "<training epoch " << indexEpoch << "> costFunVal = " << costFunVal << std::endl;
    }
  }

private:
  int inputSize;
  std::vector<int> layerSizes;
  int layerCount;         // auto
  int outputSize;         // auto
  std::vector<cv::Mat> w; // weights
  std::vector<cv::Mat> b; // biases

  void initialize()
  {
    layerCount = layerSizes.size();
    outputSize = layerSizes.back();
    // allocate all
    w.resize(layerCount);
    b.resize(layerCount);

    int L; // main iterator
    // initialize weights and biases
    for (L = 0; L < layerCount; L++)
    {
      w[L] = L > 0 ? cv::Mat::zeros(layerSizes[L], layerSizes[L - 1], CV_32FC1) : cv::Mat::zeros(layerSizes[L], inputSize, CV_32FC1); // allocate weight matrices
      b[L] = cv::Mat::zeros(layerSizes[L], 1, CV_32FC1);
    }
    for (const cv::Mat& w_ : w)
    {
      randu(w_, -1, 1); // initialize weight matrices randomly
    }
    for (const cv::Mat& b_ : b)
    {
      randu(b_, -1, 1); // initialize biases randomly
    }

    for (int L = 0; L < layerCount; L++) // debug
    {
      std::cout << std::endl;
      std::cout << "<initialization> "
                << "weight matrix:" << w[L].size() << std::endl
                << w[L] << std::endl;
      std::cout << "<initialization> "
                << "bias matrix:" << b[L].size() << std::endl
                << b[L] << std::endl;
    }
  }

  cv::Mat feedForward(const cv::Mat& input, std::vector<cv::Mat>& z, std::vector<cv::Mat>& a)
  {
    for (int L = 0; L < layerCount; L++)
    {
      z[L] = L > 0 ? w[L] * a[L - 1] + b[L] : w[L] * input + b[L];                                // feed forward
      a[L] = activate(z[L], L == (layerCount - 1) ? activationFunctionLast : activationFunction); // activate
    }
    return a[layerCount - 1];
  }

  std::vector<cv::Mat> copyVectorMat(std::vector<cv::Mat>& input, double value = 0)
  {
    std::vector<cv::Mat> output(input.size());
    for (int i = 0; i < input.size(); i++)
    {
      output[i] = 0. * input[i].clone() + value;
    }
    return output;
  }
};
