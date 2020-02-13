#include "dnspp.hpp"

#include <iostream>

template<typename T>
std::vector<T> prepend(const T& a, std::vector<T> v) { v.insert(v.begin(), a); return v; }

int main(int argc, char** argv) {
  int port = argc > 1 ? std::atoi(argv[1]) : 53;
  boost::asio::io_context ctx;
  dnspp::server d(&ctx, port);

  std::vector<std::pair<std::string, std::string>> msgs = {{"server", "Started"}};
  std::map<std::string, uint64_t> nonces;

  d.recv.connect([&](const dnspp::request& req) -> dnspp::server::hook_ret {
    auto iter = std::find(req.name.begin(), req.name.end(), "dnschat");
    std::vector<std::string> new_name{req.name.begin(), iter};
    if (iter == req.name.end()) {
      std::cout << "Rejecting ";
      for (auto& i : req.name) std::cout << i << '.';
      std::cout << std::endl;
      return {};
    }
    std::vector<std::string> path{iter, req.name.end()};

    dnspp::server::hook_ret ret;

    if (new_name.size() == 0) {
      ret.answer.emplace_back(dnspp::response {
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
                           "Post using the domain <msg>.<uname>.<nonce>.dnschat.<dns>"
                           "",
                           "Have fun!"
                        }}
                      });
    }
    else if (new_name[0] == "poll") {
      std::cout << "Poll" << std::endl;
      for (auto& i : msgs) {
        continue;
        std::cout << '\t' << i.first << std::endl;
        ret.additional.emplace_back(dnspp::response {
                           .name = prepend(i.first, path),
                           .value=dnspp::response::txt{.records={i.second}}
                         });
      }
    }
    else {
      auto& msg = req.name.at(0);
      auto& uname = req.name.at(1);
      auto i = std::stoull(req.name.at(2));
      auto nonce_iter = nonces.find(uname);
      if (nonce_iter == nonces.end()) {
        nonces.emplace(uname, i);
        ret.answer.emplace_back(dnspp::response {
                           .name = req.name,
                           .value = dnspp::response::txt{.records={"Registered user! Increment nonce and fire away!"}}
                         });
      }
      else if (nonce_iter->second + 1 != i) {
        ret.answer.emplace_back(dnspp::response {
                           .name = req.name,
                           .value = dnspp::response::txt{.records={"Nonce needs to increase by 1!"}}
                         });
      }
      else {
        nonce_iter->second = i;
        msgs.emplace_back(uname, msg);
        ret.answer.emplace_back(dnspp::response {
                           .name = req.name,
                           .value = dnspp::response::txt{.records={"Success"}}
                         });
      }
    }
    std::cout << "Done" << std::endl;

    return ret;
  });
  ctx.run();
}
