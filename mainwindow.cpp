#include <signal.h>
#if !defined(NO_UI_APPLICATION)
#include <QTime>
#include <QTimer>
#include <QShortcut>
//#include <QMessageBox>
#include <QCloseEvent>
#include <QLCDNumber>
#include <QScreen>
#include <QMouseEvent>
#include <QSize>
#endif

#include "globalapplication.h"
#include "common.h"
#include "mainwindow.h"
#include "FrameProcessThread.h"
#if !defined(NO_UI_APPLICATION)
#include "ui_mainwindow.h"
#endif

#define UNUSED(X) (void)X
#define QStringToCharPtr(qstr) (qstr.toUtf8().constData())


#if !defined(NO_UI_APPLICATION)
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
#else
MainWindow::MainWindow()
#endif
{
    char AppNameVersion[128];
    int w;
    int h;

    firstDisplayFrameTimeUsec = 0;
    displayedFrameCnt = 0;
    displayed_fps = 0;
    frame_process_thread = NULL;
#if !defined(NO_UI_APPLICATION)
    ui->setupUi(this);
#endif
    sprintf(AppNameVersion, "%s %s built at %s,%s"
        ,APP_NAME,
        APP_VERSION,
        __DATE__, __TIME__
        );
#if !defined(NO_UI_APPLICATION)
    this->setWindowTitle(AppNameVersion);
#endif
    DBG_NOTICE("AppVersion: %s, Build on QT version: %s, Runtime lib QT version: %s...",
        AppNameVersion, QT_VERSION_STR, qVersion());

#if !defined(NO_UI_APPLICATION)
    w = this->width();
    h = this->height();
    DBG_INFO("MainWindow resolution: %d X %d", w, h);

    // 设置QLCDNumber的显示属性
    ui->rtcTime->setDigitCount(8); // 显示格式为HH:MM:SS
    ui->rtcTime->display(QTime::currentTime().toString(RTCTIME_DISPLAY_FMT));

    // 创建一个定时器，每秒更新时间
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateRTCtime);
    timer->start(1000); // 设置定时器每1000毫秒触发一次

    connect(ui->quitButton, SIGNAL(clicked()), this, SLOT(Quit()));

    if (true == qApp->get_qt_ui_test())
        return ;

    ui->startStopButton->setText("Start");

#if defined(RUN_ON_ROCKCHIP)
    ui->groupBox_rgb_wkmode->setEnabled(false);
#else
    ui->groupBox_dtof_wkmode->setEnabled(false);
    ui->groupBox_dtof_environment->setEnabled(false);
    ui->confidence_bitmap->setVisible(false);
#endif
#endif

    startFrameProcessThread();
}

MainWindow::~MainWindow()
{
    DBG_INFO("frame_process_thread:%p...", frame_process_thread);

    if (NULL != frame_process_thread)
    {
        bool isRunning = frame_process_thread->isRunning();
        bool isFinished = frame_process_thread->isFinished();
        bool isSleeping = frame_process_thread->isSleeping();

        DBG_INFO("frame_process_thread isRunning():%d, isFinished(): %d, isSleeping: %d",
            isRunning,
            isFinished,
            isSleeping
            );
        if (frame_process_thread->isRunning())
        {
            frame_process_thread->exit(0);
            //frame_process_thread->wait(WAIT_TIME_4_THREAD_EXIT); // unit is milliseconds
        }
        delete frame_process_thread;
    }
#if !defined(NO_UI_APPLICATION)
    delete ui;
#endif
}

