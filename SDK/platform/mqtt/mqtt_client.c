#include <stdio.h>

#include "mqtt_client.h"
#include "mqtt_msg.h"
#include "transport.h"
#include "transport_tcp.h"
#include "transport_ssl.h"
#include "transport_ws.h"
#include "platform_tr6260.h"
#include "mqtt_outbox.h"
#include "url_parser.h"

/* using uri parser */

typedef struct mqtt_state
{
    mqtt_connect_info_t *connect_info;
    uint8_t *in_buffer;
    uint8_t *out_buffer;
    int in_buffer_length;
    int out_buffer_length;
    uint32_t message_length;
    uint32_t message_length_read;
    mqtt_message_t *outbound_message;
    mqtt_connection_t mqtt_connection;
    uint16_t pending_msg_id;
    int pending_msg_type;
    int pending_publish_qos;
    int pending_msg_count;
} mqtt_state_t;

typedef struct {
    mqtt_event_callback_t event_handle;
    int task_stack;
    int task_prio;
    char *uri;
    char *host;
    char *path;
    char *scheme;
    int port;
    bool auto_reconnect;
    void *user_context;
    int network_timeout_ms;
} mqtt_config_storage_t;

typedef enum {
    MQTT_STATE_ERROR = -1,
    MQTT_STATE_UNKNOWN = 0,
    MQTT_STATE_INIT,
    MQTT_STATE_CONNECTED,
    MQTT_STATE_WAIT_TIMEOUT,
} mqtt_client_state_t;

struct trs_mqtt_client {
    transport_list_handle_t transport_list;
    transport_handle_t transport;
    mqtt_config_storage_t *config;
    mqtt_state_t  mqtt_state;
    mqtt_connect_info_t connect_info;
    mqtt_client_state_t state;
    long long keepalive_tick;
    long long reconnect_tick;
    int wait_timeout_ms;
    int auto_reconnect;
    trs_mqtt_event_t event;
    bool run;
    bool wait_for_ping_resp;
    outbox_handle_t outbox;
    EventGroupHandle_t status_bits;
};

const static int STOPPED_BIT = BIT0;

static int trs_mqtt_dispatch_event(trs_mqtt_client_handle_t client);
static int trs_mqtt_set_config(trs_mqtt_client_handle_t client, const trs_mqtt_client_config_t *config);
static int trs_mqtt_destroy_config(trs_mqtt_client_handle_t client);
static int trs_mqtt_connect(trs_mqtt_client_handle_t client, int timeout_ms);
static int trs_mqtt_abort_connection(trs_mqtt_client_handle_t client);
static int trs_mqtt_client_ping(trs_mqtt_client_handle_t client);
static char *create_string(const char *ptr, int len);

