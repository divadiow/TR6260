/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE', which is part of this source code package.
 * Tuan PM <tuanpm at live dot com>
 */
#ifndef _TRANSPORT_H_
#define _TRANSPORT_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef struct transport_list_t* transport_list_handle_t;
typedef struct transport_item_t* transport_handle_t;

typedef int (*connect_func)(transport_handle_t t, const char *host, int port, int timeout_ms);
typedef int (*io_func)(transport_handle_t t, const char *buffer, int len, int timeout_ms);
typedef int (*io_read_func)(transport_handle_t t, char *buffer, int len, int timeout_ms);
typedef int (*trans_func)(transport_handle_t t);
typedef int (*poll_func)(transport_handle_t t, int timeout_ms);

/**
 * @brief      Create transport list
 *
 * @return     A handle can hold all transports
 */
transport_list_handle_t transport_list_init();

/**
 * @brief      Cleanup and free all transports, include itself,
 *             this function will invoke transport_destroy of every transport have added this the list
 *
 * @param[in]  list  The list
 *
 * @return
 *     - 0
 *     - -1
 */
int transport_list_destroy(transport_list_handle_t list);

/**
 * @brief      Add a transport to the list, and define a scheme to indentify this transport in the list
 *
 * @param[in]  list    The list
 * @param[in]  t       The Transport
 * @param[in]  scheme  The scheme
 *
 * @return
 *     - 0
 *     - -1
 */
int transport_list_add(transport_list_handle_t list, transport_handle_t t, const char *scheme);

/**
 * @brief      This function will remove all transport from the list,
 *             invoke transport_destroy of every transport have added this the list
 *
 * @param[in]  list  The list
 *
 * @return
 *     - 0
 *     - -1
 */
int transport_list_clean(transport_list_handle_t list);

/**
 * @brief      Get the transport by scheme, which has been defined when calling function `transport_list_add`
 *
 * @param[in]  list  The list
 * @param[in]  tag   The tag
 *
 * @return     The transport handle
 */
transport_handle_t transport_list_get_transport(transport_list_handle_t list, const char *scheme);

/**
 * @brief      Initialize a transport handle object
 *
 * @return     The transport handle
 */
transport_handle_t transport_init();

/**
 * @brief      Cleanup and free memory the transport
 *
 * @param[in]  t     The transport handle
 *
 * @return
 *     - 0
 *     - -1
 */
int transport_destroy(transport_handle_t t);

/**
 * @brief      Get default port number used by this transport
 *
 * @param[in]  t     The transport handle
 *
 * @return     the port number
 */
int transport_get_default_port(transport_handle_t t);

/**
 * @brief      Set default port number that can be used by this transport
 *
 * @param[in]  t     The transport handle
 * @param[in]  port  The port number
 *
 * @return
 *     - 0
 *     - -1
 */
int transport_set_default_port(transport_handle_t t, int port);

/**
 * @brief      Transport connection function, to make a connection to server
 *
 * @param      t           The transport handle
 * @param[in]  host        Hostname
 * @param[in]  port        Port
 * @param[in]  timeout_ms  The timeout milliseconds
 *
 * @return
 * - socket for will use by this transport
 * - (-1) if there are any errors, should check errno
 */
int transport_connect(transport_handle_t t, const char *host, int port, int timeout_ms);

/**
 * @brief      Transport read function
 *
 * @param      t           The transport handle
 * @param      buffer      The buffer
 * @param[in]  len         The length
 * @param[in]  timeout_ms  The timeout milliseconds
 *
 * @return
 *  - Number of bytes was read
 *  - (-1) if there are any errors, should check errno
 */
int transport_read(transport_handle_t t, char *buffer, int len, int timeout_ms);

/**
 * @brief      Poll the transport until readable or timeout
 *
 * @param[in]  t           The transport handle
 * @param[in]  timeout_ms  The timeout milliseconds
 *
 * @return
 *     - 0      Timeout
 *     - (-1)   If there are any errors, should check errno
 *     - other  The transport can read
 */
int transport_poll_read(transport_handle_t t, int timeout_ms);

/**
 * @brief      Transport write function
 *
 * @param      t           The transport handle
 * @param      buffer      The buffer
 * @param[in]  len         The length
 * @param[in]  timeout_ms  The timeout milliseconds
 *
 * @return
 *  - Number of bytes was written
 *  - (-1) if there are any errors, should check errno
 */
int transport_write(transport_handle_t t, const char *buffer, int len, int timeout_ms);

/**
 * @brief      Poll the transport until writeable or timeout
 *
 * @param[in]  t           The transport handle
 * @param[in]  timeout_ms  The timeout milliseconds
 *
 * @return
 *     - 0      Timeout
 *     - (-1)   If there are any errors, should check errno
 *     - other  The transport can write
 */
int transport_poll_write(transport_handle_t t, int timeout_ms);

/**
 * @brief      Transport close
 *
 * @param      t     The transport handle
 *
 * @return
 * - 0 if ok
 * - (-1) if there are any errors, should check errno
 */
int transport_close(transport_handle_t t);

/**
 * @brief      Get user data context of this transport
 *
 * @param[in]  t        The transport handle
 *
 * @return     The user data context
 */
void *transport_get_context_data(transport_handle_t t);

/**
 * @brief      Set the user context data for this transport
 *
 * @param[in]  t        The transport handle
 * @param      data     The user data context
 *
 * @return
 *     - 0
 */
int transport_set_context_data(transport_handle_t t, void *data);

/**
 * @brief      Set transport functions for the transport handle
 *
 * @param[in]  t            The transport handle
 * @param[in]  _connect     The connect function pointer
 * @param[in]  _read        The read function pointer
 * @param[in]  _write       The write function pointer
 * @param[in]  _close       The close function pointer
 * @param[in]  _poll_read   The poll read function pointer
 * @param[in]  _poll_write  The poll write function pointer
 * @param[in]  _destroy     The destroy function pointer
 *
 * @return
 *     - 0
 */
int transport_set_func(transport_handle_t t,
                             connect_func _connect,
                             io_read_func _read,
                             io_func _write,
                             trans_func _close,
                             poll_func _poll_read,
                             poll_func _poll_write,
                             trans_func _destroy);
#ifdef __cplusplus
}
#endif
#endif
