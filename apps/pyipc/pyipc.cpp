#include <utility>
#include "pybind11/pybind11.h"
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "ImageRegistration/IPC.hpp"

namespace py = pybind11;
using namespace pybind11::literals;

std::pair<f64, f64> calculate(
    py::array_t<double> image1, py::array_t<double> image2, double bpL, double bpH, size_t L2size, size_t L2Usize, double L1ratio, size_t maxIter, double CPeps)
{
  if (image1.ndim() != 2)
    throw std::runtime_error("Only 2D images are supported");
  const auto rows = static_cast<i32>(image1.shape()[0]);
  const auto cols = static_cast<i32>(image1.shape()[1]);

  cv::Mat image1cv(rows, cols, CV_64F, const_cast<double*>(image1.data()));
  cv::Mat image2cv(rows, cols, CV_64F, const_cast<double*>(image2.data()));

  IPC ipc(image1.size(), bpL, bpH);
  ipc.SetL2size(L2size);
  ipc.SetL2Usize(L2Usize);
  ipc.SetL1ratio(L1ratio);
  ipc.SetMaxIterations(maxIter);
  ipc.SetCrossPowerEpsilon(CPeps);
  const auto shift = ipc.Calculate(image1cv, image2cv);
  return {shift.x, shift.y};
}

PYBIND11_MODULE(pyipc, m)
{
  m.doc() = "Iterative phase correlation module";
  m.def("calculate", &calculate, "Calculate the subpixel shift between two images", "image1"_a, "image2"_a, "bpL"_a = 0.01, "bpH"_a = 1.0, "L2size"_a = 7, "L2Usize"_a = 357,
      "L1ratio"_a = 0.45, "maxIter"_a = 10, "CPeps"_a = 0);
}
