#ifndef GLOBALAPPLICATION_H
#define GLOBALAPPLICATION_H
#include <QObject>
#include <QApplication>
#include <QCoreApplication>
#include <common.h>

class GlobalApplication;
#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<GlobalApplication *>(QCoreApplication::instance()))

class GlobalApplication : public QApplication
{
    Q_OBJECT
public:
    GlobalApplication(int argc, char *argv[]);
    sensor_workmode get_wk_mode();
    sensortype get_sensor_type();
    int get_save_cnt();

private:

    sensortype      selected_sensor_type;
    sensor_workmode selected_wk_mode;
    int             save_frame_cnt;

    sensortype string_2_sensortype(QString& str);
    sensor_workmode string_2_workmode(QString& str);
};
#endif // GLOBALAPPLICATION_H
