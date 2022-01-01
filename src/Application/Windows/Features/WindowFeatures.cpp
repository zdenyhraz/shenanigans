
#include "WindowFeatures.h"

WindowFeatures::WindowFeatures(QWidget* parent, Globals* globals_) : QMainWindow(parent), globals(globals_)
{
  ui.setupUi(this);
  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(FeatureMatch()));
}

void WindowFeatures::FeatureMatch()
{
  LOG_FUNCTION("FeatureMatch GUI");

  FeatureMatchData fmdata;
  fmdata.path = ui.lineEdit_8->text().toStdString();
  fmdata.ftype = (FeatureType)ui.comboBox->currentIndex();
  fmdata.thresh = ui.lineEdit_4->text().toDouble();
  fmdata.matchcnt = ui.lineEdit_3->text().toInt();
  fmdata.minSpeed = ui.lineEdit_6->text().toDouble();
  fmdata.maxSpeed = ui.lineEdit_7->text().toDouble();
  fmdata.overlapdistance = ui.lineEdit_13->text().toDouble();
  fmdata.drawOverlapCircles = ui.checkBox->isChecked();
  fmdata.ratioThreshold = ui.lineEdit_14->text().toDouble();
  fmdata.upscale = ui.lineEdit_15->text().toDouble();
  fmdata.surfExtended = ui.checkBox_2->isChecked();
  fmdata.surfUpright = ui.checkBox_3->isChecked();
  fmdata.nOctaves = ui.lineEdit_16->text().toInt();
  fmdata.nOctaveLayers = ui.lineEdit_16->text().toInt();
  fmdata.mask = ui.checkBox_4->isChecked();
  fmdata.path1 = ui.lineEdit_9->text().toStdString();
  fmdata.path2 = ui.lineEdit_10->text().toStdString();

  if (!fmdata.path1.empty() and !fmdata.path2.empty())
    featureMatch2pic(fmdata);
  else
    featureMatch(fmdata);
}
