#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "majorimageprocessingthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    bool new_frame_display(QImage image);
    bool update_streaming_info(int fps, unsigned long streamed_time_us);

    void updateTime(void);
    void clickQuitButton(void);
    void on_stopButton_clicked();
    void on_RGBButton_clicked();
    void on_PHRButton_clicked();
    void on_PCMButton_clicked();
    void on_FHRButton_clicked();

private:
    Ui::MainWindow *ui;
    int         width;
    int         height;

    char* qstringToChar(QString srcString);
    MajorImageProcessingThread *imageprocessthread;
};

#endif // MAINWINDOW_H
