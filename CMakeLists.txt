cmake_minimum_required (VERSION 3.17)
project (tls-example)

link_libraries (
  ssl
  crypto
  pthread
)
set (CMAKE_CXX_STANDARD
  20
)
set (CMAKE_CXX_STANDARD_REQUIRED
  ON
)
add_executable (
  ${PROJECT_NAME}-client
  client.cpp
)
add_executable (
  ${PROJECT_NAME}-server
  server.cpp
)
