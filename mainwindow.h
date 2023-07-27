#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"v4l2.h"
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
    void clickQuitButton(void);
    void clickPhotoButton(void);
    bool new_frame_display_4_rgb(QImage image);

private:
    Ui::MainWindow *ui;
    int         width;
    int         height;
    char* qstringToChar(QString srcString);
    MajorImageProcessingThread *imageprocessthread;
};

#endif // MAINWINDOW_H
