//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <stdlib.h>

#include "server.h"

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 3)
    {
      std::cerr << "Usage: ndp <port> <dataProviderType> <dataPath>\n";
      return 1;
    }

    Server s(std::atoi(argv[1]), argv[2], argv[3]);

    s.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
