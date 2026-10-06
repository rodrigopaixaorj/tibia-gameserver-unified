#ifndef TIBIA_LOGIN_SERVER_HH_
#define TIBIA_LOGIN_SERVER_HH_ 1

#include "../compat/compat.hh"

bool LoginServerStart(int Port, const char *BindIP = "0.0.0.0", const char *Motd = "Welcome to CipSoft 7.70 Server!");
void LoginServerStop(void);
bool LoginServerIsRunning(void);

#endif // TIBIA_LOGIN_SERVER_HH_
