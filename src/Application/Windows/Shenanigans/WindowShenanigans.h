#pragma once
#include "ui_WindowShenanigans.h"
#include "Application/WindowData.h"

class WindowShenanigans : public QMainWindow
{
  Q_OBJECT

public:
  WindowShenanigans(QWidget* parent = Q_NULLPTR);

private:
  Ui::Zdeny_PhD_ShenanigansClass ui;
  std::unique_ptr<Globals> globals;
  std::unordered_map<std::string, std::unique_ptr<QMainWindow>> mWindows;

private slots:
  void closeEvent(QCloseEvent* event);
  void ShowWindowIPC();
  void ShowWindowDiffrot();
  void ShowWindowFeatures();
  void ShowWindowFITS();
  void ShowWindowFiltering();
  void Exit();
  void About();
  void CloseAll();
  void Snake();
  void GenerateLand();
  void UnitTests();
  void RandomShit();
};
