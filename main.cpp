#include <QScreen>
#include <QDesktopWidget>

#include "mainwindow.h"
#include "globalapplication.h"
#include <csignal>
#include <unistd.h>

static QApplication* app = nullptr;

static MainWindow* mainWindow = nullptr;

static void handleSignal(int signal) {
    DBG_NOTICE("Catch Unix signal %d...", signal);

    if (app && mainWindow) {
        // 将信号编号发送给 MainWindow
        QMetaObject::invokeMethod(mainWindow, "unixSignalHandler", Qt::QueuedConnection, Q_ARG(int, signal));
        // 退出应用程序
        //app->quit(); // don't need this, since MainWindow::unixSignalHandler will do exit operation.
    }
}

static void getScreenResolution()
{
    qreal screenWidth = QGuiApplication::primaryScreen()->geometry().width();
    qreal screenHeight = QGuiApplication::primaryScreen()->geometry().height();

    qCritical() << "QGuiApplication Screen Resolution:" << screenWidth << "x" << screenHeight;
    DBG_INFO("QGuiApplication Screen  resolution: %f X %f...\n", screenWidth, screenHeight);

    QRect screenGeometry = QApplication::desktop()->geometry();
    int screenWidth2 = screenGeometry.width();
    int screenHeight2 = screenGeometry.height();

    //qCritical() << "QApplication Screen Resolution:" << screenWidth2 << "x" << screenHeight2;
    DBG_INFO("QApplication Screen  resolution: %d X %d...\n", screenWidth2, screenHeight2);
}

int main(int argc, char *argv[])
{
    GlobalApplication a(argc, argv);

    MainWindow w;
    app = &a;
    mainWindow = &w;

    // 注册信号处理函数
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);
    std::signal(SIGSEGV, handleSignal);
    std::signal(SIGABRT, handleSignal);
    std::signal(SIGBUS, handleSignal);
    std::signal(SIGUSR1, handleSignal);
    std::signal(SIGUSR2, handleSignal);
    std::signal(SIGTSTP, handleSignal);

#if 0
    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGUSR1);
    sigaddset(&signal_set, SIGUSR2);
    pthread_sigmask(SIG_UNBLOCK, &signal_set, nullptr);
#endif

    w.setWindowFlags(w.windowFlags() & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);
    w.show();
    getScreenResolution();

    return a.exec();
}
