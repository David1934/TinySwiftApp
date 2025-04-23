#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#if !defined(NO_UI_APPLICATION)
#include <QMainWindow>
#endif
#include "FrameProcessThread.h"
#if defined(RUN_ON_EMBEDDED_LINUX)
#include "host_comm.h"
#endif

#define CAPTURED_PICTURE_FMT    "PNG"

namespace Ui {
    class MainWindow;
}

#if defined(NO_UI_APPLICATION)
class MainWindow : public QObject
#else
class MainWindow : public QMainWindow
#endif
{
    Q_OBJECT

public:
    #if defined(NO_UI_APPLICATION)
        explicit MainWindow();
    #else
        explicit MainWindow(QWidget *parent = 0);
    #endif
    ~MainWindow();

protected:

    #if !defined(NO_UI_APPLICATION)
        virtual void closeEvent(QCloseEvent *event);
        virtual bool eventFilter(QObject *obj, QEvent *event);
    #endif

private slots:

    #if !defined(NO_UI_APPLICATION)
        bool new_frame_display(QImage image, QImage img4confidence);
        bool update_status_info(status_params2 param2);

        void updateRTCtime(void);
        void on_startStopButton_clicked();
        void simulateButtonClick();
        void onCtrl_X_Pressed(void);
        void onCtrl_S_Pressed(void);
        void watchSpotInfoUpdate(  QPoint spot, enum frame_data_type ftype, watchPointInfo_t wpi);
    #endif
    void Quit(void);
    void onThreadEnd(int stop_request_code);
    void unixSignalHandler(int signal);
    void on_startCapture();
    void on_stopCapture();
#if defined(RUN_ON_EMBEDDED_LINUX)
    void on_capture_options_set(capture_req_param_t* param);
#endif

private:
    #if defined(RUN_ON_EMBEDDED_LINUX)
        Host_Communication *host_comm;
        Misc_Device *m_misc_dev;
    #endif

    #if !defined(NO_UI_APPLICATION)
        Ui::MainWindow *ui;
        QTimer      *test_timer;
    #endif
    int         tested_times;
    int         to_test_times;
    FrameProcessThread *frame_process_thread;
    unsigned long firstDisplayFrameTimeUsec;
    unsigned long displayedFrameCnt;
    int displayed_fps;

    #if !defined(NO_UI_APPLICATION)
        void captureAndSaveScreenshot();
    #endif
    void startFrameProcessThread(void);
};

#endif // MAINWINDOW_H
