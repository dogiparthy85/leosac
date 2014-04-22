/**
 * \file isocket.hpp
 * \author Thibault Schueller <ryp.sqrt@gmail.com>
 * \brief socket interface
 */

#ifndef ISOCKET_HPP
#define ISOCKET_HPP

#include <string>

namespace Rezzo
{
    enum Mode {
        None =      0,
        Read =      1 << 0,
        Write =     1 << 1,
        ReadWrite = Read | Write
    };

    class ISocket
    {
    public:
        enum Protocol {
            TCP = 0,
            UDP
        };

        typedef uint16_t Port;
        typedef uint32_t Ip;
        typedef struct s_address {
            Port    port;
            Ip      ip;
            bool operator==(const struct s_address& other) const
            {
                return (ip == other.ip && port == other.port);
            }
            bool operator<(const struct s_address& other) const
            {
                return (ip < other.ip);
            }
        } Address;

    public:
        virtual ~ISocket() {}
        virtual std::size_t recv(char* data, std::size_t size, int flags = 0) = 0;
        virtual std::size_t send(const char* data, std::size_t size, int flags = 0) = 0;
        virtual void        connect(const std::string& host, Port port) = 0;
        virtual void        bind(Port port) = 0;
        virtual void        listen(int backLogSize = 0) = 0;
        virtual ISocket*    accept() = 0;
        virtual void        close() = 0;
        virtual std::string getIp() const = 0;
        virtual Port        getPort() const = 0;
        virtual Address     getAddress() const = 0;
        virtual int         getHandle() const = 0;
    };
}

#endif // ISOCKET_HPP
