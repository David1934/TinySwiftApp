#include <QLCDNumber>
#include <QTimer>
#include <QTime>
#include <globalapplication.h>

#include "common.h"
#include "mainwindow.h"
#include "majorimageprocessingthread.h"
#include "ui_mainwindow.h"

#define UNUSED(X) (void)X

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    int ret = 0;
    char AppNameVersion[128];
    char auto_test_times_string[32];

    ui->setupUi(this);
    sprintf(AppNameVersion, "%s %s for swift %s module by QT %s @ %s,%s",
        APP_NAME,
        APP_VERSION,
#if defined(CONFIG_ADAPS_SWIFT_FLOOD)
        "Flood",
#else
        "Spot",
#endif
        QT_VERSION_STR,
        __DATE__, __TIME__);
    this->setWindowTitle(AppNameVersion);
    DBG_NOTICE("AppVersion: %s\n", AppNameVersion);
    //DBG_NOTICE("QT_VERSION_STR: %s, qVersion(): %s...", QT_VERSION_STR, qVersion());
    // NOTICE: <mainwindow.cpp-MainWindow() 26> QT_VERSION_STR: 5.15.8, qVersion(): 5.15.2...

    // 设置QLCDNumber的显示属性
    ui->lcdNumber->setDigitCount(8); // 显示格式为HH:MM:SS
    ui->lcdNumber->display(QTime::currentTime().toString(RTCTIME_DISPLAY_FMT));

    // 创建一个定时器，每秒更新时间
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    timer->start(1000); // 设置定时器每1000毫秒触发一次

    connect(ui->quitButton, SIGNAL(clicked()), this, SLOT(clickQuitButton()));

    if (true == qApp->get_qt_ui_test())
        return ;

    imageprocessthread = new MajorImageProcessingThread;

    connect(imageprocessthread, SIGNAL(newFrameReady4Display(QImage)),
            this, SLOT(new_frame_display(QImage)));
    connect(imageprocessthread, SIGNAL(update_runtime_display(int, unsigned long)),
            this, SLOT(update_streaming_info(int, unsigned long)));
    connect(ui->skipFrameProcessCheckbox, &QCheckBox::stateChanged, this, &MainWindow::on_skipFrameProcessCheckbox_stateChanged);
    connect(imageprocessthread, SIGNAL(threadEnd(int)), this, SLOT(onThreadEnd(int)));

    ret = imageprocessthread->init(0);
    if (ret < 0)
    {
        DBG_ERROR("Fail to imageprocessthread init()...");
        ui->mainlabel->setText("Fail to imageprocessthread init()...");
        return;
    }

    tested_times = 0;
    to_test_times = qApp->get_timer_test_times();
    if (to_test_times > 0)
    {
        test_timer = new QTimer(this);
        connect(test_timer, &QTimer::timeout, this, &MainWindow::simulateButtonClick);
    }

    sprintf(auto_test_times_string, "%d/%d", tested_times, to_test_times);
    ui->AutoTestTimes_value->setText(auto_test_times_string);

    imageprocessthread->start();

    if (to_test_times > 0)
    {
        test_timer->start(1000 * TIMER_TEST_INTERVAL);
    }
}

MainWindow::~MainWindow()
{
    delete imageprocessthread;
    delete ui;
}

void MainWindow::onThreadEnd(int stop_request_code)
{
    switch (stop_request_code) {
        case STOP_REQUEST_RGB:
            imageprocessthread->mode_switch("RGB");
            imageprocessthread->init(0);
            imageprocessthread->start();
            break;

        case STOP_REQUEST_PHR:
            imageprocessthread->mode_switch("PHR");
            imageprocessthread->init(0);
            imageprocessthread->start();
            break;

        case STOP_REQUEST_PCM:
            imageprocessthread->mode_switch("PCM");
            imageprocessthread->init(0);
            imageprocessthread->start();
            break;

        case STOP_REQUEST_FHR:
            imageprocessthread->mode_switch("FHR");
            imageprocessthread->init(0);
            imageprocessthread->start();
            break;

        case STOP_REQUEST_QUIT:
            this->close();
            break;

        case STOP_REQUEST_STOP:
            //ui->mainlabel->setText("Device is stopped");
        break;

        default:
            ui->mainlabel->setText("Device is ready");
            break;
    }

}

