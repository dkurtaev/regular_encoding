// Copyright © 2016 Dmitry Kurtaev. All rights reserved.
// License: MIT License (see LICENSE)
// e-mail: dmitry.kurtaev@gmail.com

#ifndef TEST_MACROS_H_
#define TEST_MACROS_H_

#include <sys/time.h>

#include <iostream>

void Log(std::string text, int period = 0) {
  static timeval last_log_time;
  timeval current_time;

  gettimeofday(&current_time, 0);
  if (current_time.tv_sec - last_log_time.tv_sec >= period) {
    std::cout << text << std::endl;
    last_log_time = current_time;
  }
}

#endif  // TEST_MACROS_H_
