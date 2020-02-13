#include "dnspp.hpp"

#include <iostream>

int main(int argc, char** argv) {
  int port = argc > 1 ? std::atoi(argv[1]) : 53;
  boost::asio::io_context ctx;
  dnspp::server d(&ctx, port);

  std::vector<std::pair<std::string, std::string>> msgs;

  d.recv.connect([&](const dnspp::request& req) -> std::vector<dnspp::response> {
    if (req.name.size() < 2 || req.name.back() != "dnschat")
      return {};

    std::vector<dnspp::response> ret;

    if (req.name.size() == 2 && req.name.at(0) == "poll") {
      for (auto& i : msgs)
        ret.emplace_back(dnspp::response {
                           .name = {i.first, "dnschat"},
                           .value=dnspp::response::txt{.records={i.second}}
                         });
    }
    else {
      msgs.emplace_back(req.name.at(1), req.name.at(0));
      ret.emplace_back(dnspp::response {
                         .name = req.name,
                         .value = dnspp::response::txt{.records={"Success"}}
                       });
    }

    return ret;
  });
  ctx.run();
}
