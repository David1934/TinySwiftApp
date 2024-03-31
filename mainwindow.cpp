#include "mainwindow.h"
#include "majorimageprocessingthread.h"
#include "ui_mainwindow.h"
#include <QLCDNumber>
#include <QTimer>
#include <QTime>

#define UNUSED(X) (void)X

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    char AppNameVersion[64];

    ui->setupUi(this);
    sprintf(AppNameVersion, "%s %s", APP_NAME, APP_VERSION);
    this->setWindowTitle(AppNameVersion);

    // 设置QLCDNumber的显示属性
    ui->lcdNumber->setDigitCount(8); // 显示格式为HH:MM:SS
    ui->lcdNumber->display(QTime::currentTime().toString(RTCTIME_DISPLAY_FMT));

    // 创建一个定时器，每秒更新时间
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    timer->start(1000); // 设置定时器每1000毫秒触发一次
    imageprocessthread = new MajorImageProcessingThread;

    connect(ui->quitButton, SIGNAL(clicked()), this, SLOT(clickQuitButton()));
    connect(imageprocessthread, SIGNAL(newFrameReady4Display(QImage)),
            this, SLOT(new_frame_display(QImage)));
    connect(imageprocessthread, SIGNAL(update_runtime_display(int, unsigned long)),
            this, SLOT(update_streaming_info(int, unsigned long)));

    imageprocessthread->init(0);
    imageprocessthread->start();
}

MainWindow::~MainWindow()
{
    delete imageprocessthread;
    delete ui;
}

void MainWindow::updateTime()
{
    // 获取当前时间，并更新QLCDNumber控件显示
    QTime currentTime = QTime::currentTime();
    ui->lcdNumber->display(currentTime.toString(RTCTIME_DISPLAY_FMT));
}

void MainWindow::clickQuitButton(void)
{
    imageprocessthread->stop();
    this->close();
}

/* QString 转 char* */
char * MainWindow::qstringToChar(QString srcString)
{
    if (srcString.isEmpty()) {
        return NULL;
    }

    QByteArray      ba  = srcString.toLatin1();
    char *          destCharArray;

    destCharArray           = ba.data();
    return destCharArray;
}

bool MainWindow::update_streaming_info(int fps, unsigned long streamed_time_us)
{
    char fps_string[32];
    char time_string[32];
    unsigned int streamed_time_seconds = streamed_time_us / 1000000;

    sprintf(fps_string, "%d fps", fps);
    ui->fpsLabel->setText(fps_string);

    sprintf(time_string, "%02d:%02d:%02d", streamed_time_seconds/3600, streamed_time_seconds/60, streamed_time_seconds%60);
    ui->strmTimeValueLabel->setText(time_string);

    return true;
}

bool MainWindow::new_frame_display(QImage image)
{
    ui->mainlabel->clear();
    if(image.isNull())
    {
        ui->mainlabel->setText("画面丢失！");
    }
    else
    {
        image = image.scaled(ui->mainlabel->width(),ui->mainlabel->height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        QPixmap pix = QPixmap::fromImage(image,Qt::AutoColor);
        ui->mainlabel->setPixmap(pix);

    }

    return true;
}

void MainWindow::on_stopButton_clicked()
{
    imageprocessthread->stop();
    ui->mainlabel->setText("Device is ready");

}

void MainWindow::on_RGBButton_clicked()
{
    if(imageprocessthread->isRunning())
    {
        imageprocessthread->stop();
    }
    imageprocessthread->change("RGB");
    imageprocessthread->init(0);
    imageprocessthread->start();

}

void MainWindow::on_PHRButton_clicked()
{
    if(imageprocessthread->isRunning())
    {
        imageprocessthread->stop();
    }
    imageprocessthread->change("PHR");
    imageprocessthread->init(0);
    imageprocessthread->start();
}


void MainWindow::on_PCMButton_clicked()
{
    if(imageprocessthread->isRunning())
    {
        imageprocessthread->stop();
    }
    imageprocessthread->change("PCM");
    imageprocessthread->init(0);
    imageprocessthread->start();
}

void MainWindow::on_FHRButton_clicked()
{
    if(imageprocessthread->isRunning())
    {
        imageprocessthread->stop();
    }
    imageprocessthread->change("FHR");
    imageprocessthread->init(0);
    imageprocessthread->start();
}

