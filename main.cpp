#include "MainWindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFont font("Microsoft YaHei", 10);
    app.setFont(font);

    MainWindow w;
    w.show();

    return app.exec();
}
