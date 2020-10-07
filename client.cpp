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
#include <iostream>
#include <string>
using boost::asio::ip::tcp;
using namespace boost::asio;
using namespace std;

const size_t maxLength = 1000;

class SslContext : public ssl::context
{
public:
  SslContext()
  : ssl::context(ssl::context::tlsv13_client)
  {
    set_password_callback([](size_t, ssl::context::password_purpose){ return "tls-example"s; });
    use_private_key_file("tls-example-client-1.key", ssl::context::pem);
    use_certificate_file("tls-example-client-1.crt", ssl::context::pem);

    load_verify_file("tls-example-ca.crt");
    set_verify_mode(ssl::verify_peer | ssl::verify_fail_if_no_peer_cert);
    set_verify_callback(ssl::host_name_verification("tls-example-server"));
  }
};

class Client
{
public:
  Client(io_context &ioctx, const char *server, const char *port)
  : socket(ioctx, sslctx),
    readbuf(maxLength, '\0')
  {
    connect(tcp::resolver(ioctx).resolve(server, port));
  }

private:
  void connect(const tcp::resolver::results_type &endpoints)
  {
    async_connect(socket.lowest_layer(), endpoints, [this](const boost::system::error_code &error, const tcp::endpoint&)
                                                    {
                                                      if(error)
                                                        cout << "Connect failed: " << error.message() << endl;
                                                      else
                                                        handshake();
                                                    });
  }

  void handshake()
  {
    socket.async_handshake(ssl::stream_base::client, [this](const boost::system::error_code &error)
                                                     {
                                                       if(error)
                                                         cout << "Handshake failed: " << error.message() << endl;
                                                       else
                                                         sendRequest();
                                                     });
  }

  void sendRequest()
  {
    cout << "Enter phrase: ";
    getline(cin, writebuf);

    async_write(socket, buffer(writebuf, maxLength), [this](const boost::system::error_code &error, size_t length)
                                                     {
                                                       if(error)
                                                         cout << "Write failed: " << error.message() << endl;
                                                       else
                                                         receiveResponse(length);
                                                     });
  }

  void receiveResponse(size_t length)
  {
    async_read(socket, buffer(readbuf, length), [this](const boost::system::error_code &error, size_t length)
                                                {
                                                  if(error)
                                                  {
                                                    cout << "Read failed: " << error.message() << endl;
                                                  }
                                                  else
                                                  {
                                                    cout << "Response: " << readbuf.substr(0, length) << endl;
                                                    sendRequest();
                                                  }
                                                });
  }

  SslContext sslctx;
  ssl::stream<tcp::socket> socket;
  string writebuf;
  string readbuf;
};

int main(int argc, char *argv[])
{
  try
  {
    if(argc != 3)
    {
      cerr << "Usage: " << argv[0] << " SERVER PORT" << endl;
      return 1;
    }

    const char *server = argv[1];
    const char *port = argv[2];
    io_context ioctx;
    Client clnt(ioctx, server, port);

    ioctx.run();
  }
  catch(const exception &ex)
  {
    cerr << "Exception: " << ex.what() << endl;
    return 1;
  }

  return 0;
}
