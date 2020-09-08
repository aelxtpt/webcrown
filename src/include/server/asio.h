#ifndef SERVER_ASIO_H
#define SERVER_ASIO_H

#include "asio/error_code.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/streambuf.hpp"
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <thread>
#include <atomic>
#include <iostream>
#include <map>
#include <string>
#include <memory>

namespace webcrown {
namespace server {

enum class InternetProtocol
{
  IPv4,
  IPv6
};

/*
 * ///Acceptor
 * This is a high-level acceptor concept (as compared to asio::io::tcp::acceptor).
 * This class is responsible for accepting connection requests arriving from clients and
 * instantiating objects of the Service class
*/
//class HttpAcceptor{
//  asio::io_context& Ioc;
//  unsigned short PortNum;
//  asio::ip::tcp::acceptor SocketAcceptor;
//public:
//  explicit HttpAcceptor(asio::io_context& Ioc, unsigned short PortNum)
//    : Ioc(Ioc)
//    , PortNum(PortNum)
//    , SocketAcceptor(Ioc, asio::ip::tcp::endpoint(
//	asio::ip::address_v4::any(), PortNum))
//  {}
//
//  void run();
//
//};

class HandleClientRequest {
  static const std::map<unsigned int, std::string> HttpStatusTable;

  std::shared_ptr<asio::ip::tcp::socket> ActiveSocket;
  asio::streambuf RequestBuf;
  std::string Response;
  unsigned int ResponseStatusCode;
  std::string RequestedResource; // ???
  std::map<std::string, std::string> RequestHeaders;
  std::string ResponseHeaders;

  std::unique_ptr<char[]> ResourceBuffer;
  std::size_t ResourceSizeBytes;
public:
  /// We receive the active socket corresponding to the
  // socket to handle the client connection request
  explicit HandleClientRequest(
    std::shared_ptr<asio::ip::tcp::socket> ActiveSocket)
    : ActiveSocket(ActiveSocket)
    , RequestBuf(4096) // security concearn, so the client cannot allocate memory more than 4096
    , ResponseStatusCode(200)
    , ResourceBuffer(nullptr)
    , ResourceSizeBytes(0)
  {}

  void startHandling();
private:
  void onRequestReceived(asio::error_code const& Ec,
			 std::size_t BytesTransferred);
  void onHeadersReceived(asio::error_code const& Ec,
			 std::size_t BytesTransferred);
  void onResponseSent(asio::error_code const& Ec,
		      std::size_t BytesTransferred);

  void processRequest();
  void sendResponse();

  void onFinish();
};

class HttpAcceptor{
  asio::io_context& Ioc;
  unsigned short PortNum;
  asio::ip::tcp::acceptor SocketAcceptor;

  bool IsInitialized;
  std::atomic<bool> IsStoped;
public:
  explicit HttpAcceptor(unsigned short PortNum, asio::io_context& Ioc)
    : Ioc(Ioc)
    , PortNum(PortNum)
    , SocketAcceptor(Ioc)
    , IsInitialized(false)
    , IsStoped(false)
  {
  }

  void initialize();
  void run();
  void stop();

private:
  void onAccept(asio::error_code const& Ec,
		std::shared_ptr<asio::ip::tcp::socket> Sock);

  void processNextRequest();
};

class SimpleHttpServer {
  asio::io_context Ioc;
  std::unique_ptr<asio::io_context::work> Work;
  std::unique_ptr<HttpAcceptor> Acceptor;
  std::vector<std::unique_ptr<std::thread>> ThreadPool;
public:
  SimpleHttpServer()
  {
    Work.reset(new asio::io_context::work(Ioc));
  }

  void start(unsigned short PortNum, unsigned int ThreadPoolSize) {
    assert(ThreadPoolSize > 0);

    //Create and start Acceptor
    Acceptor.reset(new HttpAcceptor(PortNum, Ioc));
    Acceptor->initialize();
    Acceptor->run();

    Ioc.run();

    // Create specified numberof threads and
    // add them to the pool
   // for(unsigned int i = 0; i < ThreadPoolSize; ++i) {
   //   std::unique_ptr<std::thread> th(
   //     new std::thread([this]()
   //   {
   //     Ioc.run();
   //   }));

   //   ThreadPool.push_back(std::move(th));
   // }
  }

  void stop() {
    Acceptor->stop();
    Ioc.stop();

    //for(auto const& th : ThreadPool) {
    //  th->join();
    //}
  }
};

}}

#endif
