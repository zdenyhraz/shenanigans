cmake_minimum_required(VERSION 3.12.2)

project(Zdeny_PhD_Shenanigans)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release"
    CACHE STRING "Configuration types" FORCE)

add_executable(Zdeny_PhD_Shenanigans
  WIN32
  Astrophysics/diffrot.cpp
  Astrophysics/diffrot.h
  Astrophysics/diffrotFileIO.h
  Astrophysics/diffrotResults.h
  Astrophysics/FITS.cpp
  Astrophysics/FITS.h
  Astrophysics/SDOipcOpt.cpp
  Astrophysics/SDOipcOpt.h
  Astrophysics/SwindFlow.h
  Core/constants.h
  Core/functionsBaseCV.h
  Core/functionsBaseSTL.h
  Core/globals.h
  Core/macros.h
  DebugStuff/DebugStuff.h
  Draw/colormap.h
  Draw/combinepics.h
  Draw/crop.h
  Draw/draw.h
  Draw/showsave.h
  Features/FeatureMatch.h
  Filtering/filtering.cpp
  Filtering/filtering.h
  Fit/nnfit.h
  Fit/polyfit.h
  Fit/trigfit.h
  Fourier/fourier.cpp
  Fourier/fourier.h
  Fractal/fractal.cpp
  Fractal/fractal.h
  Gui/Main/Zdeny_PhD_Shenanigans.cpp
  Gui/Main/Zdeny_PhD_Shenanigans.h
  Gui/Windows/Diffrot/WindowDiffrot.cpp
  Gui/Windows/Diffrot/WindowDiffrot.h
  Gui/Windows/Features/WindowFeatures.cpp
  Gui/Windows/Features/WindowFeatures.h
  Gui/Windows/IPC2PicAlign/WindowIPC2PicAlign.cpp
  Gui/Windows/IPC2PicAlign/WindowIPC2PicAlign.h
  Gui/Windows/IPCoptimize/WindowIPCoptimize.cpp
  Gui/Windows/IPCoptimize/WindowIPCoptimize.h
  Gui/Windows/IPCparameters/WindowIPCparameters.cpp
  Gui/Windows/IPCparameters/WindowIPCparameters.h
  Gui/Windows/Plot/WindowPlot.cpp
  Gui/Windows/Plot/WindowPlot.h
  Gui/Windows/Template/WindowTemplate.cpp
  Gui/Windows/Template/WindowTemplate.h
  InverseMap/inverseMapping.cpp
  InverseMap/inverseMapping.h
  IPC/IPC.cpp
  IPC/IPC.h
  IPC/ipcAux.cpp
  IPC/ipcAux.h
  IPC/IterativePhaseCorrelation.cpp
  IPC/IterativePhaseCorrelation.h
  Log/logger.cpp
  Log/logger.h
  main.cpp
  Neural/neural.cpp
  Neural/neural.h
  Optimization/Evolution.cpp
  Optimization/Evolution.h
  Optimization/GraphOptimization.cpp
  Optimization/GraphOptimization.h
  Optimization/Optimization.cpp
  Optimization/Optimization.h
  Optimization/OptimizationTestFunctions.h
  Optimization/PatternSearch.cpp
  Optimization/PatternSearch.h
  Optimization/VisualOptimization.cpp
  Optimization/VisualOptimization.h
  Plot/Plot.cpp
  Plot/Plot.h
  Plot/Plot1D.cpp
  Plot/Plot1D.h
  Plot/Plot2D.cpp
  Plot/Plot2D.h
  Plot/PlotCSV.cpp
  Plot/PlotCSV.h
  Plot/qcustomplot.cpp
  Plot/qcustomplot.h
  Procedural/baseFunctions2d.h
  Procedural/procedural.h
  Resources/logo.png
  Resources/test.png
  Resources/Zdeny_PhD_Shenanigans.ico
  Resources/Zdeny_PhD_Shenanigans.rc
  Snake/game.h
  Snake/map.cpp
  Snake/map.h
  Snake/snake.cpp
  Snake/snake.h
  stdafx.cpp
  stdafx.h
  Utils/export.h
  Utils/vectmat.h
)
# Additional include directories
set_property(TARGET Zdeny_PhD_Shenanigans
  APPEND PROPERTY INCLUDE_DIRECTORIES
  $<$<CONFIG:Debug>:
    C:\include\spdlog\include;
    C:\include\opencv\build\include;
    .\;
    C:\include\blaze\include>
  $<$<CONFIG:Release>:
    C:\include\spdlog\include;
    C:\include\opencv\build\include;
    .\;
    C:\include\blaze\include>
)

