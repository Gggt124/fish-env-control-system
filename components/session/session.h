#pragma once

#include <stdbool.h>
#include "app_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Stateless JWT session system using HMAC-SHA256.
 * Tokens are signed with an NVS-stored secret.
 */

#define SESSION_TOKEN_LEN 256
#define SESSION_MAX_AGE_SEC APP_TEMPLATE_SESSION_MAX_AGE_SEC

bool session_init(void);
bool session_create(const char *username, const char *client_ip, char token_out[SESSION_TOKEN_LEN]);
bool session_validate(const char *token, const char *client_ip);
void session_destroy(const char *token);

#ifdef __cplusplus
}
#endif
