#pragma once

#include <stdbool.h>
#include "app_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Simple in-memory session system.
 * Tokens are random 8-byte hex strings.
 *
 * WARNING: This is a local-prototype session system.
 * Sessions are stored in RAM only and do not persist across reboots.
 * Not suitable for internet-facing production use.
 */

#define SESSION_TOKEN_LEN 17  /* 16 hex chars + null */
#define SESSION_MAX_AGE_SEC APP_TEMPLATE_SESSION_MAX_AGE_SEC

bool session_init(void);
bool session_create(const char *username, char token_out[SESSION_TOKEN_LEN]);
bool session_validate(const char *token);
void session_destroy(const char *token);

#ifdef __cplusplus
}
#endif
