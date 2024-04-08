/**
 * @file udp.h
 * @author mjlee111
 * @brief
 * @version 1.0
 * @date 2023-10-06
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef UDP_H
#define UDP_H
#pragma once
#include <iostream>
#include <string>
#include <unistd.h>
#include <QtNetwork/QUdpSocket>
#include <QHostAddress>
#include <QMainWindow>
#include <QByteArray>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <thread>
#include <vector>

#define LUT_SIZE 256

static const uint8_t REFLECT_BIT_ORDER_TABLE[256] = {
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48,
  0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4,
  0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C,
  0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2,
  0x32, 0xB2, 0x72, 0xF2, 0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A,
  0xFA, 0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 0x0E, 0x8E,
  0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81, 0x41, 0xC1, 0x21,
  0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1, 0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
  0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55,
  0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD,
  0x7D, 0xFD, 0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 0x0B,
  0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47, 0xC7,
  0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F,
  0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

namespace udp
{
using namespace cv;
using namespace std;

class CRC8
{
public:
  CRC8(uint8_t polynomial, uint8_t init_value, bool reflect_input, bool reflect_output, uint8_t xor_output,
       bool use_lut)
  {
    this->polynomial = polynomial;
    this->init_value = init_value;
    this->reflect_input = reflect_input;
    this->reflect_output = reflect_output;
    this->xor_output = xor_output;
    this->use_lut = use_lut;

    memset(this->lookup_table, 0, LUT_SIZE * sizeof(uint8_t));

    if (this->use_lut)
      create_lookup_table();
  };
  CRC8(bool use_lut = false) : CRC8(0x07, 0x00, false, false, 0x00, use_lut){};

public:
  uint8_t calculate(const char* string)
  {
    std::vector<uint8_t> data;

    for (size_t i = 0; i < strlen(string); i++)
    {
      data.push_back(string[i]);
    }

    return calculate(data);
  }
  uint8_t calculate(const uint8_t* data_in, const int length)
  {
    std::vector<uint8_t> data;

    for (int i = 0; i < length; i++)
    {
      data.push_back(data_in[i]);
    }

    return calculate(data);
  }
  uint8_t calculate(std::vector<uint8_t> data)
  {
    uint8_t crc = this->init_value;
    uint8_t byte;

    if (this->use_lut)
    {
      for (size_t i = 0; i < data.size(); i++)
      {
        byte = this->reflect_input ? REFLECT_BIT_ORDER_TABLE[data[i]] : data[i];
        crc = this->lookup_table[crc ^ byte];
      }
    }
    else
    {
      for (size_t i = 0; i < data.size(); i++)
      {
        byte = this->reflect_input ? REFLECT_BIT_ORDER_TABLE[data[i]] : data[i];
        crc ^= byte;
        for (int j = 0; j < 8; j++)
        {
          crc = crc & 0x80 ? (crc << 1) ^ this->polynomial : crc << 1;
        }
      }
    }

    if (this->reflect_output)
    {
      crc = reflect(crc);
    }

    crc = (crc ^ this->xor_output) & 0xFF;

    return crc;
  }
  void printLookupTable()
  {
    for (int i = 0; i < 32; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        printf("0x%02X, ", this->lookup_table[i * 8 + j]);
      }
      printf("\n");
    }
  }

private:
  uint8_t polynomial;
  uint8_t init_value;
  bool reflect_input;
  bool reflect_output;
  uint8_t xor_output;

  uint8_t reflect(uint8_t value)
  {
    uint8_t reflected = 0;

    for (int i = 0; i < 8; i++)
    {
      if (value & 0x01)
        reflected |= (1 << ((8 - 1) - i));
      value = (value >> 1);
    }

    return reflected;
  }

  bool use_lut;
  uint8_t lookup_table[LUT_SIZE];
  void create_lookup_table()
  {
    uint8_t x;

    for (int i = 0; i < LUT_SIZE; i++)
    {
      x = (uint8_t)i;
      for (int j = 0; j < 8; j++)
      {
        x = x & 0x80 ? (x << 1) ^ this->polynomial : x << 1;
      }
      this->lookup_table[i] = x;
    }
  }
};

class UDP : public QObject
{
  Q_OBJECT
public:
  UDP(){};
  ~UDP(){};

  typedef struct
  {
    int data_size;
    cv::Mat img;
  } imgReturn;

  QHostAddress ROBOT_IP = QHostAddress("192.168.199.100");
  QHostAddress ROBOT_VELODYNE_IP = QHostAddress("192.168.188.100");
  QHostAddress OPERATOR_IP = QHostAddress("192.168.199.253");
  QHostAddress VELODYNE_PC_IP = QHostAddress("192.168.188.99");

  uint16_t camera_request_PORT = 8888;  // tx
  uint16_t cam1_PORT = 8881;            // rx
  uint16_t cam2_PORT = 8882;            // rx
  uint16_t cam3_PORT = 8883;            // rx
  uint16_t cam4_PORT = 8884;            // rx
  uint16_t cam5_PORT = 8885;            // rx
  uint16_t cam6_PORT = 8886;            // rx

  uint16_t communication_status_PORT = 4444;  // rx, tx
  uint16_t mobile_base_PORT = 1111;           // rx,tx
  uint16_t flipper_PORT = 2222;               // rx, tx
  uint16_t manipulator_PORT = 3333;           // rx, tx
  uint16_t robot_status_PORT = 5555;          // rx

  uint16_t realsense1_PORT = 7771;  // rx
  uint16_t realsense2_PORT = 7772;  // rx
  uint16_t realsense3_PORT = 7773;  // rx
  uint16_t realsense4_PORT = 7774;  // rx

  uint16_t velodyne_PORT = 6666;
  uint16_t SLAM_PORT = 6665;

  void UdpTextTransfer(QString text, uint16_t port, QHostAddress address, QUdpSocket& socket)
  {
    QByteArray packet;
    packet.append(text);
    socket.writeDatagram(packet, address, port);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    packet.clear();
    return;
  }

  void UdpArrayTransfer(QByteArray text, uint16_t port, QHostAddress address, QUdpSocket& socket)
  {
    QByteArray packet;
    packet.append(text);
    socket.writeDatagram(packet, address, port);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    packet.clear();
    return;
  }

  bool CRC8CHK(const QByteArray& data)
  {
    uint8_t end = data.at(data.size() - 1);
    QByteArray dataWithoutEnd = data;
    dataWithoutEnd.remove(dataWithoutEnd.size() - 1, 1);

    if (end != CRC8(true).calculate(dataWithoutEnd))
    {
      return false;
    }
    else
    {
      return true;
    }
    dataWithoutEnd.clear();
  }

  void CRC8INPUT(QByteArray& data)
  {
    uint8_t crc_result = CRC8(true).calculate(data);
    data.push_back(crc_result);
    return;
  }

public slots:
  QByteArray UdpReadQByteArray(uint16_t port, QHostAddress& address, QUdpSocket& socket)
  {
    QByteArray buffer;
    buffer.resize(socket.pendingDatagramSize());
    socket.readDatagram(buffer.data(), buffer.size(), &address, &port);
    /*

    put your return code here ...

    */
    return buffer;
  }

  bool UdpCheckRcv(uint16_t port, QHostAddress& address, QUdpSocket& socket)
  {
    QByteArray buffer;
    buffer.resize(socket.pendingDatagramSize());
    socket.readDatagram(buffer.data(), buffer.size(), &address, &port);

    if (buffer.at(0) == 'o' && buffer.at(1) == 'k')
    {
      QByteArray packet;
      packet.push_back("ok");
      socket.writeDatagram(packet, address, port);
      connection = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    else
    {
      QByteArray packet;
      packet.push_back("cl");  // connection lost
      socket.writeDatagram(packet, address, port);
      connection = false;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    buffer.clear();
    return connection;
  }

  void UdpCheckTrs(uint16_t port, QHostAddress& address, QUdpSocket& socket)
  {
    QByteArray packet;
    packet.append("ok");
    socket.writeDatagram(packet, address, port);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    packet.clear();
    return;
  }

  int MatImgTrs(Mat img, uint16_t port, QHostAddress& address, QUdpSocket& socket)
  {
    if (!img.empty())
    {
      std::vector<uchar> img2Vector;
      std::vector<int> param = { cv::IMWRITE_JPEG_QUALITY, 60 };

      imencode(".jpg", img, img2Vector, param);
      Mat jpegImage = imdecode(Mat(img2Vector), cv::IMREAD_COLOR);

      QByteArray array(reinterpret_cast<const char*>(img2Vector.data()), img2Vector.size());

      int sendChk = socket.writeDatagram(array.data(), array.size(), address, port);

      while (sendChk == -1)
      {
        sendChk = socket.writeDatagram(array.data(), array.size(), address, port);
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      return array.size();
    }

    else
    {
      return 0;
    }
  }

  imgReturn MatImgRcv(Mat img, uint16_t port, QHostAddress& address, QUdpSocket& socket)
  {
    QByteArray cam_buffer;
    imgReturn temp;

    cam_buffer.resize(socket.pendingDatagramSize());
    temp.data_size = socket.pendingDatagramSize();

    socket.readDatagram(cam_buffer.data(), cam_buffer.size(), &address, &port);
    std::vector<uchar> down_decoding(cam_buffer.begin(), cam_buffer.end());
    img = imdecode(Mat(down_decoding), cv::IMREAD_COLOR);
    temp.img = img;
    cam_buffer.clear();
    return temp;
  }

  //   imgReturn MatImgTrs(Mat img, uint16_t port, QHostAddress& address, QUdpSocket& socket) {
  //     imgReturn result;
  //     if (!img.empty()) {
  //         std::vector<uchar> img2Vector;
  //         std::vector<int> param = { cv::IMWRITE_JPEG_QUALITY, 60 };

  //         // Encode image to JPEG
  //         imencode(".jpg", img, img2Vector, param);

  //         // Extract image width and height
  //         int width = img.cols;
  //         int height = img.rows;

  //         // Combine image data, width, and height into QByteArray
  //         QByteArray array;
  //         QDataStream stream(&array, QIODevice::WriteOnly);
  //         stream << width << height << QByteArray(reinterpret_cast<const char*>(img2Vector.data()),
  //         img2Vector.size());

  //         // Send data via UDP
  //         int sendChk = socket.writeDatagram(array.data(), array.size(), address, port);

  //         // Check for successful sending
  //         while (sendChk == -1) {
  //             sendChk = socket.writeDatagram(array.data(), array.size(), address, port);
  //         }

  //         std::this_thread::sleep_for(std::chrono::milliseconds(1));

  //         // Populate the result struct
  //         result.time = /* Set time as needed */;
  //         result.width = width;
  //         result.height = height;
  //         result.img = img.clone(); // Clone the image to avoid modifying the original

  //         return result;
  //     } else {
  //         // If image is empty, return empty imgReturn
  //         return result;
  //     }
  // }

  // imgReturn MatImgRcv(uint16_t port, QHostAddress& address, QUdpSocket& socket)
  // {
  //   imgReturn result;

  //   // Read data from UDP socket
  //   QByteArray cam_buffer;
  //   cam_buffer.resize(socket.pendingDatagramSize());
  //   socket.readDatagram(cam_buffer.data(), cam_buffer.size(), &address, &port);

  //   // Extract width, height, and image data from QByteArray
  //   QDataStream stream(cam_buffer);
  //   stream >> result.width >> result.height;
  //   cam_buffer = cam_buffer.mid(stream.device()->pos());  // Remove the width and height from the buffer
  //   std::vector<uchar> down_decoding(cam_buffer.begin(), cam_buffer.end());

  //   // Decode image
  //   result.img = imdecode(Mat(down_decoding), cv::IMREAD_COLOR);

  //   // Clear the buffer
  //   cam_buffer.clear();

  //   return result;
  // }

private:
  bool connection = false;
};
};  // namespace udp

#endif
