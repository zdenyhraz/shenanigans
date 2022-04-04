# Zdeny's shenanigans 🪐

<p align="center">
<a href="https://sonarcloud.io/summary/new_code?id=zdenyhraz_shenanigans"><img src="https://sonarcloud.io/images/project_badges/sonarcloud-black.svg"/></a>
<br>
<a href="https://github.com/zdenyhraz/shenanigans/actions/workflows/build.yml"><img src="https://github.com/zdenyhraz/shenanigans/actions/workflows/build.yml/badge.svg"/></a>
<a href="https://github.com/zdenyhraz/shenanigans/actions/workflows/sonarcloud.yml"><img src="https://github.com/zdenyhraz/shenanigans/actions/workflows/sonarcloud.yml/badge.svg"/></a>
<a href="https://github.com/zdenyhraz/shenanigans/actions/workflows/cppcheck.yml"><img src="https://github.com/zdenyhraz/shenanigans/actions/workflows/cppcheck.yml/badge.svg"/></a>
<a href="https://github.com/zdenyhraz/shenanigans/actions/workflows/cpplint.yml"><img src="https://github.com/zdenyhraz/shenanigans/actions/workflows/cpplint.yml/badge.svg"/></a>
<br>
<a href="https://sonarcloud.io/summary/new_code?id=zdenyhraz_shenanigans"><img src="https://sonarcloud.io/api/project_badges/measure?project=zdenyhraz_shenanigans&metric=alert_status"/></a>
<a href="https://www.codacy.com/gh/zdenyhraz/shenanigans/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=zdenyhraz/shenanigans&amp;utm_campaign=Badge_Grade"><img src="https://app.codacy.com/project/badge/Grade/ea68f108539b4e4eb13b0e92a905ef50"/></a>
<a href="https://www.codefactor.io/repository/github/zdenyhraz/shenanigans/overview/master"><img src="https://www.codefactor.io/repository/github/zdenyhraz/shenanigans/badge/master"/></a>
<a href="https://sonarcloud.io/summary/new_code?id=zdenyhraz_shenanigans"><img src="https://sonarcloud.io/api/project_badges/measure?project=zdenyhraz_shenanigans&metric=sqale_rating"/></a>
<a href="https://sonarcloud.io/summary/new_code?id=zdenyhraz_shenanigans"><img src="https://sonarcloud.io/api/project_badges/measure?project=zdenyhraz_shenanigans&metric=security_rating"/></a>
<br>
<a href="https://github.com/zdenyhraz/shenanigans/commits/master"><img src="https://img.shields.io/github/last-commit/zdenyhraz/shenanigans"></a>
<a href="https://github.com/zdenyhraz/shenanigans/commits/master"><img src="https://img.shields.io/github/commit-activity/m/zdenyhraz/shenanigans"></a>
<a href="https://github.com/zdenyhraz/shenanigans/search?l=c%2B%2B"><img src="https://img.shields.io/github/languages/top/zdenyhraz/shenanigans"></a>
<a href="https://sonarcloud.io/summary/new_code?id=zdenyhraz_shenanigans"><img src="https://sonarcloud.io/api/project_badges/measure?project=zdenyhraz_shenanigans&metric=ncloc"></a>
<a href="https://github.com/zdenyhraz/shenanigans/issues"><img src="https://img.shields.io/github/issues-raw/zdenyhraz/shenanigans"></a>
</p>



