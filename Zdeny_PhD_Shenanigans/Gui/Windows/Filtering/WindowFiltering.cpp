#include "stdafx.h"
#include "WindowFiltering.h"
#include "Filtering/HistogramEqualization.h"

WindowFiltering::WindowFiltering(QWidget *parent, Globals *globals) : QMainWindow(parent), globals(globals)
{
  ui.setupUi(this);
  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(HistogramEqualize()));
}

void WindowFiltering::HistogramEqualize()
{
  Mat img = imread(ui.lineEdit->text().toStdString(), CV_LOAD_IMAGE_GRAYSCALE);

  normalize(img, img, 0, 255, CV_MINMAX);
  img.convertTo(img, CV_8UC1);

  if (ui.checkBox->isChecked())
    cv::resize(img, img, Size(ui.lineEdit_2->text().toDouble() * img.cols, ui.lineEdit_2->text().toDouble() * img.rows));

  LOG_DEBUG("Input image size [{},{}]", img.cols, img.rows);
  Mat heq = EqualizeHistogram(img);
  Mat aheq = EqualizeHistogramAdaptive(img, ui.lineEdit_3->text().toInt());

  ShowHistogram(img, "img histogram");
  ShowHistogram(heq, "heq histogram");
  ShowHistogram(aheq, "aheq histogram");

  showimg(std::vector<Mat>{img, heq, aheq}, "hist eq", false, 0, 1, 1200);
}
