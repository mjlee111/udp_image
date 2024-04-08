#ifndef UDPIMAGE_H
#define UDPIMAGE_H

#include <QMainWindow>
#include "../udp/udp.h"
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include "qwt/qwt_series_data.h"
#include <QPen>
#include <QTimer>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui
{
class udpImage;
}
QT_END_NAMESPACE

class udpImage : public QMainWindow
{
  Q_OBJECT

public:
  udpImage(QWidget* parent = nullptr);
  ~udpImage();

private:
  Ui::udpImage* ui;
  udp::UDP* udpPtr = nullptr;

  bool status = false;

  QHostAddress ip1;
  QHostAddress ip2;
  uint16_t port;

  void resetPlot();
  void byteUpdate(int current_byte);

  QUdpSocket* img_socket = new QUdpSocket(this);

  QTimer* fps_timer = new QTimer(this);
  QTimer* avg_reset_timer = new QTimer(this);

  QwtPlotCurve* fps_curve;
  QVector<QPointF> fps_curve_data;
  double fps = 0;
  double fps_min_max[2] = { 100000, 0 };  // min , max
  std::vector<double> fpsVector;
  double xValue = 0.0;

  QwtPlotCurve* byte_curve;
  QVector<QPointF> byte_curve_data;
  double byte_min_max[2] = { 10000000, 0 };  // min , max
  std::vector<double> byteVector;
  double xValueB = 0.0;

private Q_SLOTS:
  void fpsUpdate();
  void avgReset();

  void camUpdate();

  void on_btn_start_clicked();
};
#endif  // UDPIMAGE_H
