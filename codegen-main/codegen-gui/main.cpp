#include "CodegenMainWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CodegenMainWindow w;
    w.show();
    return a.exec();
}
