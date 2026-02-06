#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFile>

static void applyTheme() {
    QFile f(":/theme/theme.qss");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Stable AppDataLocation for Repository (QStandardPaths).
    QCoreApplication::setOrganizationName("Rewise");
    QCoreApplication::setApplicationName("Rewise");

    // Make QSS behavior more predictable across platforms.
    QApplication::setStyle("Fusion");

    applyTheme();

    MainWindow w;
    w.show();
    return a.exec();
}
