/*
 * Copyright 2020 Alex Syrnikov <pioneer19@post.cz>
 *
 * This file is part of libcornet.
 *
 *  libcornet is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libcornet is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libcornet.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>
#include <utility>

#include <libcornet/tcp_socket.hpp>
#include <libcornet/tls/record_layer.hpp>
#include <libcornet/coroutines_utils.hpp>

namespace pioneer19::cornet::tls13
{

class TlsSocket
{

public:
    TlsSocket()                         = default;
    TlsSocket( TlsSocket&& )            = default;
    TlsSocket& operator=( TlsSocket&& ) = default;
    ~TlsSocket() = default;

    explicit TlsSocket( RecordLayer&& ) noexcept;

    void bind( const char* ip_address, uint16_t port );
    void listen( Poller& poller );
    coroutines::CoroutineAwaiter<TlsSocket> async_accept(
            Poller& poller, sockaddr_in6* peer_addr, KeyStore* keys_store );
    coroutines::CoroutineAwaiter<bool> async_connect( Poller& poller, const char* hostname, uint16_t port
            , const char* sni=nullptr );
    auto async_read( void* buffer, size_t buffer_size )
    { return m_record_layer.async_read( buffer, buffer_size ); }
    auto async_write( const void* buffer, size_t buffer_size )
    { return m_record_layer.async_write( buffer, buffer_size ); }

    TlsSocket( const TlsSocket& )       = delete;
    TlsSocket& operator=( const TlsSocket& ) = delete;

private:
    RecordLayer m_record_layer;
};

inline TlsSocket::TlsSocket( RecordLayer&& record_layer ) noexcept
        : m_record_layer( std::move(record_layer) )
{}

inline void TlsSocket::bind( const char* ip_address, uint16_t port )
{
    m_record_layer.bind(ip_address,port);
}

inline void TlsSocket::listen( Poller& poller )
{
    m_record_layer.listen(poller);
}

inline coroutines::CoroutineAwaiter<TlsSocket> TlsSocket::async_accept(
        Poller& poller, sockaddr_in6* peer_addr, KeyStore* keys_store )
{
    return m_record_layer.tls_accept( poller, peer_addr, keys_store );
}

inline coroutines::CoroutineAwaiter<bool> TlsSocket::async_connect(
        Poller& poller, const char* hostname, uint16_t port, const char* sni )
{
    std::string tls_sni;
    if( sni )
        tls_sni = sni;
    else
        tls_sni = hostname;

    bool connected = co_await m_record_layer.tls_connect( poller, hostname, port, tls_sni );

    co_return connected;
}

}