static int trs_mqtt_set_config(trs_mqtt_client_handle_t client, const trs_mqtt_client_config_t *config)
{
    //Copy user configurations to client context
    int err = 0;
    mqtt_config_storage_t *cfg = mqtt_calloc(1, sizeof(mqtt_config_storage_t));

    client->config = cfg;

    cfg->task_prio = config->task_prio;
    if (cfg->task_prio <= 0) {
        cfg->task_prio = MQTT_TASK_PRIORITY;
    }

    cfg->task_stack = config->task_stack;
    if (cfg->task_stack == 0) {
        cfg->task_stack = MQTT_TASK_STACK;
    }
    err = -1;
    if (config->host) {
        cfg->host = os_strdup(config->host);
        MEM_CHECK(cfg->host, goto _mqtt_set_config_failed);
    }
    cfg->port = config->port;

    if (config->username) {
        client->connect_info.username = os_strdup(config->username);
        MEM_CHECK(client->connect_info.username, goto _mqtt_set_config_failed);
    }

    if (config->password) {
        client->connect_info.password = os_strdup(config->password);
        MEM_CHECK(client->connect_info.password, goto _mqtt_set_config_failed);
    }

    if (config->client_id) {
        client->connect_info.client_id = os_strdup(config->client_id);
    } else {
        client->connect_info.client_id = platform_create_id_string();
    }
    MEM_CHECK(client->connect_info.client_id, goto _mqtt_set_config_failed);
    system_printf("MQTT client_id=%s\n", client->connect_info.client_id);

    if (config->uri) {
        cfg->uri = os_strdup(config->uri);
        MEM_CHECK(cfg->uri, goto _mqtt_set_config_failed);
    }

    if (config->lwt_topic) {
        client->connect_info.will_topic = os_strdup(config->lwt_topic);
        MEM_CHECK(client->connect_info.will_topic, goto _mqtt_set_config_failed);
    }

    if (config->lwt_msg_len) {
        client->connect_info.will_message = mqtt_malloc(config->lwt_msg_len);
        MEM_CHECK(client->connect_info.will_message, goto _mqtt_set_config_failed);
        memcpy(client->connect_info.will_message, config->lwt_msg, config->lwt_msg_len);
        client->connect_info.will_length = config->lwt_msg_len;
    } else if (config->lwt_msg) {
        client->connect_info.will_message = os_strdup(config->lwt_msg);
        MEM_CHECK(client->connect_info.will_message, goto _mqtt_set_config_failed);
        client->connect_info.will_length = strlen(config->lwt_msg);
    }

    client->connect_info.will_qos = config->lwt_qos;
    client->connect_info.will_retain = config->lwt_retain;

    client->connect_info.clean_session = 1;
    if (config->disable_clean_session) {
        client->connect_info.clean_session = false;
    }
    client->connect_info.keepalive = config->keepalive;
    if (client->connect_info.keepalive == 0) {
        client->connect_info.keepalive = MQTT_KEEPALIVE_TICK;
    }
    cfg->network_timeout_ms = MQTT_NETWORK_TIMEOUT_MS;
    cfg->user_context = config->user_context;
    cfg->event_handle = config->event_handle;
    cfg->auto_reconnect = true;
    if (config->disable_auto_reconnect) {
        cfg->auto_reconnect = false;
    }


    return err;
_mqtt_set_config_failed:
    trs_mqtt_destroy_config(client);
    return err;
}

static int trs_mqtt_destroy_config(trs_mqtt_client_handle_t client)
{
    mqtt_config_storage_t *cfg = client->config;
    mqtt_free(cfg->host);
    mqtt_free(cfg->uri);
    mqtt_free(cfg->path);
    mqtt_free(cfg->scheme);
    mqtt_free(client->connect_info.will_topic);
    mqtt_free(client->connect_info.will_message);
    mqtt_free(client->connect_info.client_id);
    mqtt_free(client->connect_info.username);
    mqtt_free(client->connect_info.password);
    mqtt_free(client->config);
    return 0;
}

static int trs_mqtt_connect(trs_mqtt_client_handle_t client, int timeout_ms)
{
    int write_len, read_len, connect_rsp_code;
    client->wait_for_ping_resp = false;
    mqtt_msg_init(&client->mqtt_state.mqtt_connection,
                  client->mqtt_state.out_buffer,
                  client->mqtt_state.out_buffer_length);
    client->mqtt_state.outbound_message = mqtt_msg_connect(&client->mqtt_state.mqtt_connection,
                                          client->mqtt_state.connect_info);
    client->mqtt_state.pending_msg_type = mqtt_get_type(client->mqtt_state.outbound_message->data);
    client->mqtt_state.pending_msg_id = mqtt_get_id(client->mqtt_state.outbound_message->data,
                                        client->mqtt_state.outbound_message->length);
    system_printf("Sending MQTT CONNECT message, type: %d, id: %04X\n",
             client->mqtt_state.pending_msg_type,
             client->mqtt_state.pending_msg_id);
    int i = 0;
    write_len = transport_write(client->transport,
                                (char *)client->mqtt_state.outbound_message->data,
                                client->mqtt_state.outbound_message->length,
                                client->config->network_timeout_ms);
    if (write_len < 0) {
        system_printf("Writing failed, errno= %d\n", errno);
        return -1;
    }
    read_len = transport_read(client->transport,
                              (char *)client->mqtt_state.in_buffer,
                              client->mqtt_state.in_buffer_length,
                              client->config->network_timeout_ms);
    if (read_len < 0) {
        system_printf("Error network response\n");
        return -1;
    }

    if (mqtt_get_type(client->mqtt_state.in_buffer) != MQTT_MSG_TYPE_CONNACK) {
        system_printf("Invalid MSG_TYPE response: %d, read_len: %d\n", mqtt_get_type(client->mqtt_state.in_buffer), read_len);
        return -1;
    }
    connect_rsp_code = mqtt_get_connect_return_code(client->mqtt_state.in_buffer);
    switch (connect_rsp_code) {
        case CONNECTION_ACCEPTED:
            system_printf("Connected\n");
            return 0;
        case CONNECTION_REFUSE_PROTOCOL:
            system_printf( "Connection refused, bad protocol\n");
            return -1;
        case CONNECTION_REFUSE_SERVER_UNAVAILABLE:
            system_printf("Connection refused, server unavailable\n");
            return -1;
        case CONNECTION_REFUSE_BAD_USERNAME:
            system_printf("Connection refused, bad username or password\n");
            return -1;
        case CONNECTION_REFUSE_NOT_AUTHORIZED:
            system_printf("Connection refused, not authorized\n");
            return -1;
        default:
            system_printf("Connection refused, Unknow reason\n");
            return -1;
    }
    return 0;
}

