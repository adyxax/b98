#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tcp.h"
#include "tls.h"

int sd = 0;
gnutls_session_t session;
gnutls_certificate_credentials_t xcred;

void tls_init(void)
{
    gnutls_global_init();
    gnutls_certificate_allocate_credentials(&xcred);
    //gnutls_certificate_set_x509_trust_file(xcred, cafile, GNUTLS_X509_FMT_PEM);
    //gnutls_certificate_set_verify_function(xcred, _verify_certificate_callback);
    //gnutls_certificate_set_x509_key_file (xcred, "cert.pem", "key.pem", GNUTLS_X509_FMT_PEM);
    gnutls_init(&session, GNUTLS_CLIENT);

    // Only usefull if accessing a virtual hosting server
    //gnutls_session_set_ptr(session, (void *) "my_host_name");
    //gnutls_server_name_set(session, GNUTLS_NAME_DNS, "my_host_name", strlen("my_host_name"));

    gnutls_set_default_priority(session);
    gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, xcred);
}

int tls_connect(const char* const host, const unsigned short port)
{
    int ret;
    sd = tcp_connect(host, port);
    if (sd < 0) {
        tls_clean();
        exit(101);
    }

    gnutls_transport_set_int(session, sd);
    gnutls_handshake_set_timeout(session, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);

    /* Perform the TLS handshake */
    do {
        ret = gnutls_handshake(session);
    }
    while (ret < 0 && gnutls_error_is_fatal(ret) == 0);

    if (ret < 0) {
        fprintf(stderr, "TLS Handshake failed\n");
        gnutls_perror(ret);
        tls_clean();
        exit(102);
    } else {
        char *desc;

        desc = gnutls_session_get_desc(session);
        printf("# TLS Session info: %s\n", desc);
        gnutls_free(desc);
    }

    int sf = fcntl (sd, F_GETFL, 0);
    if (sf == -1) {
        perror("fcntl get");
        return -1;
    }
    fcntl(sd, F_SETFL, sf | O_NONBLOCK);
    if (sf == -1) {
        perror("fcntl set");
        return -1;
    }

    return sd;
}

int tls_read(int (*input_handler_callback)(const char* const sbuf, const int len))
{
    char sbuf[512]; /* string buffer */
    int len = 0;  /* len read from the server */
    len = gnutls_record_recv(session, sbuf, sizeof(sbuf));
    if (len == 0) {
        printf("Server has closed the TLS connection\n");
        tls_clean();
        return 1;
    } else if (len < 0 && gnutls_error_is_fatal(len) == 0) {
        fprintf(stderr, "*** Warning: %s\n", gnutls_strerror(len));
        return 3;
    } else if (len < 0) {
        fprintf(stderr, "*** Error: %s\n", gnutls_strerror(len));
        tls_clean();
        return 2;
    } else {
        if (input_handler_callback(sbuf, len) != 0) {
            return 0;
        }
    }
    return 4;
}

void tls_send(char *buf, int len)
{
    gnutls_record_send(session, buf, len);
}

void tls_close(void) {
    gnutls_bye(session, GNUTLS_SHUT_RDWR);
    tls_clean();
}

void tls_clean(void)
{
    tcp_close(sd);
    gnutls_deinit(session);
    gnutls_certificate_free_credentials(xcred);
    gnutls_global_deinit();
}

/* This function will verify the peer's certificate, and check
 * if the hostname matches, as well as the activation, expiration dates. */
//static int _verify_certificate_callback(gnutls_session_t session)
//{
//    unsigned int status;
//    int ret, type;
//    const char *hostname;
//    gnutls_datum_t out;
//
//    /* read hostname */
//    hostname = gnutls_session_get_ptr(session);
//
//    /* This verification function uses the trusted CAs in the credentials
//     * structure. So you must have installed one or more CA certificates. */
//    gnutls_typed_vdata_st data[2];
//
//    memset(data, 0, sizeof(data));
//
//    data[0].type = GNUTLS_DT_DNS_HOSTNAME;
//    data[0].data = (void*)hostname;
//
//    data[1].type = GNUTLS_DT_KEY_PURPOSE_OID;
//    data[1].data = (void*)GNUTLS_KP_TLS_WWW_SERVER;
//
//    ret = gnutls_certificate_verify_peers(session, data, 2, &status);
//    if (ret < 0) {
//        printf("Error\n");
//        return GNUTLS_E_CERTIFICATE_ERROR;
//    }
//
//    type = gnutls_certificate_type_get(session);
//
//    ret = gnutls_certificate_verification_status_print(status, type, &out, 0);
//    if (ret < 0) {
//        printf("Error\n");
//        return GNUTLS_E_CERTIFICATE_ERROR;
//    }
//
//    printf("%s", out.data);
//
//    gnutls_free(out.data);
//
//    if (status != 0)        /* Certificate is not trusted */
//        return GNUTLS_E_CERTIFICATE_ERROR;
//
//    /* notify gnutls to continue handshake normally */
//    return 0;
//}
