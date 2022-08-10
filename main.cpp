#include "trackersCollecter.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    trackersCollecter w;
    w.show();
    return a.exec();
}
