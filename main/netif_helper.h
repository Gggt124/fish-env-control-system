#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "esp_netif.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t netif_add_alias_ip(esp_netif_t *sta_netif, const char *ip_str);

#ifdef __cplusplus
}
#endif
