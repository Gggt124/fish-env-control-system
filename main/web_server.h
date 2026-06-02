#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool web_server_start(void);
void web_server_log_diagnostics(void);

#ifdef __cplusplus
}
#endif
