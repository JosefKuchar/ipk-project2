#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include "args.hpp"

class Server {
   protected:
    Args args;

   public:
    Server(Args args);
    static Server* create(Args args);
    virtual void run(){};
};

#endif  // __SERVER_HPP__
