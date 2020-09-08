#include "server/asio.h"
#include "asio/error_code.hpp"
#include "asio/ip/address_v4.hpp"
#include "asio/ip/tcp.hpp"

#include <iostream>

namespace webcrown {
namespace server {

void HttpAcceptor::initialize() {
  asio::error_code Ec;

  std::cout << "Initializing...\n";

  // We are a server, so we doesnt need specify the Ip Address.
  // The asio::ip::address_v4 get the ip
  asio::ip::tcp::endpoint Endpoint(asio::ip::address_v4(), PortNum);

  SocketAcceptor.open(Endpoint.protocol(), Ec);
  if(Ec.value() != 0) {
    //TODO: logger
    std::cout
      << "Error on open socket. "
      << "Message: "
      << Ec.message()
      << "\n";
    assert(Ec.value() != 0 && "Error on open socket.");
    return;
  }

  SocketAcceptor.bind(Endpoint, Ec);
  if(Ec.value() != 0) {
    // TODO: logger
    std::cout 
      << "Error on bind endpoint to the socket. "
      << "Message: "
      << Ec.message()
      << "\n";
    assert(Ec.value() != 0 && "Error on bind endpoint to the socket");
    return;
  }

  IsInitialized = true;
}

void HttpAcceptor::run() {
  if(!IsInitialized) {
    // TODO: Logger
    assert(!IsInitialized && "The context is not initialized");
    return;
  }

   // Notify the OS that we want to listening for incoming connection requests
  // on specific endpoint by this call.
  SocketAcceptor.listen();
  processNextRequest();
}

void HttpAcceptor::stop() {
  IsStoped.store(true);
}

void HttpAcceptor::processNextRequest() {
  // Create the active socket to handle the client
  // connection request
  std::shared_ptr<asio::ip::tcp::socket> Sock(
    new asio::ip::tcp::socket(Ioc));

  SocketAcceptor.async_accept(
    *Sock.get(),
    [this, Sock](asio::error_code const& Ec)
    {
      onAccept(Ec, Sock);
    });

  // Handle next connection
 // if(!IsStoped.load()) {
 //   //processNextRequest();
 // }
 // else {
 //   SocketAcceptor.close();
 // }
}

void HttpAcceptor::onAccept(asio::error_code const& Ec,
				std::shared_ptr<asio::ip::tcp::socket> Sock) {
  if(Ec.value() != 0) {
    // TODO: logger
    assert(Ec.value() != 0 && "Error on async_accept");
    return;
  }

  // TODO: ponteiro solto?
  (new HandleClientRequest(Sock))->startHandling();
}

}}