static int trs_mqtt_abort_connection(trs_mqtt_client_handle_t client)
{
    transport_close(client->transport);
    client->wait_timeout_ms = MQTT_RECONNECT_TIMEOUT_MS;
    client->reconnect_tick = platform_tick_get_ms();
    client->state = MQTT_STATE_WAIT_TIMEOUT;
    system_printf("Reconnect after %d ms\n", client->wait_timeout_ms);
    client->event.event_id = MQTT_EVENT_DISCONNECTED;
    client->wait_for_ping_resp = false;
    trs_mqtt_dispatch_event(client);
    return 0;
}

trs_mqtt_client_handle_t trs_mqtt_client_init(const trs_mqtt_client_config_t *config)
{
    trs_mqtt_client_handle_t client = mqtt_calloc(1, sizeof(struct trs_mqtt_client));
    MEM_CHECK(client, return NULL);
    trs_mqtt_set_config(client, config);
    client->transport_list = transport_list_init();
    MEM_CHECK(client->transport_list, goto _mqtt_init_failed);
    transport_handle_t tcp = transport_tcp_init();
    MEM_CHECK(tcp, goto _mqtt_init_failed);
    transport_set_default_port(tcp, MQTT_TCP_DEFAULT_PORT);
    transport_list_add(client->transport_list, tcp, "mqtt");
    if (config->transport == MQTT_TRANSPORT_OVER_TCP) {
        client->config->scheme = create_string("mqtt", 4);
        MEM_CHECK(client->config->scheme, goto _mqtt_init_failed);
    }
#if MQTT_ENABLE_WS
    transport_handle_t ws = transport_ws_init(tcp);
    MEM_CHECK(ws, goto _mqtt_init_failed);
    transport_set_default_port(ws, MQTT_WS_DEFAULT_PORT);
    transport_list_add(client->transport_list, ws, "ws");
    if (config->transport == MQTT_TRANSPORT_OVER_WS) {
        client->config->scheme = create_string("ws", 2);
        MEM_CHECK(client->config->scheme, goto _mqtt_init_failed);
    }
#endif

#if MQTT_ENABLE_SSL
    transport_handle_t ssl = transport_ssl_init();
    MEM_CHECK(ssl, goto _mqtt_init_failed);
    transport_set_default_port(ssl, MQTT_SSL_DEFAULT_PORT);
    if (config->cert_pem) {
        transport_ssl_set_cert_data(ssl, config->cert_pem, strlen(config->cert_pem));
    }
    if (config->client_cert_pem) {
        transport_ssl_set_client_cert_data(ssl, config->client_cert_pem, strlen(config->client_cert_pem));
    }
    if (config->client_key_pem) {
        transport_ssl_set_client_key_data(ssl, config->client_key_pem, strlen(config->client_key_pem));
    }
    transport_list_add(client->transport_list, ssl, "mqtts");
    if (config->transport == MQTT_TRANSPORT_OVER_SSL) {
        client->config->scheme = create_string("mqtts", 5);
        MEM_CHECK(client->config->scheme, goto _mqtt_init_failed);
    }
#endif

#if MQTT_ENABLE_WSS
    transport_handle_t wss = transport_ws_init(ssl);
    MEM_CHECK(wss, goto _mqtt_init_failed);
    transport_set_default_port(wss, MQTT_WSS_DEFAULT_PORT);
    transport_list_add(client->transport_list, wss, "wss");
    if (config->transport == MQTT_TRANSPORT_OVER_WSS) {
        client->config->scheme = create_string("wss", 3);
        MEM_CHECK(client->config->scheme, goto _mqtt_init_failed);
    }
#endif
    if (client->config->uri) {
        if (trs_mqtt_client_set_uri(client, client->config->uri) != 0) {
            goto _mqtt_init_failed;
        }
    }

    if (client->config->scheme == NULL) {
        client->config->scheme = create_string("mqtt", 4);
        MEM_CHECK(client->config->scheme, goto _mqtt_init_failed);
    }
    client->keepalive_tick = platform_tick_get_ms();
    client->reconnect_tick = platform_tick_get_ms();
    client->wait_for_ping_resp = false;
    int buffer_size = config->buffer_size;
    if (buffer_size <= 0) {
        buffer_size = MQTT_BUFFER_SIZE_BYTE;
    }
    client->mqtt_state.in_buffer = (uint8_t *)mqtt_malloc(buffer_size);
    MEM_CHECK(client->mqtt_state.in_buffer, goto _mqtt_init_failed);
    client->mqtt_state.in_buffer_length = buffer_size;
    client->mqtt_state.out_buffer = (uint8_t *)mqtt_malloc(buffer_size);
    MEM_CHECK(client->mqtt_state.out_buffer, goto _mqtt_init_failed);

    client->mqtt_state.out_buffer_length = buffer_size;
    client->mqtt_state.connect_info = &client->connect_info;
    client->outbox = outbox_init();
    MEM_CHECK(client->outbox, goto _mqtt_init_failed);
    client->status_bits = xEventGroupCreate();
    MEM_CHECK(client->status_bits, goto _mqtt_init_failed);
    return client;
_mqtt_init_failed:
    trs_mqtt_client_destroy(client);
    return NULL;
}

