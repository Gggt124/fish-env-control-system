#include "session.h"
#include "nvs_store.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "bootloader_random.h"
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
static bool sign_jwt(const char *payload, size_t payload_len, unsigned char *output_hash)
{
    if (!payload || !output_hash) return false;
    memset(output_hash, 0, 32);

    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!md_info) {
        ESP_LOGE(TAG, "sign_jwt: mbedtls_md_info_from_type(SHA256) returned NULL");
        return false;
    }

    int ret = mbedtls_md_hmac(md_info, s_jwt_secret, sizeof(s_jwt_secret), (const unsigned char *)payload, payload_len, output_hash);
    if (ret != 0) {
        ESP_LOGE(TAG, "sign_jwt: mbedtls_md_hmac failed: -0x%04x", -ret);
        return false;
    }

    return true;
}

bool session_init(void)
{
    if (nvs_store_get_jwt_secret(s_jwt_secret)) {
        s_secret_ready = true;
        ESP_LOGI(TAG, "Loaded JWT secret from NVS");
    } else {
        bootloader_random_enable();
        for (int i = 0; i < sizeof(s_jwt_secret); i++) {
            s_jwt_secret[i] = esp_random() & 0xFF;
        }
        bootloader_random_disable();
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

    char header_json[64];
    char payload_json[128];
    snprintf(header_json, sizeof(header_json), "{\"alg\":\"HS256\",\"typ\":\"JWT\"}");
    snprintf(payload_json, sizeof(payload_json), "{\"sub\":\"%s\",\"ip\":\"%s\"}", username, client_ip);

    char header_b64[40];
    char payload_b64[180];

    if (!base64url_encode((const unsigned char *)header_json, strlen(header_json), header_b64, sizeof(header_b64)) ||
        !base64url_encode((const unsigned char *)payload_json, strlen(payload_json), payload_b64, sizeof(payload_b64))) {
        return false;
    }

    char signing_input[200];
    int len = snprintf(signing_input, sizeof(signing_input), "%s.%s", header_b64, payload_b64);
    if (len >= sizeof(signing_input)) return false;

    unsigned char signature[32] = {0};
    if (!sign_jwt(signing_input, len, signature)) {
        return false;
    }

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
    const char *signing_input = token_copy; // "header.payload"
    const char *signature_b64_in = dot2 + 1;

    unsigned char expected_sig[32] = {0};
    if (!sign_jwt(signing_input, strlen(signing_input), expected_sig)) {
        ESP_LOGW(TAG, "validate: sign_jwt failed");
        return false;
    }

    char expected_sig_b64[64];
    if (!base64url_encode(expected_sig, sizeof(expected_sig), expected_sig_b64, sizeof(expected_sig_b64))) {
        ESP_LOGW(TAG, "validate: base64url encode of expected sig failed");
        return false;
    }

    size_t expected_len = strlen(expected_sig_b64);
    size_t in_len = strlen(signature_b64_in);
    
    if (in_len != expected_len) {
        ESP_LOGW(TAG, "validate: sig length mismatch expected=%d got=%d", (int)expected_len, (int)in_len);
        return false;
    }

    // Constant-time string comparison to mitigate HMAC timing attacks
    volatile int diff = 0;
    for (size_t i = 0; i < expected_len; i++) {
        diff |= (expected_sig_b64[i] ^ signature_b64_in[i]);
    }
    if (diff != 0) {
        ESP_LOGW(TAG, "validate: signature mismatch");
        ESP_LOGW(TAG, "  Expected: %s", expected_sig_b64);
        ESP_LOGW(TAG, "  Received: %s", signature_b64_in);
        ESP_LOGW(TAG, "  Signing Input: %s", signing_input);
        return false;
    }

    // Decode payload to verify IP and expiration
    const char *payload_b64 = dot1 + 1;
    unsigned char payload_json[256];
    size_t payload_len;
    if (!base64url_decode(payload_b64, payload_json, sizeof(payload_json) - 1, &payload_len)) {
        ESP_LOGW(TAG, "validate: payload decode failed");
        return false;
    }
    payload_json[payload_len] = '\0';

    // Verify IP
    char ip_key[16] = "\"ip\":\"";
    char *ip_start = strstr((char *)payload_json, ip_key);
    if (!ip_start) {
        ESP_LOGW(TAG, "validate: no ip field in payload");
        return false;
    }
    ip_start += strlen(ip_key);
    char *ip_end = strchr(ip_start, '"');
    if (!ip_end) {
        ESP_LOGW(TAG, "validate: unterminated ip field");
        return false;
    }

    size_t ip_len = ip_end - ip_start;
    
    // Handle IPv4-mapped IPv6 prefix if present in either client_ip or token_ip
    const char *c_ip = client_ip;
    if (strncmp(c_ip, "::FFFF:", 7) == 0 || strncmp(c_ip, "::ffff:", 7) == 0) c_ip += 7;
    
    const char *t_ip = ip_start;
    size_t t_ip_len = ip_len;
    if (t_ip_len > 7 && (strncmp(t_ip, "::FFFF:", 7) == 0 || strncmp(t_ip, "::ffff:", 7) == 0)) {
        t_ip += 7;
        t_ip_len -= 7;
    }

    if (strlen(c_ip) != t_ip_len || strncmp(c_ip, t_ip, t_ip_len) != 0) {
        ESP_LOGW(TAG, "validate: IP mismatch client='%s' (stripped '%s') token_ip='%.*s'",
                 client_ip, c_ip, (int)ip_len, ip_start);
        return false;
    }

    // Verify expiration removed to allow persistent sessions across reboots

    return true;
}

void session_destroy(const char *token)
{
    // JWT is stateless; destruction is handled client-side by deleting the cookie.
    (void)token;
}
