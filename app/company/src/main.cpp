#include <concepts>
#include <iostream>

#include <csignal>
#include <cstring>

#include "api/http_server.hpp"

static int interrupted;

static const char *param_names[] = {"text1", "send"};

void sigint_handler(int sig) { interrupted = 1; }

auto main(int argc, const char **argv) -> int {
  ClientController clientController;
  HttpController httpController(clientController);

  lws_protocols protocols[] = {
      {"http", http_protocol_ev, sizeof(int), 0, 0, &httpController, 0},
      {nullptr, nullptr, 0, 0, 0, nullptr, 0}};

  lws_context_creation_info info;
  lws_context *context;
  const char *p;
  int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

  signal(SIGINT, sigint_handler);

  if ((p = lws_cmdline_option(argc, argv, "-d")))
    logs = atoi(p);

  lws_set_log_level(logs, nullptr);
  lwsl_user("LWS minimal http server GET | visit http://localhost:7681\n");

  memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
  info.port = 7681;
  info.protocols = protocols;
  info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

  context = lws_create_context(&info);
  if (!context) {
    lwsl_err("lws init failed\n");
    return 1;
  }

  while (n >= 0 && !interrupted)
    n = lws_service(context, 0);

  lws_context_destroy(context);

  return 0;
}