int trs_mqtt_client_destroy(trs_mqtt_client_handle_t client)
{
    trs_mqtt_client_stop(client);
    trs_mqtt_destroy_config(client);
    if(client->transport_list){ transport_list_destroy(client->transport_list); }
    if(client->outbox) { outbox_destroy(client->outbox); }
    if(client->status_bits) { vEventGroupDelete(client->status_bits); }
    if(client->mqtt_state.in_buffer) { mqtt_free(client->mqtt_state.in_buffer); }
    if(client->mqtt_state.out_buffer) { mqtt_free(client->mqtt_state.out_buffer); }
    if(client) { mqtt_free(client); }
    return 0;
}

static char *create_string(const char *ptr, int len)
{
    char *ret;
    if (len <= 0) {
        return NULL;
    }
    ret = mqtt_calloc(1, len + 1);
    MEM_CHECK(ret, return NULL);
    memcpy(ret, ptr, len);
    return ret;
}

int trs_mqtt_client_set_uri(trs_mqtt_client_handle_t client, const char *uri)
{
    struct http_parser_url puri;
    memset(&puri, 0, sizeof(struct http_parser_url));
    int parser_status = http_parser_parse_url(uri, strlen(uri), 0, &puri);
    if (parser_status != 0) {
        system_printf("Error parse uri = %s\n", uri);
        return -1;
    }

    if (client->config->scheme == NULL) {
        client->config->scheme = create_string(uri + puri.field_data[UF_SCHEMA].off, puri.field_data[UF_SCHEMA].len);
    }

    if (client->config->host == NULL) {
        client->config->host = create_string(uri + puri.field_data[UF_HOST].off, puri.field_data[UF_HOST].len);
    }

    if (client->config->path == NULL) {
        client->config->path = create_string(uri + puri.field_data[UF_PATH].off, puri.field_data[UF_PATH].len);
    }
    if (client->config->path) {
        transport_handle_t trans = transport_list_get_transport(client->transport_list, "ws");
        if (trans) {
            transport_ws_set_path(trans, client->config->path);
        }
        trans = transport_list_get_transport(client->transport_list, "wss");
        if (trans) {
            transport_ws_set_path(trans, client->config->path);
        }
    }

    if (puri.field_data[UF_PORT].len) {
        client->config->port = strtol((const char*)(uri + puri.field_data[UF_PORT].off), NULL, 10);
    }

    char *user_info = create_string(uri + puri.field_data[UF_USERINFO].off, puri.field_data[UF_USERINFO].len);
    if (user_info) {
        char *pass = strchr(user_info, ':');
        if (pass) {
            pass[0] = 0; //terminal username
            pass ++;
            client->connect_info.password = os_strdup(pass);
        }
        client->connect_info.username = os_strdup(user_info);

        mqtt_free(user_info);
    }

    return 0;
}

