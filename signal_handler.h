#ifndef SIGNAL_H
#define SIGNAL_H
void handle_sigint(int sig);
void reset_signal_handlers(void);
void setup_signal_handlers(void);

#endif


