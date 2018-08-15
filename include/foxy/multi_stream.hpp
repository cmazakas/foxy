#ifndef FOXY_MULTI_STREAM_HPP_
#define FOXY_MULTI_STREAM_HPP_

#include <boost/asio/io_context.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/experimental/core/ssl_stream.hpp>

#include <boost/optional/optional.hpp>

#include <utility>

namespace foxy
{

struct multi_stream
{
public:
  using stream_type     = boost::asio::ip::tcp::socket;
  using ssl_stream_type = boost::beast::ssl_stream<stream_type&>;
  using executor_type   = boost::asio::io_context::executor_type;

private:
  stream_type                      stream_;
  boost::optional<ssl_stream_type> ssl_stream_;

public:
  multi_stream()                    = delete;
  multi_stream(multi_stream const&) = default;
  multi_stream(multi_stream&&)      = default;

  explicit multi_stream(boost::asio::io_context&);

  auto tcp() & noexcept -> stream_type&;
  auto ssl() & noexcept -> ssl_stream_type&;

  auto is_ssl() const noexcept -> bool;

  auto get_executor() -> executor_type;

  template <class MutableBufferSequence, class CompletionToken>
  auto
  async_read_some(MutableBufferSequence const& buffers, CompletionToken&& token)
  {
    if (ssl_stream_) {
      return ssl_stream_->async_read_some(buffers, std::forward<CompletionToken>(token));
    }
    return stream_.async_read_some(buffers, std::forward<CompletionToken>(token));
  }

  template <class ConstBufferSequence, class CompletionToken>
  auto
  async_write_some(ConstBufferSequence const& buffers, CompletionToken&& token)
  {
    if (ssl_stream_) {
      return ssl_stream_->async_write_some(buffers, std::forward<CompletionToken>(token));
    }
    return stream_.async_write_some(buffers, std::forward<CompletionToken>(token));
  }
};

} // foxy

#endif // FOXY_MULTI_STREAM_HPP_