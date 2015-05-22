#include <QApplication>
#include <QTextCodec>
#include <QSplashScreen>
#include <QTimer>
#include <QDesktopWidget>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QSettings>
#include <QMessageBox>

#include "mainwindow.h"


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    a.setApplicationName(QObject::tr(_APP_NAME).replace('_', ' '));
    a.setApplicationVersion(QString(_APP_VERSION));

    // =====================================
    // internationalization
    QTranslator appTranslator;
    QTranslator qtTranslator;

    QSettings s(QApplication::applicationDirPath() + QDir::separator() + QString("settings.ini"), QSettings::IniFormat);
    bool bRussian = 0 == s.value("lang", "en").toString().compare("ru");
    bool bTrFound = bRussian &&
            appTranslator.load("app_ru",
                               qApp->applicationDirPath() + QDir::separator() + "i18n");

    if (bTrFound) {
        qtTranslator.load("qt_ru", ":/i18n");
        a.installTranslator(&qtTranslator);
        a.installTranslator(&appTranslator);
    }
    else if (bRussian) {
        s.setValue("lang", "en");
        QMessageBox *pMsgbox = new QMessageBox(QMessageBox::Information,
                                               a.applicationName(),
                                               "A translation file for Russian language not found\n"
                                               "English UI will be used");
        pMsgbox->setAttribute(Qt::WA_DeleteOnClose);
        pMsgbox->exec();
    }
    // =====================================

    MainWindow mw;
    mw.setWindowIcon(QIcon(":/appicon.ico"));

    QPixmap splashpic(":/appicon.png");
    QSplashScreen *splash = new QSplashScreen(splashpic.scaled(240,
                                                               240,
                                                               Qt::KeepAspectRatioByExpanding,
                                                               Qt::SmoothTransformation),
                                              Qt::WindowStaysOnTopHint);
    splash->show();
//    //splash.showMessage(tr("Loading settings..."), Qt::AlignHCenter|Qt::AlignBottom);

    QTimer t;
    t.setSingleShot(true);
    QObject::connect(&t, &QTimer::timeout, &mw, &MainWindow::show);
    QObject::connect(&t, &QTimer::timeout, splash, &QSplashScreen::close);
    t.start(200);

    // -- import settings DB from parameter if it was specified
    if (argc > 1) {
        if (!mw.importSettingsFile(argv[1]))
            return EXIT_FAILURE;
    }

    return a.exec();
}
