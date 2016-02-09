#include <string>
#include <iostream>

#include <gtest/gtest.h>

#include "include/main.h"

TEST(Hello, say_hello) {
  std::string str = SayHello();
  ASSERT_EQ(str, "Hello!");
  std::cout << str << std::endl;
}

