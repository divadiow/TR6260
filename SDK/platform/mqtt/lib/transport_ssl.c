#include <string.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"


#include "platform_tr6260.h"

#include "transport.h"
#include "transport_ssl.h"

/**
 *  mbedtls specific transport data
 */
typedef struct {
    mbedtls_entropy_context  entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context      ctx;
    mbedtls_x509_crt         cacert;
    mbedtls_x509_crt         client_cert;
    mbedtls_pk_context       client_key;
    mbedtls_ssl_config       conf;
    mbedtls_net_context      client_fd;
    void                     *cert_pem_data;
    int                      cert_pem_len;
    void                     *client_cert_pem_data;
    int                      client_cert_pem_len;
    void                     *client_key_pem_data;
    int                      client_key_pem_len;
    bool                     mutual_authentication;
    bool                     ssl_initialized;
    bool                     verify_server;
} transport_ssl_t;

static int ssl_close(transport_handle_t t);

static int ssl_connect(transport_handle_t t, const char *host, int port, int timeout_ms)
{
    int ret = -1, flags;
    struct timeval tv;
    transport_ssl_t *ssl = transport_get_context_data(t);

    if (!ssl) {
        return -1;
    }
    ssl->ssl_initialized = true;
    mbedtls_ssl_init(&ssl->ctx);
    mbedtls_ctr_drbg_init(&ssl->ctr_drbg);
    mbedtls_ssl_config_init(&ssl->conf);
    mbedtls_entropy_init(&ssl->entropy);

    if ((ret = mbedtls_ssl_config_defaults(&ssl->conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        system_printf("mbedtls_ssl_config_defaults returned %d", ret);
        goto exit;
    }

    if ((ret = mbedtls_ctr_drbg_seed(&ssl->ctr_drbg, mbedtls_entropy_func, &ssl->entropy, NULL, 0)) != 0) {
        system_printf("mbedtls_ctr_drbg_seed returned %d", ret);
        goto exit;
    }

    if (ssl->cert_pem_data) {
        mbedtls_x509_crt_init(&ssl->cacert);
        ssl->verify_server = true;
        if ((ret = mbedtls_x509_crt_parse(&ssl->cacert, ssl->cert_pem_data, ssl->cert_pem_len + 1)) < 0) {
            system_printf("mbedtls_x509_crt_parse returned -0x%x\n\nDATA=%s,len=%d", -ret, (char*)ssl->cert_pem_data, ssl->cert_pem_len);
            goto exit;
        }
        mbedtls_ssl_conf_ca_chain(&ssl->conf, &ssl->cacert, NULL);
        mbedtls_ssl_conf_authmode(&ssl->conf, MBEDTLS_SSL_VERIFY_REQUIRED);

        if ((ret = mbedtls_ssl_set_hostname(&ssl->ctx, host)) != 0) {
            system_printf("mbedtls_ssl_set_hostname returned -0x%x", -ret);
            goto exit;
        }
    } else {
        mbedtls_ssl_conf_authmode(&ssl->conf, MBEDTLS_SSL_VERIFY_NONE);
    }

    if (ssl->client_cert_pem_data && ssl->client_key_pem_data) {
        mbedtls_x509_crt_init(&ssl->client_cert);
        mbedtls_pk_init(&ssl->client_key);
        ssl->mutual_authentication = true;
        if ((ret = mbedtls_x509_crt_parse(&ssl->client_cert, ssl->client_cert_pem_data, ssl->client_cert_pem_len + 1)) < 0) {
            system_printf("mbedtls_x509_crt_parse returned -0x%x\n\nDATA=%s,len=%d", -ret, (char*)ssl->client_cert_pem_data, ssl->client_cert_pem_len);
            goto exit;
        }
        if ((ret = mbedtls_pk_parse_key(&ssl->client_key, ssl->client_key_pem_data, ssl->client_key_pem_len + 1, NULL, 0)) < 0) {
            system_printf("mbedtls_pk_parse_keyfile returned -0x%x\n\nDATA=%s,len=%d", -ret, (char*)ssl->client_key_pem_data, ssl->client_key_pem_len);
            goto exit;
        }

        if ((ret = mbedtls_ssl_conf_own_cert(&ssl->conf, &ssl->client_cert, &ssl->client_key)) < 0) {
            system_printf("mbedtls_ssl_conf_own_cert returned -0x%x\n", -ret);
            goto exit;
        }
    } else if (ssl->client_cert_pem_data || ssl->client_key_pem_data) {
        system_printf("You have to provide both client_cert_pem and client_key_pem for mutual authentication");
        goto exit;
    }

    mbedtls_ssl_conf_rng(&ssl->conf, mbedtls_ctr_drbg_random, &ssl->ctr_drbg);

    if ((ret = mbedtls_ssl_setup(&ssl->ctx, &ssl->conf)) != 0) {
        system_printf("mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        goto exit;
    }

    mbedtls_net_init(&ssl->client_fd);

    ms_to_timeval(timeout_ms, &tv);

    setsockopt(ssl->client_fd.fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    system_printf("Connect to %s:%d", host, port);
    char port_str[8] = {0};
    sprintf(port_str, "%d", port);
    if ((ret = mbedtls_net_connect(&ssl->client_fd, host, port_str, MBEDTLS_NET_PROTO_TCP)) != 0) {
        system_printf("mbedtls_net_connect returned -%x", -ret);
        goto exit;
    }

    mbedtls_ssl_set_bio(&ssl->ctx, &ssl->client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    if((ret = mbedtls_ssl_set_hostname(&ssl->ctx, host)) != 0) {
        system_printf(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
        goto exit;
    }

    system_printf("Performing the SSL/TLS handshake...");

    while ((ret = mbedtls_ssl_handshake(&ssl->ctx)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            system_printf("mbedtls_ssl_handshake returned -0x%x", -ret);
            goto exit;
        }
    }

    system_printf("Verifying peer X.509 certificate...");

    if ((flags = mbedtls_ssl_get_verify_result(&ssl->ctx)) != 0) {
        /* In real life, we probably want to close connection if ret != 0 */
        system_printf("Failed to verify peer certificate!");
        if (ssl->cert_pem_data) {
            goto exit;
        }
    } else {
        system_printf( "Certificate verified.");
    }

    system_printf("Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl->ctx));
    return ret;
exit:
    ssl_close(t);
    return ret;
}

static int ssl_poll_read(transport_handle_t t, int timeout_ms)
{
    transport_ssl_t *ssl = transport_get_context_data(t);
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(ssl->client_fd.fd, &readset);
    struct timeval timeout;
    ms_to_timeval(timeout_ms, &timeout);

    return select(ssl->client_fd.fd + 1, &readset, NULL, NULL, &timeout);
}

static int ssl_poll_write(transport_handle_t t, int timeout_ms)
{
    transport_ssl_t *ssl = transport_get_context_data(t);
    fd_set writeset;
    FD_ZERO(&writeset);
    FD_SET(ssl->client_fd.fd, &writeset);
    struct timeval timeout;
    ms_to_timeval(timeout_ms, &timeout);
    return select(ssl->client_fd.fd + 1, NULL, &writeset, NULL, &timeout);
}

static int ssl_write(transport_handle_t t, const char *buffer, int len, int timeout_ms)
{
    int poll, ret;
    transport_ssl_t *ssl = transport_get_context_data(t);

    if ((poll = transport_poll_write(t, timeout_ms)) <= 0) {
        system_printf("Poll timeout or error, errno=%s, fd=%d, timeout_ms=%d", strerror(errno), ssl->client_fd.fd, timeout_ms);
        return poll;
    }
    ret = mbedtls_ssl_write(&ssl->ctx, (const unsigned char *) buffer, len);
    if (ret <= 0) {
        system_printf("mbedtls_ssl_write error, errno=%s", strerror(errno));
    }
    return ret;
}

static int ssl_read(transport_handle_t t, char *buffer, int len, int timeout_ms)
{
    int poll = -1, ret;
    transport_ssl_t *ssl = transport_get_context_data(t);

    if (mbedtls_ssl_get_bytes_avail(&ssl->ctx) <= 0) {
        if ((poll = transport_poll_read(t, timeout_ms)) <= 0) {
            return poll;
        }
    }
    ret = mbedtls_ssl_read(&ssl->ctx, (unsigned char *)buffer, len);
    if (ret == 0) {
        return -1;
    }
    return ret;
}

static int ssl_close(transport_handle_t t)
{
    int ret = -1;
    transport_ssl_t *ssl = transport_get_context_data(t);
    if (ssl->ssl_initialized) {
        system_printf("Cleanup mbedtls");
        mbedtls_ssl_close_notify(&ssl->ctx);
        mbedtls_ssl_session_reset(&ssl->ctx);
        mbedtls_net_free(&ssl->client_fd);
        mbedtls_ssl_config_free(&ssl->conf);
        if (ssl->verify_server) {
            mbedtls_x509_crt_free(&ssl->cacert);
        }
        if (ssl->mutual_authentication) {
            mbedtls_x509_crt_free(&ssl->client_cert);
            mbedtls_pk_free(&ssl->client_key);
        }
        mbedtls_ctr_drbg_free(&ssl->ctr_drbg);
        mbedtls_entropy_free(&ssl->entropy);
        mbedtls_ssl_free(&ssl->ctx);
        ssl->mutual_authentication = false;
        ssl->ssl_initialized = false;
        ssl->verify_server = false;
    }
    return ret;
}

static int ssl_destroy(transport_handle_t t)
{
    transport_ssl_t *ssl = transport_get_context_data(t);
    transport_close(t);
    mqtt_free(ssl);
    return 0;
}

void transport_ssl_set_cert_data(transport_handle_t t, const char *data, int len)
{
    transport_ssl_t *ssl = transport_get_context_data(t);
    if (t && ssl) {
        ssl->cert_pem_data = (void *)data;
        ssl->cert_pem_len = len;
    }
}

void transport_ssl_set_client_cert_data(transport_handle_t t, const char *data, int len)
{
    transport_ssl_t *ssl = transport_get_context_data(t);
    if (t && ssl) {
        ssl->client_cert_pem_data = (void *)data;
        ssl->client_cert_pem_len = len;
    }
}

void transport_ssl_set_client_key_data(transport_handle_t t, const char *data, int len)
{
    transport_ssl_t *ssl = transport_get_context_data(t);
    if (t && ssl) {
        ssl->client_key_pem_data = (void *)data;
        ssl->client_key_pem_len = len;
    }
}

transport_handle_t transport_ssl_init()
{
    transport_handle_t t = transport_init();
    transport_ssl_t *ssl = mqtt_calloc(1, sizeof(transport_ssl_t));
    MEM_CHECK(ssl, return NULL);
    mbedtls_net_init(&ssl->client_fd);
    transport_set_context_data(t, ssl);
    transport_set_func(t, ssl_connect, ssl_read, ssl_write, ssl_close, ssl_poll_read, ssl_poll_write, ssl_destroy);
    return t;
}

