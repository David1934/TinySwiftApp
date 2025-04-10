#ifndef GLOBALAPPLICATION_H
#define GLOBALAPPLICATION_H
#include <QObject>
#if !defined(NO_UI_APPLICATION)
#include <QApplication>
#endif
#include <QCoreApplication>
#include <common.h>

class GlobalApplication;

#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<GlobalApplication *>(QCoreApplication::instance()))

#if !defined(NO_UI_APPLICATION)
class GlobalApplication : public QApplication
#else
class GlobalApplication : public QCoreApplication
#endif
{
    Q_OBJECT
public:
    GlobalApplication(int argc, char *argv[]);
    sensor_workmode get_wk_mode();
    sensortype get_sensor_type();
    int get_save_cnt();
    int get_timer_test_times();
    bool get_qt_ui_test();
    int set_wk_mode(int wk_mode);
    int set_sensor_type(int sensortype);
#if defined(RUN_ON_ROCKCHIP)
    AdapsEnvironmentType get_environment_type();
    int set_environment_type(int environment_type);
    AdapsMeasurementType get_measurement_type();
    int set_measurement_type(int mtype);
#endif

private:

    sensortype      selected_sensor_type;
    sensor_workmode selected_wk_mode;
    int             save_frame_cnt;
    int             timer_test_times;
    bool            qt_ui_test;
#if defined(RUN_ON_ROCKCHIP)
    AdapsMeasurementType selected_m_type;
    AdapsEnvironmentType selected_e_type;
    //bool            no_host_comm;

    AdapsMeasurementType string_2_measurementtype(QString& str);
    AdapsEnvironmentType string_2_environmenttype(QString& str);
#endif
    sensortype string_2_sensortype(QString& str);
    sensor_workmode string_2_workmode(QString& str);
};
#endif // GLOBALAPPLICATION_H
