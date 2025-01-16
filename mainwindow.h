#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "FrameProcessThread.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent *event);

private slots:

    bool new_frame_display(QImage image);
    bool update_status_info(status_params2 param2);

    void updateTime(void);
    void clickQuitButton(void);
    void on_stopButton_clicked();
    void on_RGBButton_clicked();
    void on_PHRButton_clicked();
    void on_PCMButton_clicked();
    void on_FHRButton_clicked();
    void simulateButtonClick();
    void onThreadEnd(int stop_request_code);
    void onCtrl_X_Pressed(void);
    void onCtrl_S_Pressed(void);
    void unixSignalHandler(int signal);

private:
    Ui::MainWindow *ui;
    int         tested_times;
    int         to_test_times;
    QTimer      *test_timer;
    FrameProcessThread *frame_process_thread;
    unsigned long firstDisplayFrameTimeUsec;
    unsigned long displayedFrameCnt;
    int displayed_fps;

    void captureAndSaveScreenshot();
};

#endif // MAINWINDOW_H
