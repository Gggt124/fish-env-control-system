#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool dns_server_start(void);
void dns_server_stop(void);

#ifdef __cplusplus
}
#endif
