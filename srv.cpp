#include "dnspp.hpp"

#include <iostream>

int main(int argc, char** argv) {
  int port = argc > 1 ? std::atoi(argv[1]) : 53;
  boost::asio::io_context ctx;
  dnspp::server d(&ctx, port);

  std::vector<std::pair<std::string, std::string>> msgs;

  d.recv.connect([&](const dnspp::request& req) -> std::vector<dnspp::response> {
    if (req.name.size() < 2 || req.name[req.name.size() - 2] != "dnschat")
      return {};

    std::vector<dnspp::response> ret(1);
    auto& resp = ret.front();
    resp.name = req.name;

    if (req.name.at(0) == "poll") {
      std::vector<dnspp::response> ret;
      resp.value = dnspp::response::txt{};
      auto& val = std::get<dnspp::response::txt>(resp.value);
      for (auto& i : msgs)
        val.records.emplace_back(i.first + ": " + i.second);
    }
    else {
      msgs.emplace_back(req.name.at(1), req.name.at(2));
      resp.value = dnspp::response::txt{.records={"Success"}};
    }

    return ret;
  });
  ctx.run();
}