void MainWindow::updateTime()
{
    // 获取当前时间，并更新QLCDNumber控件显示
    QTime currentTime = QTime::currentTime();
    ui->lcdNumber->display(currentTime.toString(RTCTIME_DISPLAY_FMT));
}

void MainWindow::clickQuitButton(void)
{
    if (true == qApp->get_qt_ui_test())
    {
        //this->close();
        qApp->exit(0);
    }
    else {
        imageprocessthread->stop(STOP_REQUEST_QUIT);
    }
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
        ui->mainlabel->setText("NO IMAGE TO BE DISPLAYED!");
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
    imageprocessthread->stop(STOP_REQUEST_STOP);
}

void MainWindow::on_RGBButton_clicked()
{
    if(imageprocessthread->isRunning())
    {
        imageprocessthread->stop(STOP_REQUEST_RGB);
    }
    else {
        imageprocessthread->mode_switch("RGB");
        imageprocessthread->init(0);
        imageprocessthread->start();
    }
}

void MainWindow::on_PHRButton_clicked()
{
    if(imageprocessthread->isRunning())
    {
        imageprocessthread->stop(STOP_REQUEST_PHR);
    }
    else {
        imageprocessthread->mode_switch("PHR");
        imageprocessthread->init(0);
        imageprocessthread->start();
    }
}


void MainWindow::on_PCMButton_clicked()
{
    if(imageprocessthread->isRunning())
    {
        imageprocessthread->stop(STOP_REQUEST_PCM);
    }
    else {
        imageprocessthread->mode_switch("PCM");
        imageprocessthread->init(0);
        imageprocessthread->start();
    }
}

void MainWindow::on_FHRButton_clicked()
{
    if(imageprocessthread->isRunning())
    {
        imageprocessthread->stop(STOP_REQUEST_FHR);
    }
    else {
        imageprocessthread->mode_switch("FHR");
        imageprocessthread->init(0);
        imageprocessthread->start();
    }
}

void MainWindow::simulateButtonClick()
{
    char auto_test_times_string[32];

    if (tested_times >= to_test_times)
    {
        DBG_NOTICE("\n------Timer test done------test times: %d/%d---\n",
            tested_times, to_test_times);
        test_timer->stop();
        return;
    }

    DBG_NOTICE("\n------Timer testing------test times: %d/%d, isRunning: %d---\n", tested_times, to_test_times, imageprocessthread->isRunning());
    if(imageprocessthread->isRunning())
    {
        tested_times++;
        sprintf(auto_test_times_string, "%d/%d", tested_times, to_test_times);
        ui->AutoTestTimes_value->setText(auto_test_times_string);
        
        on_stopButton_clicked();
    }
    else {
        on_FHRButton_clicked();
    }
}

void MainWindow::on_skipFrameProcessCheckbox_stateChanged(bool checked)
{
#if 1
    imageprocessthread->set_skip_frame_process(checked);
#else
    int ret = 0;
    if (checked) {
        ret = setenv(ENV_VAR_SKIP_FRAME_PROCESS, "true", 0);
        if (0 != ret) {
            DBG_ERROR("Fail to set environment variable %s, ret:%d errno: %s (%d)...", ENV_VAR_SKIP_FRAME_PROCESS, ret,
                strerror(errno), errno);
        }
    } else {
        ret = unsetenv(ENV_VAR_SKIP_FRAME_PROCESS);
        if (0 != ret) {
            DBG_ERROR("Fail to unset environment variable %s, ret:%d errno: %s (%d)...", ENV_VAR_SKIP_FRAME_PROCESS, ret,
                strerror(errno), errno);
        }
    }
#endif
}

