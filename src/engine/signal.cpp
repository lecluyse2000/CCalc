#include "engine/signal.h"

#include <csignal>
#include <fstream>
#include <readline/readline.h>
#include <unistd.h>

#include "ui/ui.h"

namespace Signal {

namespace {

volatile sig_atomic_t g_got_sigint = 0;
volatile sig_atomic_t g_got_sigterm = 0;

void sigint_handler(int) {
    g_got_sigint = 1;
}
void sigterm_handler(int) {
    g_got_sigterm = 1;
}

}

int check_signals_hook() {
    if (g_got_sigint || g_got_sigterm) {
        rl_done = 1;
    }
    return 0;
}

void register_handlers() {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigint_handler;
    if(sigaction(SIGINT, &sa, nullptr) == -1) {
        UI::print_error("SIGINT handler error!");
    }

    sa.sa_handler = sigterm_handler;
    if(sigaction(SIGTERM, &sa, nullptr) == -1) {
        UI::print_error("SIGTERM handler error!");
    }
}

[[nodiscard]] bool signal_received() {
    return g_got_sigint || g_got_sigterm;
}

}
