#include "session.h"
#include "nvs_store.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "mbedtls/base64.h"
#include "mbedtls/md.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *TAG = "session";

static uint8_t s_jwt_secret[32];
static bool s_secret_ready = false;

/* Helper: base64url encode */
static bool base64url_encode(const unsigned char *src, size_t src_len, char *dst, size_t dst_max_len)
{
    size_t olen = 0;
    if (mbedtls_base64_encode((unsigned char *)dst, dst_max_len, &olen, src, src_len) != 0) {
        return false;
    }
    for (size_t i = 0; i < olen; i++) {
        if (dst[i] == '+') dst[i] = '-';
        else if (dst[i] == '/') dst[i] = '_';
        else if (dst[i] == '=') {
            dst[i] = '\0';
            break;
        }
    }
    return true;
}

/* Helper: base64url decode (used during validation) */
static bool base64url_decode(const char *src, unsigned char *dst, size_t dst_max_len, size_t *olen)
{
    size_t src_len = strlen(src);
    char *temp = malloc(src_len + 4);
    if (!temp) return false;
    strcpy(temp, src);
    for (size_t i = 0; i < src_len; i++) {
        if (temp[i] == '-') temp[i] = '+';
        else if (temp[i] == '_') temp[i] = '/';
    }
    size_t pad = (4 - (src_len % 4)) % 4;
    for (size_t i = 0; i < pad; i++) {
        temp[src_len + i] = '=';
    }
    temp[src_len + pad] = '\0';

    int ret = mbedtls_base64_decode(dst, dst_max_len, olen, (const unsigned char *)temp, strlen(temp));
    free(temp);
    return ret == 0;
}

/* Helper: sign payload with HMAC-SHA256 */
static void sign_jwt(const char *payload, size_t payload_len, unsigned char *output_hash)
{
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    mbedtls_md_hmac_starts(&ctx, s_jwt_secret, sizeof(s_jwt_secret));
    mbedtls_md_hmac_update(&ctx, (const unsigned char *)payload, payload_len);
    mbedtls_md_hmac_finish(&ctx, output_hash);
    mbedtls_md_free(&ctx);
}

bool session_init(void)
{
    if (nvs_store_get_jwt_secret(s_jwt_secret)) {
        s_secret_ready = true;
        ESP_LOGI(TAG, "Loaded JWT secret from NVS");
    } else {
        for (int i = 0; i < sizeof(s_jwt_secret); i++) {
            s_jwt_secret[i] = esp_random() & 0xFF;
        }
        if (nvs_store_set_jwt_secret(s_jwt_secret)) {
            s_secret_ready = true;
            ESP_LOGI(TAG, "Generated and saved new JWT secret to NVS");
        } else {
            ESP_LOGE(TAG, "Failed to save JWT secret to NVS");
            s_secret_ready = false;
            return false;
        }
    }
    return true;
}

bool session_create(const char *username, const char *client_ip, char token_out[SESSION_TOKEN_LEN])
{
    if (!s_secret_ready || !username || !client_ip || !token_out) return false;

    const char *header_json = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
    char payload_json[128];
    int64_t now = esp_timer_get_time() / 1000000;
    snprintf(payload_json, sizeof(payload_json), "{\"sub\":\"%s\",\"ip\":\"%s\",\"iat\":%lld}", username, client_ip, (long long)now);

    char header_b64[40];
    char payload_b64[128];

    if (!base64url_encode((const unsigned char *)header_json, strlen(header_json), header_b64, sizeof(header_b64)) ||
        !base64url_encode((const unsigned char *)payload_json, strlen(payload_json), payload_b64, sizeof(payload_b64))) {
        return false;
    }

    char signing_input[180];
    int len = snprintf(signing_input, sizeof(signing_input), "%s.%s", header_b64, payload_b64);
    if (len >= sizeof(signing_input)) return false;

    unsigned char signature[32];
    sign_jwt(signing_input, len, signature);

    char signature_b64[48];
    if (!base64url_encode(signature, sizeof(signature), signature_b64, sizeof(signature_b64))) {
        return false;
    }

    snprintf(token_out, SESSION_TOKEN_LEN, "%s.%s", signing_input, signature_b64);
    return true;
}

bool session_validate(const char *token, const char *client_ip)
{
    if (!s_secret_ready || !token || !client_ip) return false;

    char token_copy[SESSION_TOKEN_LEN];
    strncpy(token_copy, token, sizeof(token_copy) - 1);
    token_copy[sizeof(token_copy) - 1] = '\0';

    char *dot1 = strchr(token_copy, '.');
    if (!dot1) return false;
    char *dot2 = strchr(dot1 + 1, '.');
    if (!dot2) return false;

    *dot2 = '\0';
    const char *signing_input = token_copy;
    const char *signature_b64_in = dot2 + 1;

    unsigned char expected_sig[32];
    sign_jwt(signing_input, strlen(signing_input), expected_sig);

    char expected_sig_b64[64];
    if (!base64url_encode(expected_sig, sizeof(expected_sig), expected_sig_b64, sizeof(expected_sig_b64))) {
        return false;
    }

    if (strcmp(signature_b64_in, expected_sig_b64) != 0) {
        return false; // Signature mismatch
    }

    // Decode payload to verify IP and expiration
    const char *payload_b64 = dot1 + 1;
    unsigned char payload_json[128];
    size_t payload_len;
    if (!base64url_decode(payload_b64, payload_json, sizeof(payload_json) - 1, &payload_len)) {
        return false;
    }
    payload_json[payload_len] = '\0';

    // Verify IP
    char ip_key[16] = "\"ip\":\"";
    char *ip_start = strstr((char *)payload_json, ip_key);
    if (!ip_start) return false;
    ip_start += strlen(ip_key);
    char *ip_end = strchr(ip_start, '"');
    if (!ip_end) return false;

    size_t ip_len = ip_end - ip_start;
    if (strlen(client_ip) != ip_len || strncmp(client_ip, ip_start, ip_len) != 0) {
        return false; // IP mismatch
    }

    // Verify expiration
    if (SESSION_MAX_AGE_SEC > 0) {
        char iat_key[8] = "\"iat\":";
        char *iat_start = strstr((char *)payload_json, iat_key);
        if (iat_start) {
            iat_start += strlen(iat_key);
            int64_t iat = atoll(iat_start);
            int64_t now = esp_timer_get_time() / 1000000;
            if (now - iat > SESSION_MAX_AGE_SEC) {
                return false; // Expired
            }
        }
    }

    return true;
}

void session_destroy(const char *token)
{
    // JWT is stateless; destruction is handled client-side by deleting the cookie.
    (void)token;
}
