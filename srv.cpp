#include "dnspp.hpp"

#include <iostream>

int main(int argc, char** argv) {
  int port = argc > 1 ? std::atoi(argv[1]) : 53;
  boost::asio::io_context ctx;
  dnspp::server d(&ctx, port);
  d.recv.connect([](const dnspp::request& req) -> std::vector<dnspp::response> {
    return {
      {
        .name = req.name,
        .value = dnspp::response::txt{.records={"hi"}}
      },
      {
        .name = req.name,
        .value = dnspp::response::a{.addr = boost::asio::ip::make_address_v4("127.0.0.1")}
      }
    };
  });
  ctx.run();
}
