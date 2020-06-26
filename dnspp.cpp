#include "dnspp.hpp"

#include <iostream>

#include <boost/asio/post.hpp>

#include <boost/algorithm/string/split.hpp>

namespace dnspp {
  using udp = boost::asio::ip::udp;

  enum dns_header_flags : uint16_t {
    QR_query          =  0 <<  7,
    QR_reply          =  1 <<  7,
    QR_mask           =  1 <<  7,

    OPCODE_query      =  0 <<  3,
    OPCODE_iquery     =  1 <<  3,
    OPCODE_status     =  2 <<  3,
    OPCODE_mask       =  3 <<  3,

    AA_mask           =  1 <<  2,

    TC_mask           =  1 <<  1,

    RD_mask           =  1 <<  0,

    RA_mask           =  1 <<  15,

    Z_mask            =  7 <<  12,

    mask              = 0xffff
  };

  void dns_pack_to(std::vector<uint8_t>& buf, uint32_t i) {
    buf.reserve(buf.size() + 4);
    buf.push_back(i >> 24); buf.push_back(i >> 16); buf.push_back(i >> 8); buf.push_back(i);
  }

  void dns_pack_to(std::vector<uint8_t>& buf, std::string_view str) {
    buf.push_back(str.size());
    buf.insert(buf.end(), str.begin(), str.end());
  }

  void dns_pack_to(std::vector<uint8_t>& buf, type_t i) {
    buf.reserve(buf.size() + 2);
    buf.push_back(i >> 8); buf.push_back(i);
  }

  void dns_pack_to(std::vector<uint8_t>& buf, class_t i) {
    buf.reserve(buf.size() + 2);
    buf.push_back(i >> 8); buf.push_back(i);
  }

  void dns_pack_to(std::vector<uint8_t>& buf, dns_header_flags i) {
    buf.reserve(buf.size() + 2);
    buf.push_back(i >> 8); buf.push_back(i);
  }

  void dns_pack_to(std::vector<uint8_t>& buf, rcode_t i) {
    buf.reserve(buf.size() + 2);
    buf.push_back(i >> 8); buf.push_back(i);
  }

  void dns_pack_to(std::vector<uint8_t>& buf, uint16_t i) {
    buf.reserve(buf.size() + 2);
    buf.push_back(i >> 8); buf.push_back(i);
  }


  void dns_pack_to(std::vector<uint8_t>& buf, const std::vector<std::string>& vec) {
    for (auto& i : vec)
      dns_pack_to(buf, i);
    buf.push_back(0); // To terminate the list
  }

  void dns_pack_to(std::vector<uint8_t>& buf, const request& req) {
    dns_pack_to(buf, req.name);
    buf.push_back(req.type >> 8); buf.push_back(req.type);
    buf.push_back(req.cls >> 8); buf.push_back(req.cls);
  }

  void dns_pack_to(std::vector<uint8_t>& buf, response::txt txt) {
    for (auto& i : txt.records)
      dns_pack_to(buf, i);
  }

  void dns_pack_to(std::vector<uint8_t>& buf, response::a a) {
    auto b = a.addr.to_bytes();
    buf.insert(buf.end(), b.begin(), b.end());
  }

  void dns_pack_to(std::vector<uint8_t>& buf, response::aaaa aaaa) {
    auto b = aaaa.addr.to_bytes();
    buf.insert(buf.end(), b.begin(), b.end());
  }

  void dns_pack_to(std::vector<uint8_t>& buf, response::soa soa) {
    dns_pack_to(buf, soa.mname);
    dns_pack_to(buf, soa.rname);
    dns_pack_to(buf, soa.serial);
    dns_pack_to(buf, soa.refresh);
    dns_pack_to(buf, soa.retry);
    dns_pack_to(buf, soa.expire);
    dns_pack_to(buf, soa.minimum);
  }

  void dns_pack_to(std::vector<uint8_t>& buf, response::ns ns) {
    dns_pack_to(buf, ns.nsdname);
  }