static int mqtt_write_data(trs_mqtt_client_handle_t client)
{
    int write_len = transport_write(client->transport,
                                    (char *)client->mqtt_state.outbound_message->data,
                                    client->mqtt_state.outbound_message->length,
                                    client->config->network_timeout_ms);
    // client->mqtt_state.pending_msg_type = mqtt_get_type(client->mqtt_state.outbound_message->data);
    if (write_len <= 0) {
        system_printf("Error write data or timeout, written len = %d\n", write_len);
        return -1;
    }
    /* we've just sent a mqtt control packet, update keepalive counter
     * [MQTT-3.1.2-23]
     */
    client->keepalive_tick = platform_tick_get_ms();
    return 0;
}

static int trs_mqtt_dispatch_event(trs_mqtt_client_handle_t client)
{
    client->event.msg_id = mqtt_get_id(client->mqtt_state.in_buffer, client->mqtt_state.in_buffer_length);
    client->event.user_context = client->config->user_context;
    client->event.client = client;

    if (client->config->event_handle) {
        return client->config->event_handle(&client->event);
    }
    return -1;
}



static void deliver_publish(trs_mqtt_client_handle_t client, uint8_t *message, int length)
{
    const char *mqtt_topic, *mqtt_data;
    uint32_t mqtt_topic_length, mqtt_data_length;
    uint32_t mqtt_len, mqtt_offset = 0, total_mqtt_len = 0;
    int len_read;
    transport_handle_t transport = client->transport;

    do
    {
        if (total_mqtt_len == 0) {
            mqtt_topic_length = length;
            mqtt_topic = mqtt_get_publish_topic(message, &mqtt_topic_length);
            mqtt_data_length = length;
            mqtt_data = mqtt_get_publish_data(message, &mqtt_data_length);
            total_mqtt_len = client->mqtt_state.message_length - client->mqtt_state.message_length_read + mqtt_data_length;
            mqtt_len = mqtt_data_length;
        } else {
            mqtt_len = len_read;
            mqtt_data = (const char*)client->mqtt_state.in_buffer;
        }

        system_printf("Get data len= %d, topic len=%d\n", mqtt_len, mqtt_topic_length);
        client->event.event_id = MQTT_EVENT_DATA;
        client->event.data = (char *)mqtt_data;
        client->event.data_len = mqtt_len;
        client->event.total_data_len = total_mqtt_len;
        client->event.current_data_offset = mqtt_offset;
        client->event.topic = (char *)mqtt_topic;
        client->event.topic_len = mqtt_topic_length;
        trs_mqtt_dispatch_event(client);

        mqtt_offset += mqtt_len;
        if (client->mqtt_state.message_length_read >= client->mqtt_state.message_length) {
            break;
        }

        len_read = transport_read(client->transport,
                                  (char *)client->mqtt_state.in_buffer,
                                  client->mqtt_state.message_length - client->mqtt_state.message_length_read > client->mqtt_state.in_buffer_length ?
                                  client->mqtt_state.in_buffer_length : client->mqtt_state.message_length - client->mqtt_state.message_length_read,
                                  client->config->network_timeout_ms);
        if (len_read <= 0) {
            system_printf("Read error or timeout: %d\n", errno);
            break;
        }
        client->mqtt_state.message_length_read += len_read;
    } while (1);


}

static bool is_valid_mqtt_msg(trs_mqtt_client_handle_t client, int msg_type, int msg_id)
{
    system_printf("pending_id=%d, pending_msg_count = %d\n", client->mqtt_state.pending_msg_id, client->mqtt_state.pending_msg_count);
    if (client->mqtt_state.pending_msg_count == 0) {
        return false;
    }
    if (outbox_delete(client->outbox, msg_id, msg_type) == 0) {
        client->mqtt_state.pending_msg_count --;
        return true;
    }
    if (client->mqtt_state.pending_msg_type == msg_type && client->mqtt_state.pending_msg_id == msg_id) {
        client->mqtt_state.pending_msg_count --;
        return true;
    }

    return false;
}

static void mqtt_enqueue(trs_mqtt_client_handle_t client)
{
    system_printf("mqtt_enqueue id: %d, type=%d successful\n",
             client->mqtt_state.pending_msg_id, client->mqtt_state.pending_msg_type);
    //lock mutex
    if (client->mqtt_state.pending_msg_count > 0) {
        //Copy to queue buffer
        outbox_enqueue(client->outbox,
                       client->mqtt_state.outbound_message->data,
                       client->mqtt_state.outbound_message->length,
                       client->mqtt_state.pending_msg_id,
                       client->mqtt_state.pending_msg_type,
                       platform_tick_get_ms());
    }
    //unlock
}

