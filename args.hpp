#ifndef __ARGS_HPP__
#define __ARGS_HPP__
#include <netinet/in.h>
#include <string>

class Args {
   public:
    struct sockaddr_in address;
    std::string mode;
    Args(int argc, char** argv);
    Args();
};

#endif  // __ARGS_HPP__