  void dns_pack_to(std::vector<uint8_t>& buf, const response& res) {
    dns_pack_to(buf, res.name);
    dns_pack_to(buf, res.get_type());
    dns_pack_to(buf, res.cls);
    dns_pack_to(buf, res.ttl);
    std::visit([&](auto i) {
      std::vector<uint8_t> tmp_buf;
      dns_pack_to(tmp_buf, i);
      dns_pack_to(buf, static_cast<uint16_t>(tmp_buf.size()));
      buf.insert(buf.end(), tmp_buf.begin(), tmp_buf.end());
    }, res.value);
  }

  void server::start_receive() {

    sock.async_receive_from(boost::asio::buffer(buf), remote, [&] (const boost::system::error_code& err, auto len) {
      try {
        if (err || len < 12)
          goto next;

        // No-one cares
//        dns_header_flags flags =
//            static_cast<dns_header_flags>(static_cast<uint16_t>(buf[2]) |
//                                          static_cast<uint16_t>(buf[3]) << 8);

        request req;

        uint16_t qdcount = static_cast<uint16_t>(buf[ 5]) | static_cast<uint16_t>(buf[ 4]) << 8;
//        uint16_t ancount = static_cast<uint16_t>(buf[ 7]) | static_cast<uint16_t>(buf[ 6]) << 8;
//        uint16_t nscount = static_cast<uint16_t>(buf[ 9]) | static_cast<uint16_t>(buf[ 8]) << 8;
//        uint16_t arcount = static_cast<uint16_t>(buf[11]) | static_cast<uint16_t>(buf[10]) << 8;

        // No-one does this
        if (qdcount != 1)
          goto next;

        auto iter = buf.begin() + 12;
        auto end = buf.begin() + len;

        if (iter == end)
          goto next;

        while (true) {
          if (iter == end)
            goto next;
          uint8_t section_len = *iter++;\
          if (section_len == 0)
            break;
          else if (end - iter < section_len)
            goto next;

          auto begin = iter;
          iter += section_len;
          req.name.emplace_back(reinterpret_cast<char*>(&*begin), section_len);
        }
        req.type = static_cast<type_t>(*iter++ >> 8);
        req.type = static_cast<type_t>(req.type | *iter++);
        req.cls = static_cast<class_t>(*iter++ >> 8);
        req.cls = static_cast<class_t>(req.cls | *iter++);

        boost::asio::post(responder_pool, [this, buf{std::move(buf)}, req{std::move(req)}]() {
          concat_responses responses;

          try {
            responses = recv(req);
            responses.rcode = RCODE_noerror;
          }
          catch (rcode_t rc_err) {
            responses.rcode = rc_err;
          }
          catch (std::exception& e) {
            for (auto& i : req.name)
              std::cerr << i << '.';
            std::cerr << ": " << e.what() << std::endl;
            responses.rcode = RCODE_servfail;
          }

          std::vector<uint8_t> response_buf;
          response_buf.reserve(12);
          response_buf.insert(response_buf.end(),buf.begin(), buf.begin() + 2);
          response_buf.push_back(QR_reply);
          response_buf.push_back(responses.rcode);
          response_buf.push_back(0);response_buf.push_back(1); // 1 query
          response_buf.push_back(responses.an.size()>>8); response_buf.push_back(responses.an.size());
          response_buf.push_back(responses.ns.size()>>8); response_buf.push_back(responses.ns.size());
          response_buf.push_back(responses.ad.size()>>8); response_buf.push_back(responses.ad.size());

          dns_pack_to(response_buf, req);

          for (auto& i : responses.an)
            dns_pack_to(response_buf, i);

          for (auto& i : responses.ns)
            dns_pack_to(response_buf, i);

          for (auto& i : responses.ad)
            dns_pack_to(response_buf, i);

          sock.async_send_to(boost::asio::buffer(response_buf), remote, [](auto...) {});
        });
      }
      catch(...) { goto next; }

      next:
      start_receive();
    });
  }

  server::server(boost::asio::io_context* io_ctx_, uint16_t port) :
    //udp::v6()
    io_ctx{io_ctx_},
    sock{*io_ctx, udp::endpoint{boost::asio::ip::udp::v4(), port}} {
    start_receive();
  }
}

