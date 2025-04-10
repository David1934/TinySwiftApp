#include <globalapplication.h>
#include <QDebug>
#include <string>
#include <QCommandLineParser>

#if !defined(NO_UI_APPLICATION)
GlobalApplication::GlobalApplication(int argc, char *argv[]):QApplication(argc, argv)
#else
GlobalApplication::GlobalApplication(int argc, char *argv[]):QCoreApplication(argc, argv)
#endif
{
    bool ok;
    QString option1Value;
    QString option2Value;
    int option3Value;
    int option4Value;

    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption sensor_wk_mode_opt(QStringList() << "m" << "mode", "Work mode for the sensor (PCM PHR FHR NV12 YUYV)", "mode");
    QCommandLineOption sensor_type_opt(QStringList() << "t" << "type", "Type of the sensor (RGB DTOF)", "type");
    QCommandLineOption save_frame_cnt_opt(QStringList() << "s" << "save", "Number of frames to save (>=0)", "count");
    QCommandLineOption timer_test_times_opt(QStringList() << "times", "times to be tested by timer (>=0)", "count");
    QCommandLineOption qt_ui_test_opt(QStringList() << "qttest", "a simple UI display test with QT");
#if defined(RUN_ON_ROCKCHIP)
    QCommandLineOption environment_type_opt(QStringList() << "e" << "etype", "Type of the environment (Indoor Outdoor)", "etype");
    QCommandLineOption measurement_type_opt(QStringList() << "m" << "mtype", "Type of the measurement (Normal Short Full)", "mtype");
    //QCommandLineOption no_host_comm_opt(QStringList() << "nohostcomm", "local run mode without host communication");
    //QCommandLineOption output_data_type_opt(QStringList() << "o" << "otype", "Type of the output data type (Raw Grayscale Depth16 Depth16XY PointCloud) to host", "otype");
#endif

    parser.addOption(sensor_wk_mode_opt);
    parser.addOption(sensor_type_opt);
    parser.addOption(save_frame_cnt_opt);
    parser.addOption(timer_test_times_opt);
    parser.addOption(qt_ui_test_opt);
#if defined(RUN_ON_ROCKCHIP)
    parser.addOption(environment_type_opt);
    parser.addOption(measurement_type_opt);
    //parser.addOption(no_host_comm_opt);
    //parser.addOption(output_data_type_opt);
#endif

    //parser.process(qApp->arguments());
    parser.process(*this);
    qt_ui_test = false;
    timer_test_times = 0;
    selected_sensor_type = DEFAULT_SENSOR_TYPE;
    selected_wk_mode = DEFAULT_WORK_MODE;
    save_frame_cnt = DEFAULT_SAVE_FRAME_CNT;
    timer_test_times = DEFAULT_TIMER_TEST_TIMES;
    qt_ui_test = false;
#if defined(RUN_ON_ROCKCHIP)
    selected_e_type = DEFAULT_ENVIRONMENT_TYPE;
    selected_m_type = DEFAULT_MEASUREMENT_TYPE;
#endif

    //DBG_INFO( "---------------");
    if (parser.isSet(sensor_wk_mode_opt)) {
        //DBG_INFO( "---------------");
        option1Value = parser.value(sensor_wk_mode_opt);
        selected_wk_mode = string_2_workmode(option1Value);
    }

    if (parser.isSet(sensor_type_opt)) {
        option2Value = parser.value(sensor_type_opt);
        selected_sensor_type = string_2_sensortype(option2Value);
    }

#if defined(RUN_ON_ROCKCHIP)
    if (parser.isSet(environment_type_opt)) {
        option2Value = parser.value(environment_type_opt);
        selected_e_type = string_2_environmenttype(option2Value);
    }

    if (parser.isSet(measurement_type_opt)) {
        option2Value = parser.value(measurement_type_opt);
        selected_m_type = string_2_measurementtype(option2Value);
    }
#endif

    if (parser.isSet(save_frame_cnt_opt)) {
        option3Value = parser.value(save_frame_cnt_opt).toInt(&ok);
        if(!ok) {
            DBG_ERROR("Failed to convert save frame count to int. Using default value of 0.");
            option3Value = DEFAULT_SAVE_FRAME_CNT;
        }
        save_frame_cnt = option3Value;
    }

    if (parser.isSet(timer_test_times_opt)) {
        option4Value = parser.value(timer_test_times_opt).toInt(&ok);
        if(!ok) {
            DBG_ERROR("Failed to convert timer test count to int. Using default value of 1.");
            option4Value = DEFAULT_TIMER_TEST_TIMES;
        }
        timer_test_times = option4Value;
    }

    if (parser.isSet(qt_ui_test_opt)) {
        qt_ui_test = true;
    }

    DBG_INFO( "-----selected_wk_mode:%d selected_sensor_type:%d, save_frame_cnt:%d----------", selected_wk_mode, selected_sensor_type,save_frame_cnt);
}

