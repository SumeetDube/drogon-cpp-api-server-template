#include "utils/SignalHandler.h"

#include <cstring>
#include <drogon/drogon.h>
#include <signal.h>
#include <unistd.h>

namespace
{
    volatile sig_atomic_t lastSignal = 0;

    void safeSignalHandler(int sig, siginfo_t*, void*)
    {
        lastSignal = sig;

        const char msg[] = "Signal received\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);

        if (sig == SIGSEGV || sig == SIGABRT ||
            sig == SIGBUS  || sig == SIGFPE  ||
            sig == SIGILL)
        {
            _exit(128 + sig);
        }
    }
}

namespace signalutils
{
    void registerSignals()
    {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));

        sa.sa_sigaction = safeSignalHandler;
        sa.sa_flags     = SA_SIGINFO | SA_RESTART;
        sigemptyset(&sa.sa_mask);

        int signals[] = {
            SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP,
            SIGABRT, SIGBUS, SIGFPE, SIGKILL, SIGUSR1,
            SIGSEGV, SIGUSR2, SIGPIPE, SIGALRM, SIGTERM,
            SIGSTKFLT, SIGCHLD, SIGCONT, SIGTSTP, SIGTTIN, SIGTTOU
        };

        for (int sig : signals)
        {
            sigaction(sig, &sa, nullptr);
        }
    }

    void attachDrogonSignalWatcher()
    {
        drogon::app().registerBeginningAdvice([]() {
            drogon::app().getLoop()->runEvery(
                0.5,
                []() {
                    if (lastSignal == 0)
                        return;

                    int sig = lastSignal;

                    LOG_WARN << "Process received signal "
                             << sig << " (" << strsignal(sig) << ")";

                    if (sig == SIGINT || sig == SIGTERM)
                    {
                        LOG_INFO << "Graceful shutdown initiated";
                        drogon::app().quit();
                    }

                    lastSignal = 0;
                });
        });
    }

    int getLastSignal()
    {
        return lastSignal;
    }
}
