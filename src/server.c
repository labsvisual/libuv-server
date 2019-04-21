#include <stdio.h>
#include <stdlib.h>

/* Include the primary headers from libuv and http-parser  */ 
#include "../libuv/include/uv.h"
#include "../http-parser/http_parser.h"

/* Static constants */
const static char* BIND_HOST = "0.0.0.0"; /* bind to all interfaces */
const static int BIND_PORT = 8080;
const static int BACKLOG = 128;

/* Type Definitions */
typedef struct {
    uv_tcp_t handle;
    http_parser parser;
} _http_client_t;

#define RESPONSE                  \
    "HTTP/1.1 200 OK\r\n"           \
    "Content-Type: text/plain\r\n"  \
    "Content-Length: 14\r\n"        \
    "\r\n"                          \
    "Hello, World!\n"

/* Variable definitions */
static uv_tcp_t server;
static http_parser_settings parser_settings;

/**
 * The close callback is called by uv_close to free up the memory which was used.
 */ 
void _on_close(uv_handle_t* handle) {
    _http_client_t *client = (_http_client_t*) handle->data;
    free(client);
}

void _on_shutdown(uv_shutdown_t *shutdown_req, int status) {
    uv_close((uv_handle_t *) shutdown_req->handle, _on_close);
    free(shutdown_req);
}

void _on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

void _on_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
    int err_no = 0;
    _http_client_t *client = (_http_client_t*) handle->data;

    if (nread >= 0) {
    
        size_t parsed = http_parser_execute(&client->parser, &parser_settings, buf->base, nread);

        if (parsed < nread) {
            printf("parse error\n");
            uv_close((uv_handle_t *) handle, _on_close);
        }

    } else {
    
        if (nread != UV_EOF) {
        
            printf("read: %s\n", uv_strerror(nread));
            
        }
    
        uv_shutdown_t *shutdown_req = malloc(sizeof(uv_shutdown_t));
        err_no = uv_shutdown(shutdown_req, handle, _on_shutdown);

    }

    free(buf->base);
}

void _on_write(uv_write_t* write_req, int status) {
    uv_close((uv_handle_t*) write_req->handle, _on_close);
    free(write_req);
}

int _on_headers_complete(http_parser* parser) {
    _http_client_t* client = (_http_client_t*) parser->data;

    uv_write_t *write_req = malloc(sizeof(uv_write_t));
    uv_buf_t buf = uv_buf_init(RESPONSE, sizeof(RESPONSE));
    int err_no = uv_write(write_req, (uv_stream_t*) &client->handle, &buf, 1, _on_write);

    if (err_no) {
        printf("on-headers-complete: %s\n", uv_strerror(err_no));
    }        

    return 1;
}

void _on_connection(uv_stream_t *server, int status) {
    _http_client_t *client = malloc(sizeof(_http_client_t));
    
    int err_no = uv_tcp_init(server->loop, &client->handle);
    if (err_no) {
        fprintf(stderr, "tcp_init: %s\n", uv_strerror(err_no));
        return;
    }
    client->handle.data = client;

    err_no = uv_accept(server, (uv_stream_t*) &client->handle);
    if (err_no) {
        fprintf(stderr, "tcp_init: %s\n", uv_strerror(err_no));

        uv_shutdown_t *shutdown_req = malloc(sizeof(uv_shutdown_t));
        uv_shutdown(shutdown_req, (uv_stream_t*) &client->handle, _on_shutdown);
    }

    http_parser_init(&client->parser, HTTP_REQUEST);
    client->parser.data = client;
    err_no = uv_read_start((uv_stream_t*) &client->handle, _on_alloc, _on_read);

}

void server_init(uv_loop_t *loop) {
    int err_no = uv_tcp_init(loop, &server);
    if (err_no) {
        fprintf(stderr, "tcp_init: %s\n", uv_strerror(err_no));
    }

    struct sockaddr_in addr;
    err_no = uv_ip4_addr(BIND_HOST, BIND_PORT, &addr);
    if (err_no) {
        fprintf(stderr, "ip4_addr: %s\n", uv_strerror(err_no));
    }

    err_no = uv_tcp_bind(&server, (struct sockaddr*) &addr, 0);
    if (err_no) {
        fprintf(stderr, "tcp_bind: %s\n", uv_strerror(err_no));
    }

    err_no = uv_listen((uv_stream_t*) &server, BACKLOG, _on_connection);
    if (err_no) {
        fprintf(stderr, "uv_listen: %s\n", uv_strerror(err_no));
    }
}


int main() {

    uv_loop_t* loop = uv_default_loop();

    parser_settings.on_headers_complete = _on_headers_complete;
    server_init(loop);

    int err_no = uv_run(loop, UV_RUN_DEFAULT);
    
    // MAKE_VALGRIND_HAPPY();
    return 0;

}

