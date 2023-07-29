#include "mainwindow.h"
#include <globalapplication.h>

int main(int argc, char *argv[])
{
    GlobalApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
