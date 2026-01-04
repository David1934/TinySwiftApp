#include <csignal>
#include <unistd.h>
#include <execinfo.h>
#include <cstdlib>
#include <getopt.h>

#include "common.h"
#include "dtof_main.h"

#define DEPTH_LIB_SO_PATH                   "/vendor/lib64/libadaps_swift_decode.so"

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
    DBG_NOTICE("recieve signal: %d.\n", signo);

    if (SIGINT == signo)
    {
        if (NULL != s_dToF_main)
        {
            DBG_NOTICE("CTRL-C is detected, streaming will be stopped!\n");
            s_dToF_main->stop();
        }
    }
    else {
        dump_stack();
        exit(0);
    }

}

static void usage(char *name)
{
    printf("Usage: %s options\n"
        "         -w, --setworkmode,          required, sensor work mode (PCM=1, FHR=2).\n"
        "         -d, --dumpframe,            optional, how many frames to write files.\n"
        "         -k, --module_kernel_type,   optional, module kernel type for adaps algo lib.\n"
        "         -r, --roi_sram_rolling,             , enable roisram rolling (TRUE=1, FALSE=0).\n"
        "         -h, --help,                         , print this usage.\n\n",
        name);
}


static int parse_args(int argc, char **argv, struct sensor_params *sns_param)
{
    int c;
    int ret = 0;

    sns_param->to_dump_frame_cnt = 0;

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
                sns_param->work_mode = (enum sensor_workmode) atoi(optarg);
                break;

            case 'd':
                sns_param->to_dump_frame_cnt = atoi(optarg);
                break;

            case 'k':
                sns_param->module_kernel_type = atoi(optarg);
                break;

            case 'r':
                sns_param->roi_sram_rolling = atoi(optarg);
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

int requirement_check()
{
    int ret = 0;
    Utils *utils;
    utils = new Utils();

    ret = utils->check_regular_file_exist(DEPTH_LIB_SO_PATH);
    if (ret < 0)
    {
        DBG_ERROR("Adaps algo lib <%s> does not exist, please check it!\n", DEPTH_LIB_SO_PATH);
        goto error_exit;
    }

    ret = utils->check_regular_file_exist(DEPTH_LIB_CONFIG_PATH);
    if (ret < 0)
    {
        DBG_ERROR("Adaps algo lib config file <%s> does not exist, please check it!\n", DEPTH_LIB_CONFIG_PATH);
        goto error_exit;
    }

    if (true == Utils::is_env_var_true(ENV_VAR_ENABLE_ALGO_LIB_DUMP_DATA))
    {
        ret = utils->check_dir_exist_and_writable(DEPTH_LIB_DATA_DUMP_PATH);
        if (ret < 0)
        {
            DBG_ERROR("dump directory <%s> does not exist or not writable, please check it!\n", DEPTH_LIB_DATA_DUMP_PATH);
            goto error_exit;
        }
    }

error_exit:
    delete utils;

    return ret;
}

int main(int argc, char *argv[])
{
    int ret;
    struct sensor_params sns_param;

    printf("%s\nVersion: %s\nBuild Time: %s,%s\n\n",
        APP_NAME,
        APP_VERSION,
        __DATE__,
        __TIME__
        );

    memset(&sns_param, 0, sizeof(struct sensor_params));
    sns_param.module_kernel_type = 0xFF;

    //parse command arguments
    ret =parse_args(argc, argv, &sns_param);
    if (ret <0)
        return ret;

    if (sns_param.module_kernel_type >= CNT_MODULE)
    {
        DBG_ERROR("Invalid module_kernel_type %d.\n", sns_param.module_kernel_type);
        usage(argv[0]);
        return 0 - __LINE__;
    }

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
            usage(argv[0]);
            return 0 - __LINE__;
            break;
    }

    ret =requirement_check();
    if (ret < 0)
        return ret;

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
