#ifndef __HTTP_SERVER_HPP__
#define __HTTP_SERVER_HPP__

#include <iostream>
#include <map>

#include <libwebsockets.h>

int counter = 0;

struct ClientData {
  lws *wsi;
};

class ClientController {
  std::map<lws *, ClientData> clientData;

public:
  // TODO: cleanup this code
  auto new_client(lws *wsi) -> void {
    if (this->clientData.count(wsi) == 0) {
      std::cout << "New client connected\n";
      this->clientData[wsi] = ClientData{wsi};
    } else {
      throw "Trying to add an existing client to the map";
    }
  }

  // TODO: cleanup this code
  auto remove_client(lws *wsi) -> void {
    if (this->clientData.erase(wsi) == 0) {
      throw "No client found in the map";
    }
  }
};

class HttpController {
  ClientController &clientController;

public:
  HttpController(ClientController &clientController)
      : clientController{clientController} {}

  auto new_client(lws *wsi) -> void { this->clientController.new_client(wsi); }

  auto remove_client(lws *wsi) -> void {
    this->clientController.remove_client(wsi);
  }
};

struct ServerContext {
  int a;
};

static HttpController *get_http_controller(lws *wsi) {
  return static_cast<HttpController *>(lws_get_protocol(wsi)->user);
}

static int wsi_create(lws *wsi) {
  std::cout << " - LWS_CALLBACK_WSI_CREATE" << std::endl;
  auto connection_manager = get_http_controller(wsi);
  connection_manager->new_client(wsi);
  return 0;
}

static int get_thread_id(lws *wsi) {
  std::cout << " - LWS_CALLBACK_GET_THREAD_ID" << std::endl;
  return 0;
}

static int protocol_init(lws *wsi) {
  std::cout << " - LWS_CALLBACK_PROTOCOL_INIT" << std::endl;
  return 0;
}

static int event_wait_canceled(lws *wsi) {
  std::cout << " - LWS_CALLBACK_EVENT_WAIT_CANCELLED" << std::endl;
  return 0;
}

static int filter_network_connection(lws *wsi, void *user, void *in,
                                     size_t len) {
  std::cout << " - LWS_CALLBACK_FILTER_NETWORK_CONNECTION" << std::endl;
  return 0;
}

static int new_client_instantiated(lws *wsi) {
  std::cout << " - LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED" << std::endl;
  return 0;
}

static int filter_http_connection(lws *wsi, void *user, void *in, size_t len) {
  std::cout << " - LWS_CALLBACK_FILTER_HTTP_CONNECTION" << std::endl;
  return 0;
}

static int bind_protocol(lws *wsi, void *user) {
  std::cout << " - LWS_CALLBACK_HTTP_BIND_PROTOCOL" << std::endl;
  return 0;
}

static int callback_http(lws *wsi, void *user, void *in, size_t len) {
  std::cout << " - LWS_CALLBACK_HTTP" << std::endl;
  lws_callback_on_writable(wsi);
  return 0;
}

static int http_writable(lws *wsi, void *user) {
  std::cout << " - LWS_CALLBACK_HTTP_WRITEABLE" << std::endl;

  uint8_t buf[LWS_PRE + 4048], *start = &buf[LWS_PRE], *p = start,
                               *end = &buf[sizeof(buf) - 1];

  if (lws_add_http_header_status(wsi, http_status::HTTP_STATUS_OK, &p, end)) {
    return 1;
  }
  if (lws_add_http_header_content_length(wsi, 0, &p, end)) {
    return 1;
  }
  if (lws_add_http_header_by_token(wsi, WSI_TOKEN_CONNECTION,
                                   (unsigned char *)"keep-alive", 10, &p,
                                   end)) {
    return 1;
  }
  if (lws_finalize_write_http_header(wsi, start, &p, end)) {
    return 1;
  }
  return lws_http_transaction_completed(wsi);
}

static int http_drop_protocol(lws *wsi, void *user, void *in, size_t len) {
  std::cout << " - LWS_CALLBACK_HTTP_DROP_PROTOCOL" << std::endl;
  return 0;
}

static int closed_http(lws *wsi, void *user) {
  std::cout << "  - LWS_CALLBACK_CLOSED_HTTP" << std::endl;
  return 0;
}

static int wsi_destroy(lws *wsi, void *user) {
  std::cout << " - LWS_CALLBACK_WSI_DESTROY" << std::endl;
  auto httpController = get_http_controller(wsi);
  httpController->remove_client(wsi);
  return 0;
}

int http_protocol_ev(lws *wsi, lws_callback_reasons reason, void *user,
                     void *in, size_t len) {
  int a = 0;
  if (user && reason != LWS_CALLBACK_FILTER_NETWORK_CONNECTION) {
    ServerContext *cxt = (ServerContext *)user;
    if (cxt->a == 0) {
      cxt->a = ++counter;
    }
    a = cxt->a;
  }

  std::cout << "wsi:" << wsi << " user:" << user << "-" << a
            << " event :" << reason << " in:" << in;

  switch (reason) {
  case LWS_CALLBACK_GET_THREAD_ID:
    return get_thread_id(wsi);

  case LWS_CALLBACK_PROTOCOL_INIT:
    return protocol_init(wsi);

  case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
    return event_wait_canceled(wsi);

  case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
    return filter_network_connection(wsi, user, in, len);

  case LWS_CALLBACK_WSI_CREATE:
    return wsi_create(wsi);

  case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
    return new_client_instantiated(wsi);

  case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
    return filter_http_connection(wsi, user, in, len);

  case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
    return bind_protocol(wsi, user);

  case LWS_CALLBACK_HTTP:
    return callback_http(wsi, user, in, len);

  case LWS_CALLBACK_HTTP_WRITEABLE:
    return http_writable(wsi, user);

  case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
    return http_drop_protocol(wsi, user, in, len);

  case LWS_CALLBACK_CLOSED_HTTP:
    return closed_http(wsi, user);

  case LWS_CALLBACK_WSI_DESTROY:
    return wsi_destroy(wsi, user);

  default:
    std::cout << " - Not Handled\n";
    break;
  }
  return 0;
}

#endif