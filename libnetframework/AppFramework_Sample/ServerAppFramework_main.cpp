#include <stdio.h>
#include <string.h>

#include "ServerAppFramework.h"

int main()
{
	SLOG_INIT("./config/slog.config");

	ServerAppFramework app_server;
	app_server.start_server();

	SLOG_UNINIT();
	return 0;
}