static int mqtt_process_receive(trs_mqtt_client_handle_t client)
{
    int read_len;
    uint8_t msg_type;
    uint8_t msg_qos;
    uint16_t msg_id;

    read_len = transport_read(client->transport, (char *)client->mqtt_state.in_buffer, client->mqtt_state.in_buffer_length, 1000);

    if (read_len < 0) {
        system_printf("Read error or end of stream\n");
        return -1;
    }

    if (read_len == 0) {
        return 0;
    }

    msg_type = mqtt_get_type(client->mqtt_state.in_buffer);
    msg_qos = mqtt_get_qos(client->mqtt_state.in_buffer);
    msg_id = mqtt_get_id(client->mqtt_state.in_buffer, client->mqtt_state.in_buffer_length);

    system_printf("msg_type=%d, msg_id=%d\n", msg_type, msg_id);
    switch (msg_type)
    {
        case MQTT_MSG_TYPE_SUBACK:
            if (is_valid_mqtt_msg(client, MQTT_MSG_TYPE_SUBSCRIBE, msg_id)) {
                system_printf("Subscribe successful\n");
                client->event.event_id = MQTT_EVENT_SUBSCRIBED;
                trs_mqtt_dispatch_event(client);
            }
            break;
        case MQTT_MSG_TYPE_UNSUBACK:
            if (is_valid_mqtt_msg(client, MQTT_MSG_TYPE_UNSUBSCRIBE, msg_id)) {
                system_printf("UnSubscribe successful\n");
                client->event.event_id = MQTT_EVENT_UNSUBSCRIBED;
                trs_mqtt_dispatch_event(client);
            }
            break;
        case MQTT_MSG_TYPE_PUBLISH:
            if (msg_qos == 1) {
                client->mqtt_state.outbound_message = mqtt_msg_puback(&client->mqtt_state.mqtt_connection, msg_id);
            }
            else if (msg_qos == 2) {
                client->mqtt_state.outbound_message = mqtt_msg_pubrec(&client->mqtt_state.mqtt_connection, msg_id);
            }

            if (msg_qos == 1 || msg_qos == 2) {
                system_printf("Queue response QoS: %d\n", msg_qos);

                if (mqtt_write_data(client) != 0) {
                    system_printf("Error write qos msg repsonse, qos = %d\n", msg_qos);
                    // TODO: Shoule reconnect?
                    // return -1;
                }
            }
            client->mqtt_state.message_length_read = read_len;
            client->mqtt_state.message_length = mqtt_get_total_length(client->mqtt_state.in_buffer, client->mqtt_state.message_length_read);
            system_printf("deliver_publish, message_length_read=%d, message_length=%d\n", read_len, client->mqtt_state.message_length);

            deliver_publish(client, client->mqtt_state.in_buffer, client->mqtt_state.message_length_read);
            break;
        case MQTT_MSG_TYPE_PUBACK:
            if (is_valid_mqtt_msg(client, MQTT_MSG_TYPE_PUBLISH, msg_id)) {
                system_printf("received MQTT_MSG_TYPE_PUBACK, finish QoS1 publish\n");
                client->event.event_id = MQTT_EVENT_PUBLISHED;
                trs_mqtt_dispatch_event(client);
            }

            break;
        case MQTT_MSG_TYPE_PUBREC:
            system_printf("received MQTT_MSG_TYPE_PUBREC\n");
            client->mqtt_state.outbound_message = mqtt_msg_pubrel(&client->mqtt_state.mqtt_connection, msg_id);
            mqtt_write_data(client);
            break;
        case MQTT_MSG_TYPE_PUBREL:
            system_printf("received MQTT_MSG_TYPE_PUBREL\n");
            client->mqtt_state.outbound_message = mqtt_msg_pubcomp(&client->mqtt_state.mqtt_connection, msg_id);
            mqtt_write_data(client);

            break;
        case MQTT_MSG_TYPE_PUBCOMP:
            system_printf("received MQTT_MSG_TYPE_PUBCOMP\n");
            if (is_valid_mqtt_msg(client, MQTT_MSG_TYPE_PUBLISH, msg_id)) {
                system_printf("Receive MQTT_MSG_TYPE_PUBCOMP, finish QoS2 publish\n");
                client->event.event_id = MQTT_EVENT_PUBLISHED;
                trs_mqtt_dispatch_event(client);
            }
            break;
        case MQTT_MSG_TYPE_PINGRESP:
            system_printf("MQTT_MSG_TYPE_PINGRESP\n");
            client->wait_for_ping_resp = false;
            break;
    }

    return 0;
}

