#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <signal.h>
#include <dirent.h>
#include <termios.h>
#include <sys/prctl.h>
#include <math.h>
#include <execinfo.h>

#include "mainwindow.h"
#include "globalapplication.h"

typedef void (*SIGNALHANDLE)(int signo);

static SIGNALHANDLE g_signalHandleCB = NULL;

static void dump_proc(int rfd, const char *filename)
{
    int wfd;
    ssize_t nr, nw, off;
    char *buf = NULL;
    struct stat sbuf;

    wfd = fileno(stdout);
    if (fstat(wfd, &sbuf) == -1) {
        LOG_ERROR("fstat %d fail.\n", wfd);
        return;
    }

    //stdout sbuf.st_blksize = 1024 byte
    buf = (char *)malloc(sbuf.st_blksize);
    if (buf == NULL) {
        LOG_ERROR("malloc %d buffer fail.\n", sbuf.st_blksize);
        return;
    }

    while ((nr = read(rfd, buf, sbuf.st_blksize)) != -1 && nr != 0) {
        for (off = 0; nr; nr -= nw, off += nw) {
            if ((nw = write(wfd, buf + off, nr)) == -1 || nw == 0) {
                free(buf);
                return;
            }
        }
    }

    if (nr == -1) {
        free(buf);
        LOG_ERROR("read %s size fail.\n", filename);
        return;
    }
}

static void show_proc()
{
    struct dirent* ep;
    DIR* pstrDirPoint = NULL;
    static char cMapInfo[270] = {0x00};
    char cPidBuf[64] = {0x00};

    pid_t pid = getpid();
    sprintf(cPidBuf, "%d", pid);

    pstrDirPoint = opendir("/proc");
    if (pstrDirPoint == NULL) {
        LOG_ERROR("/proc donot exist.\n");
        return;
    }

    while ((ep = readdir(pstrDirPoint))) {
        const char* name = ep->d_name;
        if (!strncmp(name, cPidBuf, strlen(cPidBuf))) {

            sprintf(cMapInfo, "/proc/%s/maps", name);
            FILE *f = fopen(cMapInfo, "r");
            if (f == NULL) {
                LOG_ERROR("fopen %s fail.\n", cMapInfo);
                return;
            }

            LOG_INFO("  -----show %s process maps info-----  \n", cMapInfo);
            dump_proc(fileno(f), cMapInfo);
            LOG_INFO(" ------------------------------------- \n");

            fclose(f);
            f = NULL;

            memset(cMapInfo, 0, 270);
            memcpy(cMapInfo, "/proc/meminfo", sizeof("/proc/meminfo"));
            f = fopen(cMapInfo, "r");
            if (f == NULL) {
                LOG_ERROR("fopen %s fail.\n", cMapInfo);
                return;
            }

            LOG_INFO("  -----show %s total memory info-----  \n", cMapInfo);
            dump_proc(fileno(f), cMapInfo);
            LOG_INFO(" ------------------------------------- \n");

            fclose(f);
            f = NULL;

            memset(cMapInfo, 0, 270);
            sprintf(cMapInfo, "/proc/%s/status", name);
            f = fopen(cMapInfo, "r");
            if (f == NULL) {
                LOG_ERROR("fopen %s fail.\n", cMapInfo);
                return;
            }

            LOG_INFO("  ----show %s process memory info----  \n", cMapInfo);
            dump_proc(fileno(f), cMapInfo);
            LOG_INFO(" ------------------------------------- \n");

            fclose(f);
            f = NULL;
        }
    }

    closedir(pstrDirPoint);
    return;
}

static void dump_stack(int signo)
{
    const int len = 1024;
    void *func[len];
    int size;
    int i;
    char **funs;

    signal(signo, SIG_DFL);
    size = backtrace(func, len);
    funs = (char**)backtrace_symbols(func, size);
    if (funs == NULL) {
        LOG_ERROR("signal %d, dump stack %d trace fail.\n", signo, size);
        return;
    }

    LOG_ERROR("=========> stack trace: %d <=========", size);
    for(i = 0; i < size; ++i)
        LOG_ERROR("%d %s", i, funs[i]);
    free(funs);
}

static void signal_handle(int signo)
{
    LOG_ERROR("=========>>>catch signal %d <<<=========\n", signo);

    //show_proc();

    dump_stack(signo);

    if (g_signalHandleCB)
        g_signalHandleCB(signo);

    exit(0);
}

void init_signal(SIGNALHANDLE signalHandleCB)
{
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGSEGV);
    sigaddset(&mask, SIGABRT);
    sigaddset(&mask, SIGBUS);

    g_signalHandleCB = signalHandleCB;
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    struct sigaction new_action, old_action;
    new_action.sa_handler = signal_handle;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGINT, &new_action, NULL);

    sigaction(SIGQUIT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGQUIT, &new_action, NULL);

    sigaction(SIGTERM, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGTERM, &new_action, NULL);

    sigaction(SIGSEGV, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGSEGV, &new_action, NULL);

    sigaction(SIGABRT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGABRT, &new_action, NULL);

    sigaction(SIGBUS, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGBUS, &new_action, NULL);

    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
}

int main(int argc, char *argv[])
{
    GlobalApplication a(argc, argv);

    MainWindow w;
    //init_signal(signal_handle);
    w.setWindowFlags(w.windowFlags() & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);
    w.show();

    return a.exec();
}
