#pragma once
#include "ui_Zdeny_PhD_Shenanigans.h"
#include "Gui/Windows/IPC/WindowIPC.h"
#include "Gui/Windows/Diffrot/WindowDiffrot.h"
#include "Gui/Windows/Features/WindowFeatures.h"
#include "Gui/Windows/FITS/WindowFITS.h"

class Zdeny_PhD_Shenanigans : public QMainWindow
{
  Q_OBJECT

public:
  Zdeny_PhD_Shenanigans(QWidget *parent = Q_NULLPTR);

private:
  Ui::Zdeny_PhD_ShenanigansClass ui;
  std::unique_ptr<Globals> globals;

  std::unique_ptr<WindowIPC> windowIPC;
  std::unique_ptr<WindowDiffrot> windowDiffrot;
  std::unique_ptr<WindowFeatures> windowFeatures;
  std::unique_ptr<WindowFITS> windowFITS;

private slots:
  void exit();
  void about();
  void ShowWindowIPC();
  void CloseAll();
  void debug();
  void ShowWindowDiffrot();
  void playSnake();
  void generateLand();
  void closeEvent(QCloseEvent *event);
  void ShowWindowFeatures();
  void ShowWindowFITS();
};
