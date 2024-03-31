#include <globalapplication.h>
#include <QDebug>
#include <string>
#include <QCommandLineParser>

GlobalApplication::GlobalApplication(int argc, char *argv[]):QApplication(argc, argv)
{
    bool ok;
    QString option1Value;
    QString option2Value;
    int option3Value;

    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
#if 1
    QCommandLineOption sensor_wk_mode_opt({"m", "mode"}, "Work mode for the sensor", "mode");
    QCommandLineOption sensor_type_opt({"t", "type"}, "Type of the sensor", "type");
    QCommandLineOption save_frame_cnt_opt({"s", "save"}, "Number of frames to save", "count");
#else
    QCommandLineOption sensor_wk_mode_opt({"m", "mode"});
    QCommandLineOption sensor_type_opt({"t", "type"});
    QCommandLineOption save_frame_cnt_opt({"s", "save"});

    sensor_wk_mode_opt.setDescription("select work mode: PCM PHR FHR NV12 YUYV.");
    sensor_type_opt.setDescription("select sensor type: RGB DTOF.");
    save_frame_cnt_opt.setDescription("frame count to be saved: < 100.");
#endif

    parser.addOption(sensor_wk_mode_opt);
    parser.addOption(sensor_type_opt);
    parser.addOption(save_frame_cnt_opt);

    //parser.process(qApp->arguments());
    parser.process(*this);

    DBG_INFO( "---------------");
    if (parser.isSet(sensor_wk_mode_opt)) {
        DBG_INFO( "---------------");
        option1Value = parser.value(sensor_wk_mode_opt);
        selected_wk_mode = string_2_workmode(option1Value);
    }
    else {
        selected_wk_mode = DEFAULT_WORK_MODE;
    }

    if (parser.isSet(sensor_type_opt)) {
        option2Value = parser.value(sensor_type_opt);
        selected_sensor_type = string_2_sensortype(option2Value);
    }
    else {
        selected_sensor_type = DEFAULT_SENSOR_TYPE;
    }

    if (parser.isSet(save_frame_cnt_opt)) {
        option3Value = parser.value(save_frame_cnt_opt).toInt(&ok);
        if(!ok) {
            DBG_ERROR("Failed to convert save frame count to int. Using default value of 1.");
            option3Value = DEFAULT_SAVE_FRAME_CNT;
        }
        save_frame_cnt = option3Value;
    }
    else {
        save_frame_cnt = DEFAULT_SAVE_FRAME_CNT;
    }

    DBG_INFO( "-----selected_wk_mode:%d selected_sensor_type:%d, save_frame_cnt:%d----------", selected_wk_mode, selected_sensor_type,save_frame_cnt);
}

sensor_workmode GlobalApplication::get_wk_mode()
{
    return selected_wk_mode;
}

sensortype GlobalApplication::get_sensor_type()
{
    return selected_sensor_type;
}

int GlobalApplication::get_save_cnt()
{
    return save_frame_cnt;
}

sensortype GlobalApplication::string_2_sensortype(QString& str)
{
    if (!str.compare("RGB"))
        return SENSOR_TYPE_RGB;
    else
        return SENSOR_TYPE_DTOF;
}

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