void MainWindow::startFrameProcessThread(void)
{
    int ret = 0;
    char auto_test_times_string[32];
    sensortype sensor_type = qApp->get_sensor_type();
    sensor_workmode wk_mode = qApp->get_wk_mode();

    if (SENSOR_TYPE_DTOF == sensor_type)
    {
        if (wk_mode >= WK_DTOF_PHR && wk_mode <= WK_DTOF_FHR)
        {
            // okay, continue
        }
        else {
            // No correct work mode is selected, exit.
            return;
        }
    }
    else if (SENSOR_TYPE_RGB == sensor_type)
    {
        if (wk_mode >= WK_RGB_NV12 && wk_mode <= WK_RGB_YUYV)
        {
            // okay, continue
        }
        else {
            // No correct work mode is selected, exit.
            return;
        }
    }
    else {
        // No correct sensor type is selected, exit.
        return;
    }

    frame_process_thread = new FrameProcessThread;
    ret = frame_process_thread->init(0);
    if (ret < 0)
    {
        DBG_ERROR("Fail to frame_process_thread init()...");
#if !defined(NO_UI_APPLICATION)
        ui->mainlabel->setText("Fail to frame_process_thread init(),\nPlease check camera is ready or not?");
#endif
        delete frame_process_thread;
        frame_process_thread = NULL;
        //QMessageBox::information(nullptr, "Warning Prompt", "No camera is detected, please double check!");
        return;
    }

    connect(frame_process_thread, SIGNAL(threadEnd(int)), this, SLOT(onThreadEnd(int)));

#if !defined(NO_UI_APPLICATION)
    qRegisterMetaType<enum frame_data_type>("enum frame_data_type");
    qRegisterMetaType<watchPointInfo_t>("watchPointInfo_t");
    connect(frame_process_thread, SIGNAL(updateWatchSpotInfo(QPoint , enum frame_data_type, watchPointInfo_t)), 
        this, SLOT(watchSpotInfoUpdate(QPoint , enum frame_data_type, watchPointInfo_t)));
    connect(frame_process_thread, SIGNAL(newFrameReady4Display(QImage, QImage)),
            this, SLOT(new_frame_display(QImage, QImage)));
    qRegisterMetaType<status_params2>("status_params2");
    connect(frame_process_thread, SIGNAL(update_runtime_display(status_params2)),
            this, SLOT(update_status_info(status_params2)));

    // 创建一个QShortcut对象，将Ctrl + C组合键与槽函数onCtrlCPressed关联起来
    QShortcut *shortcut_ctrlX = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_X), this);
    connect(shortcut_ctrlX, &QShortcut::activated, this, &MainWindow::onCtrl_X_Pressed);

    QShortcut *shortcut_ctrlS = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this);
    connect(shortcut_ctrlS, &QShortcut::activated, this, &MainWindow::onCtrl_S_Pressed);

    ui->mainlabel->installEventFilter(this);

    tested_times = 0;
    to_test_times = qApp->get_timer_test_times();
    if (to_test_times > 0)
    {
        test_timer = new QTimer(this);
        connect(test_timer, &QTimer::timeout, this, &MainWindow::simulateButtonClick);
    }

    sprintf(auto_test_times_string, "%d/%d", tested_times, to_test_times);
    ui->AutoTestTimes_value->setText(auto_test_times_string);
#endif

    frame_process_thread->start();

#if !defined(NO_UI_APPLICATION)
    if (to_test_times > 0)
    {
        test_timer->start(1000 * TIMER_TEST_INTERVAL);
    }
    ui->startStopButton->setText("Stop");
#endif
}

void MainWindow::onThreadEnd(int stop_request_code)
{
#if !defined(NO_UI_APPLICATION)
    switch (stop_request_code) {
        case STOP_REQUEST_RGB:
            break;

        case STOP_REQUEST_PHR:
            break;

        case STOP_REQUEST_PCM:
            break;

        case STOP_REQUEST_FHR:
            break;

        case STOP_REQUEST_QUIT:
            this->close();
            break;

        case STOP_REQUEST_STOP:
            delete frame_process_thread;
            frame_process_thread = NULL;
            ui->mainlabel->setText("Device is stopped");
        break;

        default:
            ui->mainlabel->setText("Device is ready");
            break;
    }
#endif

}

