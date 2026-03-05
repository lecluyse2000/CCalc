#ifndef SIGNAL_H
#define SIGNAL_H

namespace Signal {

[[nodiscard]] bool signal_received();
int check_signals_hook();
void register_handlers();

}

#endif
