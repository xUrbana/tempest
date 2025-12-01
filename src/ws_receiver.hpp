#pragma once
#include "tsqueue.hpp"
#include <memory>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

namespace tempest
{
using ws_client = websocketpp::client<websocketpp::config::asio_client>;
using json      = nlohmann::json;

class WebsocketReceiver
{
  public:
    using QueueType = ThreadSafeQueue<std::string>;

    WebsocketReceiver(const std::string&         token,
                      std::string                device_id,
                      std::shared_ptr<QueueType> queue)
      : device_id_(std::move(device_id))
      , ws_uri_("ws://ws.weatherflow.com/swd/data?token=" + token)
      , queue_(std::move(queue))
    {
        if (token.size() <= 0 || device_id_.size() <= 0)
        {
            throw std::runtime_error(std::format(
              R"(Invalid token or device id passed to websocket receiver. Token: "{}", Device ID: "{}")",
              token,
              device_id));
        }

        init();
    }

    void run() { ws_client_.run(); }

  private:
    void init()
    {
        ws_client_.set_access_channels(websocketpp::log::alevel::all);
        ws_client_.clear_access_channels(websocketpp::log::alevel::frame_payload);
        ws_client_.set_error_channels(websocketpp::log::elevel::all);
        ws_client_.init_asio();
        ws_client_.set_message_handler([this](auto handle, auto msg)
                                       { return handle_receive(handle, msg); });

        websocketpp::lib::error_code ec;
        ws_con_ptr_ = ws_client_.get_connection(ws_uri_, ec);
        if (!ws_con_ptr_)
        {
            throw std::runtime_error(std::format("failed to create connection: {}", ec.message()));
        }
        ws_client_.connect(ws_con_ptr_);
    }

    void handle_receive(websocketpp::connection_hdl hdl, ws_client::message_ptr msg)
    {
        auto payload_string = msg->get_payload();
        spdlog::debug("ws: {}\n", msg->get_payload());

        auto payload = json::parse(payload_string);
        if (payload["type"] == "connection_opened")
        {
            json listen_start = { { "type", "listen_start" },
                                  { "device_id", device_id_ },
                                  { "id", "1" } };
            ws_client_.send(hdl, listen_start.dump(), msg->get_opcode());
        }
        else if (payload["type"] == "obs_st")
        {
            queue_->push(std::move(payload_string));
        }
        else
        {
            // nothing for now
        }
    }

    std::string                device_id_;
    std::string                ws_uri_;
    ws_client                  ws_client_;
    ws_client::connection_ptr  ws_con_ptr_;
    std::shared_ptr<QueueType> queue_;
};

}