Semi-random funky stuff, mainly for my PhD. Contains calculations and algorithm implementations for various applied mathematics / astrophysics articles I worked on - [Solar Dynamics Observatory](https://www.nasa.gov/mission_pages/sdo/main/index.html) image processing, image registration, non-convex optimization. With C++ [ImGui](https://github.com/ocornut/imgui) GUI, #uses [C++20](https://en.cppreference.com/w/cpp/20), [OpenCV](https://opencv.org/), [OpenMP](https://www.openmp.org/), [CUDA](https://developer.nvidia.com/cuda-toolkit), [Matplotlib](https://matplotlib.org/), [torch](https://pytorch.org/), [astropy](https://github.com/astropy/astropy), [spdlog](https://github.com/gabime/spdlog), [flamegraph](https://github.com/brendangregg/FlameGraph), [fmt](https://fmt.dev/latest/index.html), [googletest](https://github.com/google/googletest), [json](https://github.com/nlohmann/json), [optick](https://github.com/bombomby/optick), [pybind11](https://github.com/pybind/pybind11), [QCustomPlot](https://www.qcustomplot.com/), [tracy](https://github.com/wolfpld/tracy), [glad](https://glad.dav1d.de/), [GLFW](https://www.glfw.org/), [ImGui](https://github.com/ocornut/imgui). ***Examples below.***

## Article links
- 📌 [Iterative Phase Correlation Algorithm for High-precision Subpixel Image Registration](https://iopscience.iop.org/article/10.3847/1538-4365/ab63d7)
- 🪐 [Measuring Solar Differential Rotation with an Iterative Phase Correlation Method](https://iopscience.iop.org/article/10.3847/1538-4365/abc702)

## The [ImGui](https://github.com/ocornut/imgui/) GUI
<img src="data/readme/gui.png" width="100%">

## Subpixel [image registration](https://en.wikipedia.org/wiki/Image_registration) via [Iterative Phase Correlation](https://iopscience.iop.org/article/10.3847/1538-4365/ab63d7) - gradual 2px shift via [bilinear interpolation](https://en.wikipedia.org/wiki/Bilinear_interpolation) of a 128x128 image (standard vs [IPC](https://iopscience.iop.org/article/10.3847/1538-4365/ab63d7))
<img src="data/dissertation/refinement/peakshift_L1B.gif" width="50%"><img src="data/dissertation/refinement/peakshift_L1A.gif" width="50%">

## [Solar photosphere](https://en.wikipedia.org/wiki/Photosphere) [differential rotation](https://en.wikipedia.org/wiki/Differential_rotation) measurements from [SDO/HMI](http://hmi.stanford.edu/) continuum images
<p align="center">
<img src="data/articles/diffrot/pics/gif/1.gif" width="43%">&nbsp; &nbsp; &nbsp; &nbsp; &nbsp;<img src="data/articles/diffrot/pics/gif/2.gif" width="43%">
</p>

## Visualization of [optimizer](https://en.wikipedia.org/wiki/Mathematical_optimization) progress on a given objective function landscape
<img src="data/articles/optim/rosenbrock_paths.png" width="100%">

## Visualization of *meta*-[optimizer](https://en.wikipedia.org/wiki/Mathematical_optimization) progress and optimization improvement on a given *meta*-objective function landscape
<img src="data/articles/optim/metaopt_paths.png" width="100%">

## Adaptive [histogram equalization](https://en.wikipedia.org/wiki/Adaptive_histogram_equalization)
<img src="data/articles/random/aheq.PNG" width="100%">

## [Convolution theorem](https://en.wikipedia.org/wiki/Convolution_theorem) aware frequency domain image filtering
<img src="data/dissertation/bandpass/2DBandpassRingRIDFT.png" width="32%"> <img src="data/dissertation/bandpass/2DBandpassImageR.png" width="32%"> <img src="data/dissertation/bandpass/2DBandpassImageG.png" width="32%">

## Modeling of the [windowing effect](https://en.wikipedia.org/wiki/Window_function) on the [Discrete Fourier transform](https://en.wikipedia.org/wiki/Discrete_Fourier_transform)
<img src="data/dissertation/window/1DWindows.png" width="47%"> <img src="data/dissertation/window/1DWindowsDFT.png" width="47%">
<img src="data/dissertation/window/2DWindowDFTR.png" width="32%"> <img src="data/dissertation/window/2DImageDFT.png" width="32%">  <img src="data/dissertation/window/2DImageWindowDFT.png" width="32%">

## Profiling
### Profiling with [Valgrind](https://valgrind.org/) / [Callgrind](https://valgrind.org/docs/manual/cl-manual.html) / [Kcachegrind](http://kcachegrind.sourceforge.net/html/Documentation.html)
- run `./script/perf/kcachegrind.sh`

### Profiling with [Optick](https://github.com/bombomby/optick)
- run the app with `ENABLE_PROFILING` defined
- trace file is created when app terminates
- run the Optick GUI app (only on Windows) and import the trace file from `build/`
<img src="data/readme/optick.png" width="100%">

### Profiling with [Tracy](https://github.com/wolfpld/tracy)
- install the required libraries `sudo apt-get -y install libglfw3-dev libgtk-3-dev libcapstone-dev libtbb-dev`
- build the profiler via `make release -j12` in `./libs/tracy/profiler/build/unix/`
- run the app with `ENABLE_PROFILING` defined
- run `script/perf/tracy.sh` or `./libs/tracy/profiler/build/unix/Tracy-release` and click connect
- save the trace file if needed
<img src="data/readme/tracy.png" width="100%">