void MainWindow::Quit(void)
{
    if (true == qApp->get_qt_ui_test())
    {
        //this->close();
        qApp->exit(0);
    }
    else {
        if (NULL != frame_process_thread)
        {
            bool isRunning = frame_process_thread->isRunning();
            bool isFinished = frame_process_thread->isFinished();
            bool isSleeping = frame_process_thread->isSleeping();

            DBG_INFO("frame_process_thread isRunning():%d, isFinished(): %d, isSleeping: %d",
                isRunning,
                isFinished,
                isSleeping
                );
            if (isRunning)
            {
                frame_process_thread->stop(STOP_REQUEST_QUIT);
            }
            else {
                if (isFinished)
                {
#if !defined(NO_UI_APPLICATION)
                    this->close(); // frame_process_thread->exit(1);
#else
                    qApp->exit(0);
#endif
                }
            }
        }
        else {
            //this->close();
            qApp->exit(0);
        }
    }
}

void MainWindow::unixSignalHandler(int signal)
{
    switch (signal) {
        case SIGTSTP:
            DBG_NOTICE("CTRL-Z recieved, screen shot will be excuted!");
#if !defined(NO_UI_APPLICATION)
            captureAndSaveScreenshot();
#endif
            break;

        case SIGUSR1:
            DBG_ERROR("User Signal 1 recieved\nPls check kernel log for detailed info!");
#if !defined(NO_UI_APPLICATION)
            ui->mainlabel->setText("User Signal 1 recieved\nPls check kernel log for detailed info!");
#endif
            if (NULL != frame_process_thread)
            {
                frame_process_thread->stop(STOP_REQUEST_STOP);
            }
            break;

        case SIGUSR2:
            DBG_ERROR("User Signal 2 recieved\nPls check kernel log for detailed info!");
#if !defined(NO_UI_APPLICATION)
            ui->mainlabel->setText("User Signal 2 recieved\nPls check kernel log for detailed info!");
#endif
            if (NULL != frame_process_thread)
            {
                frame_process_thread->stop(STOP_REQUEST_STOP);
            }
            break;

        case SIGINT:
            DBG_NOTICE("CTRL-C recieved, graceful close() is executed!");
            Quit();
            break;

#if 0 // handled in main.cpp
        case SIGSEGV:
        case SIGABRT:
        case SIGBUS:
        case SIGTERM:
            DBG_ERROR("graceful close() is executed!");
            Quit();
            break;
#endif

        default:
            break;
    }

    return;
}