static void trs_mqtt_task(void *pv)
{
    trs_mqtt_client_handle_t client = (trs_mqtt_client_handle_t) pv;
    client->run = true;
    //get transport by scheme
    client->transport = transport_list_get_transport(client->transport_list, client->config->scheme);

    if (client->transport == NULL) {
        system_printf("There are no transports valid, stop mqtt client, config scheme = %s\n", client->config->scheme);
        client->run = false;
    }
    //default port
    if (client->config->port == 0) {
        client->config->port = transport_get_default_port(client->transport);
    }
    client->state = MQTT_STATE_INIT;
    xEventGroupClearBits(client->status_bits, STOPPED_BIT);
    while (client->run) {
        switch ((int)client->state) {
            case MQTT_STATE_INIT:
                if (client->transport == NULL) {
                    system_printf("There are no transport\n");
                    client->run = false;
                }

                if (transport_connect(client->transport,
                                      client->config->host,
                                      client->config->port,
                                      client->config->network_timeout_ms) < 0) {
                    system_printf("Error transport connect\n");
                    trs_mqtt_abort_connection(client);
                    break;
                }
                system_printf("Transport connected to %s://%s:%d\n", client->config->scheme, client->config->host, client->config->port);
                if (trs_mqtt_connect(client, client->config->network_timeout_ms) != 0) {
                    system_printf("Error MQTT Connected\n");
                    trs_mqtt_abort_connection(client);
                    break;
                }
                client->event.event_id = MQTT_EVENT_CONNECTED;
                client->state = MQTT_STATE_CONNECTED;
                trs_mqtt_dispatch_event(client);

                break;
            case MQTT_STATE_CONNECTED:
                // receive and process data
                if (mqtt_process_receive(client) == -1) {
                    trs_mqtt_abort_connection(client);
                    break;
                }

                if (platform_tick_get_ms() - client->keepalive_tick > client->connect_info.keepalive * 1000 / 2) {
                    //No ping resp from last ping => Disconnected
                	if(client->wait_for_ping_resp){
                    	system_printf("No PING_RESP, disconnected\n");
                    	trs_mqtt_abort_connection(client);
                    	client->wait_for_ping_resp = false;
                    	break;
                    }
                	if (trs_mqtt_client_ping(client) == -1) {
                        system_printf("Can't send ping, disconnected\n");
                        trs_mqtt_abort_connection(client);
                        break;
                    } else {
                    	client->wait_for_ping_resp = true;
                    }
                	system_printf("PING sent\n");
                }

                //Delete mesaage after 30 senconds
                outbox_delete_expired(client->outbox, platform_tick_get_ms(), OUTBOX_EXPIRED_TIMEOUT_MS);
                //
                outbox_cleanup(client->outbox, OUTBOX_MAX_SIZE);
                break;
            case MQTT_STATE_WAIT_TIMEOUT:

                if (!client->config->auto_reconnect) {
                    client->run = false;
                    break;
                }
                if (platform_tick_get_ms() - client->reconnect_tick > client->wait_timeout_ms) {
                    client->state = MQTT_STATE_INIT;
                    client->reconnect_tick = platform_tick_get_ms();
                    system_printf("Reconnecting...\n");
                }
                system_printf("waiting for connnecting\n");
                vTaskDelay(client->wait_timeout_ms / 2 / portTICK_RATE_MS);
                break;
        }
    }
    transport_close(client->transport);
    xEventGroupSetBits(client->status_bits, STOPPED_BIT);
    system_printf("delete mqtt\n");
    vTaskDelete(NULL);
}

int trs_mqtt_client_start(trs_mqtt_client_handle_t client)
{
    if (client->state >= MQTT_STATE_INIT) {
        system_printf("Client has started\n");
        return -1;
    }
    if (xTaskCreate(trs_mqtt_task, "mqtt_task", client->config->task_stack, client, client->config->task_prio, NULL) != pdTRUE) {
        system_printf("Error create mqtt task\n");
        return -1;
    }
    return 0;
}

