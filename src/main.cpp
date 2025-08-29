#include "MainWindow.h"
#include <QApplication>
#include <QSettings>
#include <QTranslator>
#include <QFile>
#include <QDebug>
#include <QDirIterator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("MyCompany");
    QCoreApplication::setApplicationName("AudiobookPlayer");

    QSettings settings;

    // --- BƯỚC 1: VÔ HIỆU HÓA TẠM THỜI TÍNH NĂNG NGÔN NGỮ ---
    // QTranslator translator;
    // int langIndex = settings.value("Settings/language", 0).toInt();
    // if (langIndex == 1) { // English
    //     // File .qm được tạo bởi lrelease và thêm vào resources thông qua CMake
    //     if (translator.load(":/translations/lang_en.qm")) {
    //         app.installTranslator(&translator);
    //     } else {
    //         qWarning() << "Could not load translation file ':/translations/lang_en.qm'";
    //     }
    // }

    // --- BƯỚC 1: VÔ HIỆU HÓA TẠM THỜI TÍNH NĂNG CHỦ ĐỀ (THEME) ---
    // QString theme = settings.value("Settings/theme", "light").toString();
    // if (theme == "dark") {
    //     QFile styleFile(":/styles/dark.qss");
    //     if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
    //         app.setStyleSheet(QLatin1String(styleFile.readAll()));
    //         styleFile.close();
    //     } else {
    //         qWarning() << "Could not load dark theme stylesheet.";
    //     }
    // }
    // Nếu là "light", không cần làm gì vì đó là mặc định (không có stylesheet)

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
