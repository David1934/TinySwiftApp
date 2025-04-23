#ifndef GLOBALAPPLICATION_H
#define GLOBALAPPLICATION_H

#include <QObject>
#if !defined(NO_UI_APPLICATION)
	#include <QApplication>
#else
	#include <QCoreApplication>
#endif
#include <common.h>
#include"v4l2.h"
#include "misc_device.h"

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
    V4L2* get_v4l2_instance();
    int register_v4l2_instance(V4L2* new_instance);

#if defined(RUN_ON_EMBEDDED_LINUX)
    Misc_Device* get_misc_dev_instance();
    int register_misc_dev_instance(Misc_Device* new_instance);
    bool is_capture_req_from_host();
    int set_capture_req_from_host(bool val);
    AdapsFramerateType get_framerate_type();
    int set_framerate_type(int framerate_type);
    AdapsEnvironmentType get_environment_type();
    int set_environment_type(int environment_type);
    AdapsMeasurementType get_measurement_type();
    int set_measurement_type(int mtype);

    int get_GrayScaleMinMappedRange();
    int set_GrayScaleMinMappedRange(int value);

    int get_GrayScaleMaxMappedRange();
    int set_GrayScaleMaxMappedRange(int value);

    float get_RealDistanceMinMappedRange();
    int set_RealDistanceMinMappedRange(float value);

    float get_RealDistanceMaxMappedRange();
    int set_RealDistanceMaxMappedRange(float value);
#endif

private:

    sensortype      selected_sensor_type;
    sensor_workmode selected_wk_mode;
    int             save_frame_cnt;
    int             timer_test_times;
    bool            qt_ui_test;
    V4L2            *v4l2_instance;

#if defined(RUN_ON_EMBEDDED_LINUX)
    AdapsMeasurementType selected_m_type;
    AdapsEnvironmentType selected_e_type;
    AdapsFramerateType selected_framerate_type;
    bool            capture_req_from_host;
    int GrayScaleMinMappedRange;
    int GrayScaleMaxMappedRange;
    float RealDistanceMinMappedRange;
    float RealDistanceMaxMappedRange;
    Misc_Device     *misc_dev_instance;

    AdapsMeasurementType string_2_measurementtype(QString& str);
    AdapsEnvironmentType string_2_environmenttype(QString& str);
#endif
    sensortype string_2_sensortype(QString& str);
    sensor_workmode string_2_workmode(QString& str);
};

#endif // GLOBALAPPLICATION_H

