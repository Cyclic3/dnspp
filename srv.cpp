#include "dnspp.hpp"

#include <iostream>

int main(int argc, char** argv) {
  int port = argc > 1 ? std::atoi(argv[1]) : 53;
  boost::asio::io_context ctx;
  dnspp::server d(&ctx, port);

  std::vector<std::pair<std::string, std::string>> msgs;

  d.recv.connect([&](const dnspp::request& req) -> std::vector<dnspp::response> {
    auto iter = std::find(req.name.begin(), req.name.end(), "dnschat");
    std::vector<std::string> new_name{req.name.begin(), iter};
    if (new_name.size() == 0)
      return {};
    std::vector<std::string> path{iter, req.name.end()};

    std::vector<dnspp::response> ret;

    if (req.name.size() == 1) {
      ret.emplace_back(dnspp::response {
                         .name = req.name,
                         .value = dnspp::response::txt{.records={
                                                         "Welcome to dnschat!",
                                                         "",
                                                         "To get started, use one of the following commands:"
                                                         ""
                                                         "nslookup -type=txt poll.dnschat dns.c3murk.dev"
                                                         ""
                                                         "dig txt poll.dnschat.<dns>"
                                                         "",
                                                         "Post using the domain <msg>.<uname>.dnschat.<dns>"
                                                         "",
                                                         "Have fun!"
                                                       }}
                       });
    }
    else if (new_name[0] == "poll") {
      for (auto& i : msgs)
        ret.emplace_back(dnspp::response {
                           .name = {i.first},
                           .value=dnspp::response::txt{.records={i.second}}
                         });
    }
    else {
      msgs.emplace_back(req.name.at(1), new_name[0]);
      ret.emplace_back(dnspp::response {
                         .name = req.name,
                         .value = dnspp::response::txt{.records={"Success"}}
                       });
    }

    return ret;
  });
  ctx.run();
}
