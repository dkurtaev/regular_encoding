#include <string>
#include <iostream>

#include <gtest/gtest.h>

#include "include/main.h"

TEST(Hello, say_hello) {
  std::string str = SayHello();
  ASSERT_NE(str, "Hello!");
  std::cout << str << std::endl;
}

