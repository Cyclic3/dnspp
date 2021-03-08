#include <boost/asio/ip/udp.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/signals2/signal.hpp>

#include <optional>
#include <thread>
#include <variant>

namespace dnspp {
  enum type_t : uint16_t {
    TYPE_A = 1,
    TYPE_NS = 2,
    TYPE_CNAME = 5,
    TYPE_SOA = 6,
    TYPE_PTR = 12,
    TYPE_MX = 15,
    TYPE_TXT = 16,
    TYPE_RP = 17,
    TYPE_SIG = 24,
    TYPE_KEY = 25,
    TYPE_AAAA = 28,
    TYPE_LOC = 29,
    TYPE_SRV = 33,
    TYPE_SSHFP = 44,
    TYPE_RRSIG = 36,
    TYPE_CAA = 257,
  };

  enum rcode_t : uint16_t {
    RCODE_noerror     =  0,
    RCODE_formerr     =  1,
    RCODE_servfail    =  2,
    RCODE_nxdomain    =  3,
    RCODE_notimp      =  4,
    RCODE_refused     =  5,
    RCODE_mask        = 15,
  };

  enum class_t : uint16_t {
    CLASS_IN = 0x1
  };

  struct request {
    boost::asio::ip::udp::endpoint src;
    std::vector<std::string> name;
    class_t cls = CLASS_IN;
    type_t type;
  };

  struct response {
    struct txt {
      std::vector<std::string> records;
    };
    struct a {
      boost::asio::ip::address_v4 addr;
    };
    struct aaaa {
      boost::asio::ip::address_v6 addr;
    };
    struct ns {
      std::vector<std::string> nsdname;
    };
    struct soa {
      std::vector<std::string> mname;
      std::vector<std::string> rname;
      uint32_t serial;
      uint32_t refresh;
      uint32_t retry;
      uint32_t expire;
      uint32_t minimum;
    };

    std::vector<std::string> name;
    class_t cls = CLASS_IN;
    uint32_t ttl = 0;
    std::variant<txt, a, aaaa, soa, ns> value;

    type_t get_type() const noexcept {
      constexpr type_t arr[] = {TYPE_TXT, TYPE_A, TYPE_AAAA, TYPE_SOA, TYPE_NS};
      return arr[value.index()];
    }
  };

  class server {
  private:
    struct concat_responses {
      std::vector<response> an;
      std::vector<response> ns;
      std::vector<response> ad;
      rcode_t rcode;
    };

    struct dns_concat {
      using result_type = concat_responses;

      template<typename InputIterator>
      result_type operator()(InputIterator first, InputIterator last) const {
        concat_responses ret;
        for (; first != last; ++first) {
          if (ret.rcode != RCODE_servfail && first->rcode != RCODE_noerror)
            ret.rcode = (ret.rcode != RCODE_noerror ? RCODE_servfail : first->rcode);

          ret.an.insert(ret.an.end(), first->answer.begin(), first->answer.end());
          ret.ns.insert(ret.ns.end(), first->authority.begin(), first->authority.end());
          ret.ad.insert(ret.ad.end(), first->additional.begin(), first->additional.end());
        }
        return ret;
      }
    };
  public:
    struct hook_ret {
      std::vector<response> answer;
      std::vector<response> authority;
      std::vector<response> additional;
      rcode_t rcode = RCODE_noerror;
    };

  private:
    boost::asio::io_context* io_ctx;
    boost::asio::ip::udp::socket sock;
    std::array<uint8_t, 512> buf;
    boost::asio::ip::udp::endpoint remote;
    boost::asio::thread_pool responder_pool{1024};

  private:
    void start_receive();
  public:
    boost::signals2::signal<hook_ret(const request&), dns_concat> recv;

  public:
    server(boost::asio::io_context* io_ctx, uint16_t port = 53);
  };

  class client {};
}