sensor_workmode GlobalApplication::get_wk_mode()
{
    return selected_wk_mode;
}

int GlobalApplication::set_wk_mode(int wk_mode)
{
    int ret = 0;

    if (SENSOR_TYPE_DTOF == selected_sensor_type)
    {
        if (wk_mode >= WK_DTOF_PHR && wk_mode <= WK_DTOF_FHR)
        {
            selected_wk_mode = (sensor_workmode) wk_mode;
        }
        else {
            ret = -1;
        }
    }
    else if (SENSOR_TYPE_RGB == selected_sensor_type)
    {
        if (wk_mode >= WK_RGB_NV12 && wk_mode <= WK_RGB_YUYV)
        {
            selected_wk_mode = (sensor_workmode) wk_mode;
        }
        else {
            ret = -1;
        }
    }
    else {
        ret = -1;
    }


    return ret;
}

#if defined(RUN_ON_ROCKCHIP)
AdapsEnvironmentType GlobalApplication::get_environment_type()
{
    return selected_e_type;
}

int GlobalApplication::set_environment_type(int environment_type)
{
    int ret = 0;

    if (AdapsEnvTypeIndoor == environment_type || AdapsEnvTypeOutdoor == environment_type)
    {
        selected_e_type = (AdapsEnvironmentType) environment_type;
    }
    else {
        ret = -1;
    }

    return ret;
}

AdapsMeasurementType GlobalApplication::get_measurement_type()
{
    return selected_m_type;
}

int GlobalApplication::set_measurement_type(int mtype)
{
    int ret = 0;

    if (mtype >= AdapsMeasurementTypeNormal && mtype <= AdapsMeasurementTypeFull)
    {
        selected_m_type = (AdapsMeasurementType) mtype;
    }
    else {
        ret = -1;
    }

    return ret;
}
#endif

sensortype GlobalApplication::get_sensor_type()
{
    return selected_sensor_type;
}

int GlobalApplication::set_sensor_type(int sensor_type)
{
    int ret = 0;

    if (SENSOR_TYPE_RGB == sensor_type || SENSOR_TYPE_DTOF == sensor_type)
    {
        selected_sensor_type = (sensortype) sensor_type;
    }
    else {
        ret = -1;
    }

    return ret;
}

int GlobalApplication::get_save_cnt()
{
    return save_frame_cnt;
}

int GlobalApplication::get_timer_test_times()
{
    return timer_test_times;
}

bool GlobalApplication::get_qt_ui_test()
{
    return qt_ui_test;
}

sensortype GlobalApplication::string_2_sensortype(QString& str)
{
    if (!str.compare("RGB"))
        return SENSOR_TYPE_RGB;
    else
        return SENSOR_TYPE_DTOF;
}

#if defined(RUN_ON_ROCKCHIP)
AdapsEnvironmentType GlobalApplication::string_2_environmenttype(QString& str)
{
    if (!str.compare("Outdoor"))
        return AdapsEnvTypeOutdoor;
    else
        return AdapsEnvTypeIndoor;
}

AdapsMeasurementType GlobalApplication::string_2_measurementtype(QString& str)
{
    if (!str.compare("Short"))
        return AdapsMeasurementTypeShort;
    else if (!str.compare("Normal"))
        return AdapsMeasurementTypeNormal;
    else
        return AdapsMeasurementTypeFull;
}
#endif

sensor_workmode GlobalApplication::string_2_workmode(QString& str)
{
    if (!str.compare("PCM"))
        return WK_DTOF_PCM;
    else if (!str.compare("FHR"))
        return WK_DTOF_FHR;
    else if (!str.compare("NV12"))
        return WK_RGB_NV12;
    else if (!str.compare("YUYV"))
        return WK_RGB_YUYV;
    else
        return WK_DTOF_PHR;
}
