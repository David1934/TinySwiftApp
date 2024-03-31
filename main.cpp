#include "mainwindow.h"
#include <globalapplication.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

static void handler(int signo) {
    const int len = 64;
    void *func[len];
    int size;
#if 1
    int i;
    char **funs;

    signal(signo, SIG_DFL);
    size = backtrace(func, len);
    funs = (char**)backtrace_symbols(func, size);
    if (funs == NULL) {
        DBG_ERROR("signal %d, dump stack %d trace fail.\n", signo, size);
        return;
    }
  
    DBG_ERROR("=========>>>catch signal %d <<<=========\n", signo);
    DBG_ERROR("=========> stack trace: %d <=========", size);
    for(i = 0; i < size; ++i)
        DBG_ERROR("%d %s", i, funs[i]);
    free(funs);
#else
    size = backtrace(func, len);

    DBG_ERROR("=========> stack trace: %d <=========", size);
    backtrace_symbols_fd(func, size, STDERR_FILENO);
#endif
  exit(1);
}

int main(int argc, char *argv[])
{
    GlobalApplication a(argc, argv);

    MainWindow w;
    signal(SIGSEGV, handler);
    w.show();

    return a.exec();
}
