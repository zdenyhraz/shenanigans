# Zdeny's shenanigans

<p align="center">
<a href="https://www.codacy.com/gh/zdenyhraz/shenanigans/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=zdenyhraz/shenanigans&amp;utm_campaign=Badge_Grade"><img src="https://app.codacy.com/project/badge/Grade/ea68f108539b4e4eb13b0e92a905ef50"/></a>
<a href="https://www.codefactor.io/repository/github/zdenyhraz/shenanigans/overview/master"><img src="https://www.codefactor.io/repository/github/zdenyhraz/shenanigans/badge/master" alt="CodeFactor" /></a>
<a href="https://github.com/zdenyhraz/shenanigans/commits/master"><img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/zdenyhraz/shenanigans"></a>
<a href="https://github.com/zdenyhraz/shenanigans/issues"><img alt="GitHub issues" src="https://img.shields.io/github/issues-raw/zdenyhraz/shenanigans"></a>
<a href="https://github.com/zdenyhraz/shenanigans/tree/master/src"><img alt="Lines of code" src="https://img.shields.io/tokei/lines/github/zdenyhraz/shenanigans"></a>
<a href="https://github.com/zdenyhraz/shenanigans/search?l=c%2B%2B"><img alt="GitHub top language" src="https://img.shields.io/github/languages/top/zdenyhraz/shenanigans"></a>
<a href="https://github.com/zdenyhraz/shenanigans/commits/master"><img alt="GitHub commit activity" src="https://img.shields.io/github/commit-activity/m/zdenyhraz/shenanigans"></a>
<img alt="GitHub code size in bytes" src="https://img.shields.io/github/languages/code-size/zdenyhraz/shenanigans">
<img alt="GitHub repo size" src="https://img.shields.io/github/repo-size/zdenyhraz/shenanigans">
</p>

Semi-random PhD funky stuff. Mainly contains calculations for astrophysics articles I worked on - [Solar Dynamics Observatory](https://www.nasa.gov/mission_pages/sdo/main/index.html) image processing, image registration, also non-convex optimization, fractals. With [Qt5 GUI](https://www.qt.io/), #uses [C++17](https://en.cppreference.com/w/cpp/17), [OpenCV](https://opencv.org/), [OpenMP](https://www.openmp.org/), [fmt](https://fmt.dev/latest/index.html), [QCustomPlot](https://www.qcustomplot.com/), [spdlog](https://github.com/gabime/spdlog), [CUDA](https://developer.nvidia.com/cuda-toolkit). ***Examples below.***

## Article links
- 📄 [Iterative Phase Correlation Algorithm for High-precision Subpixel Image Registration](https://iopscience.iop.org/article/10.3847/1538-4365/ab63d7)
- 📄 [Measuring Solar Differential Rotation with an Iterative Phase Correlation Method](https://iopscience.iop.org/article/10.3847/1538-4365/abc702)

## Subpixel [image registration](https://en.wikipedia.org/wiki/Image_registration) via [Iterative Phase Correlation](https://iopscience.iop.org/article/10.3847/1538-4365/ab63d7) (gradual 1 pixel shift via [bilinear interpolation](https://en.wikipedia.org/wiki/Bilinear_interpolation) of a 256x256 image, no noise vs heavy noise)
<img src="dissertation/refinement/peakshift.gif" width="50%"><img src="dissertation/refinement/peakshift_noise.gif" width="50%">

## [Solar photosphere](https://en.wikipedia.org/wiki/Photosphere) [differential rotation](https://en.wikipedia.org/wiki/Differential_rotation) measurements from [SDO/HMI](http://hmi.stanford.edu/) continuum images
<p align="center">
<img src="articles/diffrot/pics/gif/1.gif" width="43%">&nbsp; &nbsp; &nbsp; &nbsp; &nbsp;<img src="articles/diffrot/pics/gif/2.gif" width="43%">
</p>

## The [Qt5](https://www.qt.io/) GUI
<img src="articles/random/gui.PNG" width="100%">

## Visualization of [optimizer](https://en.wikipedia.org/wiki/Mathematical_optimization) progress on a given objective function landscape
<img src="articles/optim/rosenbrock_paths.png" width="100%">

## Visualization of *meta*-[optimizer](https://en.wikipedia.org/wiki/Mathematical_optimization) progress and optimization improvement on a given *meta*-objective function landscape
<img src="articles/optim/metaopt_paths.png" width="100%">

## Adaptive [histogram equalization](https://en.wikipedia.org/wiki/Adaptive_histogram_equalization)
<img src="articles/random/aheq.PNG" width="100%">

## [Convolution theorem](https://en.wikipedia.org/wiki/Convolution_theorem) aware frequency domain image filtering
<img src="dissertation/bandpass/2DBandpassRingRIDFT.png" width="32%"> <img src="dissertation/bandpass/2DBandpassImageR.png" width="32%"> <img src="dissertation/bandpass/2DBandpassImageG.png" width="32%">

## Modeling of the [windowing effect](https://en.wikipedia.org/wiki/Window_function) on the [Discrete Fourier transform](https://en.wikipedia.org/wiki/Discrete_Fourier_transform)
<img src="dissertation/window/1DWindows.png" width="47%"> <img src="dissertation/window/1DWindowsDFT.png" width="47%">
<img src="dissertation/window/2DWindowDFTR.png" width="32%"> <img src="dissertation/window/2DImageDFT.png" width="32%">  <img src="dissertation/window/2DImageWindowDFT.png" width="32%"> 