#if !defined(NO_UI_APPLICATION)
void MainWindow::watchSpotInfoUpdate(  QPoint spot, enum frame_data_type ftype, watchPointInfo_t wpi)
{
    char temp_string[64];
    const char *confidence_level_names[] =
    {
        "High",
        "Low",
        "Un-defined",
        "Medium",
    };

    switch (ftype) 
    {
        case FDATA_TYPE_YUYV:
        case FDATA_TYPE_NV12:
            sprintf(temp_string, "P(%d, %d) r: %d, g: %d, b: %d", 
                spot.x(), spot.y(), wpi.red, wpi.green, wpi.blue);
            break;

        case FDATA_TYPE_DTOF_RAW_DEPTH:
            sprintf(temp_string, "P(%d, %d): %d mm (%s)", 
                spot.x(), spot.y(), wpi.distance, confidence_level_names[wpi.confidence]);
            break;

        default:
            break;
    }

    ui->spot_depth->setText(temp_string);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->mainlabel && event->type() == QEvent::MouseButtonPress)
    {
        QSize labelSize = ui->mainlabel->size();
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint pos = mouseEvent->pos();
        DBG_INFO("pos (%d, %d) label.size(): %d X %d", 
            pos.x(), pos.y(), labelSize.width(), labelSize.height());
        frame_process_thread->setWatchSpot(labelSize, pos);

        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::updateRTCtime()
{
    // 获取当前时间，并更新QLCDNumber控件显示
    QTime currentTime = QTime::currentTime();
    ui->rtcTime->display(currentTime.toString(RTCTIME_DISPLAY_FMT));
}

void MainWindow::onCtrl_X_Pressed(void)
{
    DBG_INFO("Ctrl + X was pressed.");
    Quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    DBG_INFO("Last chance to do something before exit...");

    // 在这里执行退出前的处理逻辑
    // 例如保存数据、释放资源等

    // 接受关闭事件
    event->accept();
}

bool MainWindow::update_status_info(status_params2 param2)
{
    char temp_string[32];
    const char *work_mode_name[]={
        "PTM-PHR",
        "PCM-Gray",
        "PTM-FHR",
        "RGB-NV12",
        "RGB-YUYV",
    };

    const char *environment_type_name[]={
        "Unknown",
        "Indoor",
        "Outdoor",
    };

    const char *measurement_range_name[]={
        "Unknown",
        "Normal",
        "Short",
        "Full",
    };

    const char *power_mode_names[] =
    {
        "Unknown",
        "Div1",
        "Div3",
    };

    unsigned int streamed_time_seconds = param2.streamed_time_us / 1000000;

    sprintf(temp_string, "%d (%d) fps", displayed_fps, param2.mipi_rx_fps);
    ui->framerate_value->setText(temp_string);

    sprintf(temp_string, "%02d:%02d:%02d", streamed_time_seconds/3600, streamed_time_seconds/60, streamed_time_seconds%60);
    //ui->strmTime_value->setText(temp_string);
    ui->strmTime_value->display(temp_string);

    if (param2.work_mode >= WK_DTOF_PHR && param2.work_mode < WK_COUNT)
    {
        ui->cur_work_mode_value->setText(work_mode_name[param2.work_mode]);
    }
    else {
        ui->cur_work_mode_value->setText("Unknown");
    }

#if defined(RUN_ON_ROCKCHIP)
    if (SENSOR_TYPE_RGB == param2.sensor_type)
    {
        ui->cur_module_type_value->setText("RGB");
    }
    else {
        ui->cur_module_type_value->setText(SWIFT_MODULE_TYPE_NAME);

    #if defined(CONFIG_ADAPS_SWIFT_FLOOD)
        ui->pvddLabel->setVisible(false);
        ui->cur_exp_pvdd_value->setVisible(false);
    #else
        sprintf(temp_string, "%d.%02d V", param2.curr_exp_pvdd/100, param2.curr_exp_pvdd%100);
        ui->cur_exp_pvdd_value->setText(temp_string);
    #endif
    }

    sprintf(temp_string, "%d.%02d ℃", param2.curr_temperature/100, param2.curr_temperature%100);
    ui->cur_temperature_value->setText(temp_string);

    sprintf(temp_string, "-%d.%02d V", param2.curr_exp_vop_abs/100, param2.curr_exp_vop_abs%100);
    ui->cur_exp_vop_value->setText(temp_string);

    if (param2.env_type >= AdapsEnvTypeIndoor && param2.env_type <= AdapsEnvTypeOutdoor)
    {
        ui->cur_environment_value->setText(environment_type_name[param2.env_type]);
    }
    else {
        ui->cur_environment_value->setText("Unknown");
    }

    if (param2.measure_type >= AdapsMeasurementTypeNormal && param2.measure_type <= AdapsMeasurementTypeFull)
    {
        ui->cur_measurement_value->setText(measurement_range_name[param2.measure_type]);
    }
    else {
        ui->cur_measurement_value->setText("Unknown");
    }
#else
    ui->cur_module_type_value->setText("RGB camera");
    sprintf(temp_string, "%s", "NA");
    ui->cur_temperature_value->setText(temp_string);
    ui->cur_exp_vop_value->setText(temp_string);
    ui->cur_exp_pvdd_value->setText(temp_string);
    ui->cur_environment_value->setText(temp_string);
    ui->cur_measurement_value->setText(temp_string);

#endif

    // disable the current mode button
    switch (param2.work_mode) {
        case WK_DTOF_PHR:
            break;

        case WK_DTOF_FHR:
             break;

        case WK_DTOF_PCM:
            break;

        case WK_RGB_NV12:
        case WK_RGB_YUYV:
            break;

        default:
            break;
    }

    return true;
}

bool MainWindow::new_frame_display(QImage image, QImage img4confidence)
{
    struct timeval tv;
    long currTimeUsec;
    unsigned long streamed_timeUs;

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

        if(false == img4confidence.isNull())
        {
            img4confidence = img4confidence.scaled(ui->confidence_bitmap->width(),ui->confidence_bitmap->height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            QPixmap pix = QPixmap::fromImage(img4confidence,Qt::AutoColor);
            ui->confidence_bitmap->setPixmap(pix);
        }
        else {
            ui->confidence_bitmap->setText("NO CONFIDENCE TO BE DISPLAYED!");
        }

        gettimeofday(&tv,NULL);
        displayedFrameCnt++;
        if (1 == displayedFrameCnt)
        {
            firstDisplayFrameTimeUsec = tv.tv_sec*1000000 + tv.tv_usec;
        }
        else {
            currTimeUsec = tv.tv_sec*1000000 + tv.tv_usec;
            streamed_timeUs = (currTimeUsec - firstDisplayFrameTimeUsec);
            displayed_fps = (displayedFrameCnt * 1000000) / streamed_timeUs;
        }
    }

    return true;
}

void MainWindow::on_startStopButton_clicked()
{
    if (NULL != frame_process_thread)
    {
        frame_process_thread->stop(STOP_REQUEST_STOP);
        ui->startStopButton->setText("Start");
    }
    else {
        sensortype sensor_type = qApp->get_sensor_type();

        if (SENSOR_TYPE_DTOF == sensor_type)
        {
            if(ui->radioButton_fhr->isChecked()) {
                qApp->set_wk_mode(WK_DTOF_FHR);
            } else if(ui->radioButton_pcm->isChecked()) {
                qApp->set_wk_mode(WK_DTOF_PCM);
            } else {
                qApp->set_wk_mode(WK_DTOF_PHR);
            }
        }
        else if (SENSOR_TYPE_RGB == sensor_type)
        {
            if(ui->radioButton_yuyv->isChecked()) {
                qApp->set_wk_mode(WK_RGB_YUYV);
            } else {
                qApp->set_wk_mode(WK_RGB_NV12);
            }
        }
        else {
            // No correct sensor type is selected, exit.
            return;
        }

        startFrameProcessThread();
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

    DBG_NOTICE("\n------Timer testing------test times: %d/%d, isRunning: %d---\n", tested_times, to_test_times, frame_process_thread->isRunning());
    if (NULL != frame_process_thread)
    {
        if(frame_process_thread->isRunning())
        {
            tested_times++;
            sprintf(auto_test_times_string, "%d/%d", tested_times, to_test_times);
            ui->AutoTestTimes_value->setText(auto_test_times_string);
            
            on_startStopButton_clicked();
        }
        else {
            on_startStopButton_clicked();
        }
    }
}

void MainWindow::captureAndSaveScreenshot()
{
    //QList<QByteArray> formats = QImageWriter::supportedImageFormats();
    //qCritical() << "Supported image formats:" << formats;

    // 获取当前窗口的屏幕
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        DBG_ERROR("Failed to get screen object.");
        return;
    }

    // 截取当前窗口的截图
    QPixmap screenshot = screen->grabWindow(winId());

    // 获取当前进程ID
    qint64 processId = QCoreApplication::applicationPid();

    // 获取当前日期和时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString dateStr = currentDateTime.toString("yyyyMMdd");
    QString timeStr = currentDateTime.toString("HHmmss");

    // 构建保存路径
    QString savePath = QString(DATA_SAVE_PATH "Screenshot_4_process%1_%2_%3.png")
                           .arg(processId)
                           .arg(dateStr)
                           .arg(timeStr);

    // 保存截图到文件
    if (screenshot.save(savePath, "PNG")) {
        DBG_NOTICE("Save screenshot to %s successfully.", QStringToCharPtr(savePath));
    } else {
        DBG_ERROR("Failed to save screenshot to %s.", QStringToCharPtr(savePath));
    }
}

void MainWindow::onCtrl_S_Pressed(void)
{
    DBG_INFO("Ctrl + S was pressed.");
    captureAndSaveScreenshot();
}
#endif

