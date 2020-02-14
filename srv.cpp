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
    const std::vector<std::string> new_name{req.name.begin(), iter};
    if (iter == req.name.end()) {
      std::cout << "Rejecting ";
      for (auto& i : req.name) std::cout << i << '.';
      std::cout << std::endl;
      return {};
    }
    const std::vector<std::string> path{iter, req.name.end()};

    dnspp::server::hook_ret ret;

    if (new_name.size() == 0) {
      ret.answer.emplace_back(dnspp::response {
                         .name = req.name,
                         .value = dnspp::response::txt{.records={
                           "Welcome to dnschat, the dns based chat system!",
                           ""
                           "The chat server intercepts all commands to a domain with dnschat in it, "
                           "but you should use the nameserver dns.c3murk.dev for best results, "
                           "as the global DNS networks weren't really built for this.",
                           "",
                           "All commands are domains, which should be request with one of the following commands:"
                           "",
                           "    nslookup -type=txt <command>.dnschat.<optional domain> <optional nameserver>",
                           "",
                           "    dig txt poll.dnschat.<optional domain> <optional nameserver>",
                           "",
                           "Post using the command '<msg>.<uname>.<nonce>' (dns can support spaces, but your client may not)",
                           "",
                           "Since chat responses are served with TXT records, unless you use a custom nameserver "
                           "you will probably lose the order of messages from the standard 'poll' command. If this is the case, ",
                           "use whichever of 'poll-num' and 'poll-concat' take your fancy.",
                           "",
                           "Have fun!"
                        }}
                      });
    }
    else if (new_name.size() == 1 && new_name.front() == "poll") {
      for (auto& i : msgs) {
        ret.answer.emplace_back(dnspp::response {
          .name = {i.first},
          .ttl = 1,
          .value=dnspp::response::txt{.records={i.second}}
        });
      }
    }
    else if (new_name.size() == 1 && new_name.front() == "poll-num") {
      size_t pos = 0;
      for (auto& i : msgs) {
        ret.answer.emplace_back(dnspp::response {
          .name = req.name,
          .ttl = 1,
          .value=dnspp::response::txt{.records={std::to_string(pos++), i.first, i.second}}
        });
      }
    }
    else if (new_name.size() == 1 && new_name.front() == "poll-concat") {
      dnspp::response::txt rec;
      for (auto& i : msgs)
        rec.records.push_back(i.first + ": " + i.second);
      ret.answer.emplace_back(dnspp::response {
                         .name = req.name,
                         .ttl = 1,
                         .value= std::move(rec)
      });
    }
    else {
      auto& msg = new_name.at(0);
      auto& uname = new_name.at(1);
      auto i = std::stoull(new_name.at(2));
      auto nonce_iter = nonces.find(uname);
      if (nonce_iter == nonces.end()) {
        nonces.emplace(uname, i);
        ret.answer.emplace_back(dnspp::response {
                           .name = req.name,
                           .value = dnspp::response::txt{.records={"Registered user! Increment nonce and fire away!"}}
                         });
      }
      else if (nonce_iter->second >= i || i - nonce_iter->second >= 16) {
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

    return ret;
  });
  ctx.run();
}
