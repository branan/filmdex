#include "filmdex.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FilmDex w;
    w.show();

    return a.exec();
}