# Preprocessor definitions
target_compile_definitions(Zdeny_PhD_Shenanigans PRIVATE
  
)

# Minimal rebuild
if (MSVC)
  target_compile_options(Zdeny_PhD_Shenanigans PRIVATE
    "$<$<CONFIG:Debug>:/Gm->"
    "$<$<CONFIG:Release>:/Gm->"
  )
endif ()

# Multi-processor compilation
if (MSVC)
  target_compile_options(Zdeny_PhD_Shenanigans PRIVATE
    "$<$<CONFIG:Debug>:/MP>"
    "$<$<CONFIG:Release>:/MP>"
  )
endif ()

# Precompiled header files
if (MSVC)
  set_property(SOURCE Astrophysics/diffrot.cpp IPC/ipcAux.cpp IPC/IterativePhaseCorrelation.cpp Log/logger.cpp Optimization/Evolution.cpp Optimization/PatternSearch.cpp Optimization/VisualOptimization.cpp Plot/Plot.cpp Plot/Plot1D.cpp Plot/Plot2D.cpp Plot/PlotCSV.cpp Snake/map.cpp Snake/snake.cpp Gui/Windows/Plot/WindowPlot.cpp Gui/Windows/Features/WindowFeatures.cpp Filtering/filtering.cpp Astrophysics/FITS.cpp Fourier/fourier.cpp Fractal/fractal.cpp Astrophysics/SDOipcOpt.cpp Optimization/GraphOptimization.cpp InverseMap/inverseMapping.cpp IPC/IPC.cpp Neural/neural.cpp Optimization/Optimization.cpp main.cpp Gui/Windows/Diffrot/WindowDiffrot.cpp Gui/Windows/IPC2PicAlign/WindowIPC2PicAlign.cpp Gui/Windows/IPCoptimize/WindowIPCoptimize.cpp Gui/Windows/IPCparameters/WindowIPCparameters.cpp Gui/Windows/Template/WindowTemplate.cpp Gui/Main/Zdeny_PhD_Shenanigans.cpp
    APPEND_STRING PROPERTY COMPILE_FLAGS
    "$<$<CONFIG:Debug>:/Yu>\
     $<$<CONFIG:Release>:/Yu>")
  set_property(SOURCE stdafx.cpp
    APPEND_STRING PROPERTY COMPILE_FLAGS
    "$<$<CONFIG:Debug>:/Yc>\
     $<$<CONFIG:Release>:/Yc>")
endif ()

# Additional library directories
if (MSVC)
  target_link_options(Zdeny_PhD_Shenanigans PRIVATE
    $<$<CONFIG:Debug>:
      /LIBPATH:C:\include\opencv\build\x64\vc14\lib>
    $<$<CONFIG:Release>:
      /LIBPATH:C:\include\opencv\build\x64\vc14\lib>
  )
else ()
  target_link_options(Zdeny_PhD_Shenanigans PRIVATE
    $<$<CONFIG:Debug>:
      -LC:\include\opencv\build\x64\vc14\lib>
    $<$<CONFIG:Release>:
      -LC:\include\opencv\build\x64\vc14\lib>
  )
endif ()

# Link libraries
set_property(TARGET Zdeny_PhD_Shenanigans
  APPEND PROPERTY LINK_LIBRARIES
  "$<$<CONFIG:Debug>:opencv_world343d.lib>"
  "$<$<CONFIG:Release>:opencv_world343.lib>"
)
