#include <csignal>
#include <unistd.h>
#include <execinfo.h>
#include <cstdlib>
#include <getopt.h>

#include "common.h"
#include "dtof_main.h"

static DToF_Main* s_dToF_main = nullptr;

static void dump_stack()
{
    const int len = 1024;
    void *func[len];
    int size;
    int i;
    char **funs;

    size = backtrace(func, len);
    funs = (char**)backtrace_symbols(func, size);
    if (funs == NULL) {
        DBG_ERROR("backtrace_symbols() fail.\n");
        return;
    }

    DBG_NOTICE("=========> stack trace: %d <=========", size);
    for(i = 0; i < size; ++i)
        DBG_NOTICE("%d %s", i, funs[i]);

    free(funs);
}

static void signal_handle(int signo)
{
    DBG_NOTICE("recieve signal: %d.!!!\n", signo);

    dump_stack();

    if (SIGINT == signo)
    {
        if (NULL != s_dToF_main)
        {
            s_dToF_main->stop();
        }
    }
    else {
        exit(0);
    }

}

static void usage(char *name)
{
    printf("Usage: %s options\n"
        "         --setworkmode,            default 0,            required, sensor work mode (PHR).\n"
        "         --dumpframe,              default 0             optional, how many frames to write files.\n\n"
        "         --module_kernel_type,     default 0             optional, module kernel type for adaps algo lib.\n\n"
        "         --roi_sram_rolling,                                     , enable roisram rolling.\n"
        "         --help,                                                 , print this usage.\n",
        name);
}


static int parse_args(int argc, char **argv, struct sensor_params *ctx)
{
    int c;
    int ret = 0;

    ctx->to_dump_frame_cnt = 0;

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"setworkmode",              required_argument, 0, 'w' },
            {"dumpframe",                required_argument, 0, 'd' },
            {"module_kernel_type",       required_argument, 0, 'k' },
            {"roi_sram_rolling",         no_argument,       0, 'r' },
            {"help",                     no_argument,       0, 'h' },
            {0,                          0,                 0,  0  }
        };

        c = getopt_long(argc, argv, "w:d:k:r:h:", long_options, &option_index);
        if (c == -1)
        {
            break;
        }

        switch (c) {
            case 'w':
                ctx->work_mode = (enum sensor_workmode) atoi(optarg);
                break;

            case 'd':
                ctx->to_dump_frame_cnt = atoi(optarg);
                break;

            case 'k':
                ctx->module_kernel_type = atoi(optarg);
                break;

            case 'r':
                ctx->roi_sram_rolling = atoi(optarg);
                break;

            case '?':
            case 'h':
                usage(argv[0]);
                //exit(-1);
                ret = -1;
                break;

            default:
                DBG_ERROR("?? getopt returned character code 0%o ??\n", c);
        }
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int ret;
    struct sensor_params sns_param;

    DBG_NOTICE("%s\nVersion: %s\nBuild Time: %s,%s\n\n",
        APP_NAME,
        APP_VERSION,
        __DATE__,
        __TIME__
        );

    memset(&sns_param, 0, sizeof(struct sensor_params));

    //parse command arguments
    ret =parse_args(argc, argv, &sns_param);
    if (ret <0)
        return ret;

    switch (sns_param.work_mode)
    {
        case WK_DTOF_FHR:
             sns_param.measure_type = AdapsMeasurementTypeFull;
             sns_param.env_type = AdapsEnvTypeOutdoor;
             sns_param.power_mode = AdapsPowerModeNormal;
             sns_param.framerate_type = AdapsFramerateType30FPS;
             break;

        case WK_DTOF_PCM:
             sns_param.measure_type = AdapsMeasurementTypeFull;
             sns_param.env_type = AdapsEnvTypeIndoor;
             sns_param.power_mode = AdapsPowerModeDiv3;
             sns_param.framerate_type = AdapsFramerateType30FPS;
             break;

        default:
            DBG_ERROR("Invalid work mode %d.\n", sns_param.work_mode);
            return 0 - __LINE__;
            break;
    }

    // 注册信号处理函数
    signal(SIGINT, signal_handle);
    signal(SIGTERM, signal_handle);
    std::signal(SIGSEGV, signal_handle);
    std::signal(SIGABRT, signal_handle);
    std::signal(SIGBUS, signal_handle);
    std::signal(SIGUSR1, signal_handle);
    std::signal(SIGUSR2, signal_handle);

    DToF_Main dToF_main(sns_param);
    s_dToF_main = &dToF_main;

    return ret;
}
