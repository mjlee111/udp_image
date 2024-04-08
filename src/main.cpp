#include "../include/udpimage.h"

#include <QApplication>

int main(int argc, char* argv[])
{
  QApplication a(argc, argv);
  udpImage w;
  w.show();
  return a.exec();
}
