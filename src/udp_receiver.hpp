#pragma once
#include "tsqueue.hpp"
#include <array>
#include <boost/asio.hpp>
#include <cstdint>
#include <memory>
#include <net/if.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>

namespace tempest
{
class UDPReceiver
{
  public:
    using QueueType = ThreadSafeQueue<std::string>;

    UDPReceiver(uint16_t port, std::shared_ptr<ThreadSafeQueue<std::string>> queue)
      : queue_(std::move(queue))
      , socket_(io_context_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
    {
        socket_.set_option(boost::asio::socket_base::broadcast(true));
        socket_.set_option(boost::asio::socket_base::reuse_address(true));
        receive();
    }

    void run() { io_context_.run(); }

    void receive()
    {
        spdlog::debug("Starting receive...");
        socket_.async_receive_from(boost::asio::buffer(buffer_),
                                   remote_endpoint_,
                                   [this](auto error, auto bytes_read)
                                   { return handle_receive(error, bytes_read); });
    }

    void handle_receive([[maybe_unused]] const boost::system::error_code& error,
                        std::size_t                                       bytes_read)
    {
        // Copy data into packet
        std::string packet(buffer_.data(), bytes_read);

        // Start the next receive
        receive();

        spdlog::debug("Received packet: ", error.to_string());
        queue_->push(std::move(packet));
    }

  private:
    boost::asio::io_context        io_context_;
    std::shared_ptr<QueueType>     queue_;
    boost::asio::ip::udp::socket   socket_;
    std::array<char, 1024>         buffer_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
};
}
