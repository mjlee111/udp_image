#include "../include/udpimage.h"
#include "ui_udpimage.h"

udpImage::udpImage(QWidget* parent) : QMainWindow(parent), ui(new Ui::udpImage)
{
  ui->setupUi(this);
  udpPtr = new udp::UDP;

  ui->ip1->setPlaceholderText("192.168.1.1");
  ui->ip2->setPlaceholderText("192.168.1.100");
  ui->port->setPlaceholderText("8888");

  resetPlot();

  connect(fps_timer, SIGNAL(timeout()), this, SLOT(fpsUpdate()));
  connect(avg_reset_timer, SIGNAL(timeout()), this, SLOT(avgReset()));
}

udpImage::~udpImage()
{
  delete fps_timer;
  delete avg_reset_timer;
  delete fps_curve;
  delete byte_curve;
  delete udpPtr;
  delete ui;
}

void udpImage::resetPlot()
{
  ui->fps_plot->setTitle("FPS");
  ui->fps_plot->setAxisTitle(QwtPlot::xBottom, "Time");
  fps_curve = new QwtPlotCurve("FPS");
  fps_curve->attach(ui->fps_plot);
  fps_curve->setPen(QColor(Qt::green));
  QPen pen = fps_curve->pen();
  pen.setWidth(2);
  fps_curve->setPen(pen);
  ui->fps_plot->setAxisScale(QwtPlot::xBottom, 0.0, 10.0);
  ui->fps_plot->setAxisScale(QwtPlot::yLeft, 0.0, 60.0);

  ui->byte_plot->setTitle("byte");
  ui->byte_plot->setAxisTitle(QwtPlot::xBottom, "Time");
  byte_curve = new QwtPlotCurve("byte");
  byte_curve->attach(ui->byte_plot);
  byte_curve->setPen(QColor(Qt::yellow));
  QPen pen2 = byte_curve->pen();
  pen.setWidth(2);
  byte_curve->setPen(pen2);
  ui->byte_plot->setAxisScale(QwtPlot::xBottom, 0.0, 10.0);
  ui->byte_plot->setAxisScale(QwtPlot::yLeft, 0.0, 20000.0);
}

void udpImage::fpsUpdate()
{
  fps = fps * 4;
  if (fps >= fps_min_max[1])
  {
    fps_min_max[1] = fps;
  }
  if (fps < fps_min_max[0])
  {
    fps_min_max[0] = fps;
  }
  ui->fps->setText(QString::number(fps));

  fps_curve_data.append(QPointF(xValue, fps));
  xValue += 0.25;

  QVector<double> xData, yData;
  for (const QPointF& point : fps_curve_data)
  {
    xData.append(point.x());
    yData.append(point.y());
  }
  if (xValue > 10.0)
  {
    ui->fps_plot->setAxisScale(QwtPlot::xBottom, 0.0, xValue + 10.0);
  }

  fps_curve->setSamples(xData, yData);
  ui->fps_plot->replot();
  ui->max_fps->setText(QString::number(fps_min_max[1]));
  ui->min_fps->setText(QString::number(fps_min_max[0]));
  fpsVector.push_back(fps);

  fps = 0;
}

void udpImage::byteUpdate(int current_byte)
{
  if (current_byte >= byte_min_max[1])
  {
    byte_min_max[1] = current_byte;
  }
  if (current_byte < fps_min_max[0])
  {
    byte_min_max[0] = current_byte;
  }
  ui->data_byte->setText(QString::number(current_byte));

  byte_curve_data.append(QPointF(xValueB, current_byte));
  xValueB += 1;
  QVector<double> xData, yData;
  for (const QPointF& point : byte_curve_data)
  {
    xData.append(point.x());
    yData.append(point.y());
  }
  if (xValue > 10.0)
  {
    ui->byte_plot->setAxisScale(QwtPlot::xBottom, 0.0, xValue + 10.0);
  }

  byte_curve->setSamples(xData, yData);
  ui->byte_plot->replot();
  ui->max_bps->setText(QString::number(byte_min_max[1]));
  ui->min_bps->setText(QString::number(byte_min_max[0]));
  byteVector.push_back(current_byte);
}

void udpImage::avgReset()
{
  double sum = 0.0;
  for (int i = 0; i < fpsVector.size(); ++i)
  {
    sum += fpsVector[i];
  }
  double avg = sum / fpsVector.size();
  ui->avg_fps->setText(QString::number(avg));
  fpsVector.clear();
}

void udpImage::camUpdate()
{
  udp::UDP::imgReturn temp;
  cv::Mat img_temp;
  temp = udpPtr->MatImgRcv(img_temp, port, ip2, *img_socket);
  if (temp.data_size > 0)
  {
    fps++;
    byteUpdate(temp.data_size);
  }
  else
  {
    byteUpdate(0);
    return;
  }

  cv::resize(temp.img, temp.img, cv::Size(320, 240));
  QImage raw_image((const unsigned char*)(temp.img.data), temp.img.cols, temp.img.rows, QImage::Format_RGB888);
  ui->raw_image->setPixmap(QPixmap::fromImage(raw_image.rgbSwapped()));
}

void udpImage::on_btn_start_clicked()
{
  QString as = ui->ip1->text();
  QString bs = ui->ip2->text();
  QString cs = ui->port->text();

  if (as.isEmpty() || bs.isEmpty() || cs.isEmpty())
  {
    QMessageBox::warning(this, "WARNING", "YOU NEED TO ENTER ALL THE DATA!");
    return;
  }

  if (!status)
  {
    status = true;
    ui->btn_start->setStyleSheet("background-color: red;");
    ui->btn_start->setText("STOP");

    ip1 = QHostAddress(ui->ip1->text());
    ip2 = QHostAddress(ui->ip2->text());
    port = ui->port->text().toInt();
    fps_timer->start(250);
    avg_reset_timer->start(1000);

    if (img_socket->bind(ip1, port, QUdpSocket::ShareAddress))
    {
      connect(img_socket, SIGNAL(readyRead()), this, SLOT(camUpdate()));
    }
    else
    {
      return;
    }
  }

  else if (status)
  {
    disconnect(img_socket, SIGNAL(readyRead()), this, SLOT(camUpdate()));
    img_socket->abort();
    img_socket->close();
    status = false;
    ui->btn_start->setStyleSheet("background-color: green;");
    ui->btn_start->setText("START");

    fps_timer->stop();
    avg_reset_timer->stop();
  }
}