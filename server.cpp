/*
 * tls-example
 * Copyright (c) 2020 Peter Nebe (mail@peter-nebe.dev)
 *
 * This file is part of tls-example.
 *
 * tls-example is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tls-example is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tls-example.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/algorithm/string/split.hpp"
#include <iostream>
#include <string>
using boost::asio::ip::tcp;
using namespace boost::asio;
using namespace std;

const size_t maxLength = 1000;

class Session : public enable_shared_from_this<Session>
{
public:
  Session(ssl::stream<tcp::socket> sock)
  : socket(move(sock)),
    readbuf(maxLength, '\0')
  {
  }

  void start()
  {
    handshake();
  }

private:
  void handshake()
  {
    auto self = shared_from_this();
    socket.async_handshake(ssl::stream_base::server, [this, self](const boost::system::error_code &error)
                                                     {
                                                       if(error)
                                                         cout << "Handshake failed: " << error.message() << endl;
                                                       else
                                                         receiveRequest();
                                                     });
  }

  void receiveRequest()
  {
    auto self = shared_from_this();
    socket.async_read_some(buffer(readbuf), [this, self](const boost::system::error_code &error, size_t length)
                                            {
                                              if(!error)
                                                sendResponse(length);
                                            });
  }

  void sendResponse(size_t length)
  {
    auto self = shared_from_this();
    vector<string> words;
    boost::algorithm::split(words, readbuf.substr(0, length), [](char c){ return c == ' '; });

    ostringstream oss;
    if(!words.empty())
    {
      auto wi = words.crbegin();
      oss << *wi++;
      for( ; wi != words.crend(); ++wi)
        oss << ' ' << *wi;
    }

    writebuf = oss.str();
    async_write(socket, buffer(writebuf), [this, self](const boost::system::error_code &error, size_t)
                                          {
                                            if(!error)
                                              receiveRequest();
                                          });
  }

  ssl::stream<tcp::socket> socket;
  string readbuf;
  string writebuf;
};

bool verifyClient(bool preverified, ssl::verify_context &ctx)
{
  if(!preverified)
    return false;

  if(X509_STORE_CTX_get_error_depth(ctx.native_handle()) > 0)
    return true;

  X509_NAME *subjectName = X509_get_subject_name(X509_STORE_CTX_get_current_cert(ctx.native_handle()));
  const X509_NAME_ENTRY *nameEntry = X509_NAME_get_entry(subjectName, X509_NAME_get_index_by_NID(subjectName, NID_commonName, -1));
  const string commonName = reinterpret_cast<const char*>(X509_NAME_ENTRY_get_data(nameEntry)->data);

  return commonName.starts_with("tls-example-client");
}

class Server
{
public:
  Server(io_context &ioctx, uint16_t port)
  : acceptor(ioctx, tcp::endpoint(tcp::v4(), port)),
    sslctx(ssl::context::tlsv13_server)
  {
    sslctx.set_options(ssl::context::default_workarounds);
    sslctx.set_password_callback([](size_t, ssl::context::password_purpose){ return "tls-example"s; });
    sslctx.use_private_key_file("tls-example-server.key", ssl::context::pem);
    sslctx.use_certificate_file("tls-example-server.crt", ssl::context::pem);

    sslctx.load_verify_file("tls-example-ca.crt");
    sslctx.set_verify_mode(ssl::verify_peer | ssl::verify_fail_if_no_peer_cert);
    sslctx.set_verify_callback(verifyClient);

    accept();
  }

private:
  void accept()
  {
    acceptor.async_accept([this](const boost::system::error_code &error, tcp::socket sock)
                          {
                            if(!error)
                              make_shared<Session>(ssl::stream<tcp::socket>(move(sock), sslctx))->start();

                            accept();
                          });
  }

  tcp::acceptor acceptor;
  ssl::context sslctx;
};

int main(int argc, char *argv[])
{
  try
  {
    if(argc != 2)
    {
      cerr << "Usage: " << argv[0] << " PORT" << endl;
      return 1;
    }

    const uint16_t port = stoul(argv[1]);
    io_context ioctx;
    Server svr(ioctx, port);

    ioctx.run();
  }
  catch(const exception &ex)
  {
    cerr << "Exception: " << ex.what() << endl;
    return 1;
  }

  return 0;
}
