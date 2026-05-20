#include "session.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SESSIONS 4

typedef struct {
    char token[SESSION_TOKEN_LEN];
    char username[32];
    int64_t created_at;
    bool active;
} session_entry_t;

static session_entry_t s_sessions[MAX_SESSIONS];
static SemaphoreHandle_t s_session_mutex;

bool session_init(void)
{
    memset(s_sessions, 0, sizeof(s_sessions));
    s_session_mutex = xSemaphoreCreateMutex();
    return s_session_mutex != NULL;
}

bool session_create(const char *username, char token_out[SESSION_TOKEN_LEN])
{
    uint8_t rand_bytes[8];
    for (int i = 0; i < 4; i++) {
        uint32_t r = esp_random();
        rand_bytes[i * 2]     = (r >> 0)  & 0xFF;
        rand_bytes[i * 2 + 1] = (r >> 8)  & 0xFF;
    }

    for (int i = 0; i < 8; i++) {
        snprintf(&token_out[i * 2], 3, "%02x", rand_bytes[i]);
    }
    token_out[16] = '\0';

    xSemaphoreTake(s_session_mutex, portMAX_DELAY);

    int slot = -1;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!s_sessions[i].active) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        slot = 0;
        int64_t oldest = s_sessions[0].created_at;
        for (int i = 1; i < MAX_SESSIONS; i++) {
            if (s_sessions[i].created_at < oldest) {
                oldest = s_sessions[i].created_at;
                slot = i;
            }
        }
    }

    strncpy(s_sessions[slot].token, token_out, SESSION_TOKEN_LEN - 1);
    strncpy(s_sessions[slot].username, username, sizeof(s_sessions[slot].username) - 1);
    s_sessions[slot].created_at = esp_timer_get_time() / 1000000;
    s_sessions[slot].active = true;

    xSemaphoreGive(s_session_mutex);
    return true;
}

bool session_validate(const char *token)
{
    if (!token || !token[0]) return false;

    xSemaphoreTake(s_session_mutex, portMAX_DELAY);

    int64_t now = esp_timer_get_time() / 1000000;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (s_sessions[i].active && strcmp(s_sessions[i].token, token) == 0) {
            if (SESSION_MAX_AGE_SEC > 0 && (now - s_sessions[i].created_at) > SESSION_MAX_AGE_SEC) {
                memset(&s_sessions[i], 0, sizeof(session_entry_t));
                xSemaphoreGive(s_session_mutex);
                return false;
            }
            xSemaphoreGive(s_session_mutex);
            return true;
        }
    }
    xSemaphoreGive(s_session_mutex);
    return false;
}

void session_destroy(const char *token)
{
    if (!token) return;

    xSemaphoreTake(s_session_mutex, portMAX_DELAY);

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (s_sessions[i].active && strcmp(s_sessions[i].token, token) == 0) {
            memset(&s_sessions[i], 0, sizeof(session_entry_t));
            xSemaphoreGive(s_session_mutex);
            return;
        }
    }
    xSemaphoreGive(s_session_mutex);
}