int trs_mqtt_client_stop(trs_mqtt_client_handle_t client)
{
    client->run = false;
    system_printf("current state is %d\n",client->state);
    if(client->state > MQTT_STATE_INIT) {
        xEventGroupWaitBits(client->status_bits, STOPPED_BIT, false, true, portMAX_DELAY);
    }
    client->state = MQTT_STATE_UNKNOWN;
    return 0;
}

static int trs_mqtt_client_ping(trs_mqtt_client_handle_t client)
{
    client->mqtt_state.outbound_message = mqtt_msg_pingreq(&client->mqtt_state.mqtt_connection);

    if (mqtt_write_data(client) != 0) {
        system_printf("Error sending ping\n");
        return -1;
    }
    system_printf("Sent PING successful\n");
    return 0;
}

int trs_mqtt_client_subscribe(trs_mqtt_client_handle_t client, const char *topic, int qos)
{
    if (client->state != MQTT_STATE_CONNECTED) {
        system_printf("Client has not connected\n");
        return -1;
    }
    mqtt_enqueue(client); //move pending msg to outbox (if have)
    client->mqtt_state.outbound_message = mqtt_msg_subscribe(&client->mqtt_state.mqtt_connection,
                                          topic, qos,
                                          &client->mqtt_state.pending_msg_id);

    client->mqtt_state.pending_msg_type = mqtt_get_type(client->mqtt_state.outbound_message->data);
    client->mqtt_state.pending_msg_count ++;

    if (mqtt_write_data(client) != 0) {
        system_printf("Error to subscribe topic=%s, qos=%d\n", topic, qos);
        return -1;
    }

    system_printf("Sent subscribe topic=%s, id: %d, type=%d successful\n", topic, client->mqtt_state.pending_msg_id, client->mqtt_state.pending_msg_type);
    return client->mqtt_state.pending_msg_id;
}

int trs_mqtt_client_unsubscribe(trs_mqtt_client_handle_t client, const char *topic)
{
    if (client->state != MQTT_STATE_CONNECTED) {
        system_printf("Client has not connected\n");
        return -1;
    }
    mqtt_enqueue(client);
    client->mqtt_state.outbound_message = mqtt_msg_unsubscribe(&client->mqtt_state.mqtt_connection,
                                          topic,
                                          &client->mqtt_state.pending_msg_id);
    system_printf("unsubscribe, topic\"%s\", id: %d\n", topic, client->mqtt_state.pending_msg_id);

    client->mqtt_state.pending_msg_type = mqtt_get_type(client->mqtt_state.outbound_message->data);
    client->mqtt_state.pending_msg_count ++;

    if (mqtt_write_data(client) != 0) {
        system_printf("Error to unsubscribe topic=%s\n", topic);
        return -1;
    }

    system_printf("Sent Unsubscribe topic=%s, id: %d, successful\n", topic, client->mqtt_state.pending_msg_id);
    return client->mqtt_state.pending_msg_id;
}

int trs_mqtt_client_publish(trs_mqtt_client_handle_t client, const char *topic, const char *data, int len, int qos, int retain)
{
    uint16_t pending_msg_id = 0;
    if (client->state != MQTT_STATE_CONNECTED) {
        system_printf("Client has not connected\n");
        return -1;
    }
    if (len <= 0) {
        len = strlen(data);
    }

    client->mqtt_state.outbound_message = mqtt_msg_publish(&client->mqtt_state.mqtt_connection,
                                          topic, data, len,
                                          qos, retain,
                                          &pending_msg_id);
    if (qos > 0) {
        client->mqtt_state.pending_msg_type = mqtt_get_type(client->mqtt_state.outbound_message->data);
        client->mqtt_state.pending_msg_id = pending_msg_id;
        client->mqtt_state.pending_msg_count ++;
        mqtt_enqueue(client);
    }

    if (mqtt_write_data(client) != 0) {
        system_printf("Error to public data to topic=%s, qos=%d\n", topic, qos);
        return -1;
    }
    return pending_msg_id;
}

int trs_mqtt_client_get_states(trs_mqtt_client_handle_t client)
{
    if(client){
        return (int)client->state;
    } else {
        return -1;
    }
}

char* trs_mqtt_client_get_url(trs_mqtt_client_handle_t client)
{
    if(client){
        return (char* )client->config->uri;
    } else {
        return NULL;
    }
}