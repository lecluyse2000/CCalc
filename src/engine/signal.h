#ifndef SIGNAL_H
#define SIGNAL_H

namespace Signal {

int check_signals_hook();
void register_handlers();
[[nodiscard]] bool signal_received();

}

#endif
