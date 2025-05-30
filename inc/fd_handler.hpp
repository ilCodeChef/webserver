#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <vector>
# include <map>
# include <sys/socket.h>
# include <sys/epoll.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <iostream>
# include <unistd.h>
# include <cstring>
# include <chrono>

# include "defines.hpp"

class Fd_handler
{
    protected:

	    int 					_fd;
        std::chrono::time_point<std::chrono::steady_clock> _last_activity;

    public:

        Fd_handler() = default;
        virtual ~Fd_handler() = default;

        int 					get_fd() const {return this->_fd;}
        void                    set_fd(int fd) {this->_fd = fd;}
        virtual void            input(void){return;}
        virtual void            output(void){return;}
		virtual void			hangup(void){return;}
        
        void                    reset_last_activity(){_last_activity = std::chrono::steady_clock::now();}
        bool                    has_timeout()
        {
            auto now = std::chrono::steady_clock::now();
            auto difference = std::chrono::duration_cast<std::chrono::seconds>(now - _last_activity);
            if (difference.count() > TIMEOUT)
                return true;
            return false;
        }


};

void make_socket_non_blocking(int socket_fd);

#endif