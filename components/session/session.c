#include "session.h"
#include "esp_random.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SESSIONS 4

typedef struct {
    char token[SESSION_TOKEN_LEN];
    char username[32];
    bool active;
} session_entry_t;

static session_entry_t s_sessions[MAX_SESSIONS];

bool session_init(void)
{
    memset(s_sessions, 0, sizeof(s_sessions));
    return true;
}

bool session_create(const char *username, char token_out[SESSION_TOKEN_LEN])
{
    /* Generate 8 random bytes -> 16 hex chars */
    uint8_t rand_bytes[8];
    for (int i = 0; i < 4; i++) {
        uint32_t r = esp_random();
        rand_bytes[i * 2]     = (r >> 16) & 0xFF;
        rand_bytes[i * 2 + 1] = (r >> 24) & 0xFF;
    }

    /* Convert to lowercase hex */
    for (int i = 0; i < 8; i++) {
        snprintf(&token_out[i * 2], 3, "%02x", rand_bytes[i]);
    }
    token_out[16] = '\0';

    /* Store in first free slot or overwrite oldest */
    int slot = -1;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!s_sessions[i].active) {
            slot = i;
            break;
        }
    }
    if (slot < 0) slot = 0;

    strncpy(s_sessions[slot].token, token_out, SESSION_TOKEN_LEN - 1);
    strncpy(s_sessions[slot].username, username, sizeof(s_sessions[slot].username) - 1);
    s_sessions[slot].active = true;

    return true;
}

bool session_validate(const char *token)
{
    if (!token || !token[0]) return false;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (s_sessions[i].active && strcmp(s_sessions[i].token, token) == 0) {
            return true;
        }
    }
    return false;
}

void session_destroy(const char *token)
{
    if (!token) return;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (s_sessions[i].active && strcmp(s_sessions[i].token, token) == 0) {
            s_sessions[i].active = false;
            memset(s_sessions[i].token, 0, SESSION_TOKEN_LEN);
            return;
        }
    }
}
