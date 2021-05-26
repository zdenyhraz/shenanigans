#include "stdafx.h"
#include "WindowFeatures.h"

WindowFeatures::WindowFeatures(QWidget* parent, Globals* globals) : QMainWindow(parent), globals(globals)
{
  ui.setupUi(this);
  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(FeatureMatch()));
}

void WindowFeatures::FeatureMatch()
{
  LOG_FUNCTION("FeatureMatch GUI");

  FeatureMatchData data;
  data.path = ui.lineEdit_8->text().toStdString();
  data.ftype = (FeatureType)ui.comboBox->currentIndex();
  data.thresh = ui.lineEdit_4->text().toDouble();
  data.matchcnt = ui.lineEdit_3->text().toInt();
  data.minSpeed = ui.lineEdit_6->text().toDouble();
  data.maxSpeed = ui.lineEdit_7->text().toDouble();
  data.overlapdistance = ui.lineEdit_13->text().toDouble();
  data.drawOverlapCircles = ui.checkBox->isChecked();
  data.ratioThreshold = ui.lineEdit_14->text().toDouble();
  data.upscale = ui.lineEdit_15->text().toDouble();
  data.surfExtended = ui.checkBox_2->isChecked();
  data.surfUpright = ui.checkBox_3->isChecked();
  data.nOctaves = ui.lineEdit_16->text().toInt();
  data.nOctaveLayers = ui.lineEdit_16->text().toInt();
  data.mask = ui.checkBox_4->isChecked();

  featureMatch(data);
}
