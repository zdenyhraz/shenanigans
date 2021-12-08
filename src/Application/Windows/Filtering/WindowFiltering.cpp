
#include "WindowFiltering.h"
#include "Filtering/HistogramEqualization.h"

WindowFiltering::WindowFiltering(QWidget* parent, Globals* globals_) : QMainWindow(parent), globals(globals_)
{
  ui.setupUi(this);
  connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(HistogramEqualize()));
}

void WindowFiltering::HistogramEqualize()
{
  cv::Mat img = cv::imread(ui.lineEdit->text().toStdString(), cv::IMREAD_GRAYSCALE);

  normalize(img, img, 0, 255, cv::NORM_MINMAX);
  img.convertTo(img, CV_8UC1);

  if (ui.checkBox->isChecked())
    cv::resize(img, img, cv::Size(ui.lineEdit_2->text().toDouble() * img.cols, ui.lineEdit_2->text().toDouble() * img.rows));

  LOG_DEBUG("Input image size [{},{}]", img.cols, img.rows);
  cv::Mat heq = EqualizeHistogram(img);
  cv::Mat aheq = EqualizeHistogramAdaptive(img, ui.lineEdit_3->text().toInt());

  ShowHistogram(img, "img histogram");
  ShowHistogram(heq, "heq histogram");
  ShowHistogram(aheq, "aheq histogram");

  showimg(std::vector<cv::Mat>{img, heq, aheq}, "hist eq", false, 0, 1, 1200);
}
