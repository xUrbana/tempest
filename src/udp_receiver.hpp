#pragma once
#include "tsqueue.hpp"
#include <array>
#include <boost/asio.hpp>
#include <cstdint>
#include <memory>
#include <spdlog/spdlog.h>

struct Packet
{
    std::string                    data;
    boost::asio::ip::udp::endpoint endpoint;
};

class Receiver
{
  public:
    using QueueType = ThreadSafeQueue<Packet>;

    Receiver(boost::asio::io_context &io_context, uint16_t port, std::shared_ptr<ThreadSafeQueue<Packet>> queue)
        : queue_(std::move(queue)),
          socket_(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
    {
        receive();
    }

    void receive()
    {
        spdlog::debug("Starting receive...");
        socket_.async_receive_from(boost::asio::buffer(buffer_), remote_endpoint_,
                                   [this](auto error, auto bytes_read) { return handle_receive(error, bytes_read); });
    }

    void handle_receive([[maybe_unused]] const boost::system::error_code &error, std::size_t bytes_read)
    {
        // Copy data into packet
        Packet packet{.data{buffer_.data(), bytes_read}, .endpoint{remote_endpoint_}};

        // Start the next receive
        receive();

        spdlog::debug("Pushing new packet onto queue...");
        queue_->push(std::move(packet));
    }

  private:
    std::shared_ptr<ThreadSafeQueue<Packet>> queue_;
    boost::asio::ip::udp::socket             socket_;
    std::array<char, 1024>                   buffer_;
    boost::asio::ip::udp::endpoint           remote_endpoint_;
};
