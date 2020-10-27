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

  std::unordered_map<std::string, std::unique_ptr<QMainWindow>> mWindows;

private slots:
  void CloseEvent(QCloseEvent *event);
  void ShowWindowIPC();
  void ShowWindowDiffrot();
  void ShowWindowFeatures();
  void ShowWindowFITS();
  void Exit();
  void About();
  void CloseAll();
  void Debug();
  void Snake();
  void GenerateLand();
};
