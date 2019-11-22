find_package(Boost 1.71 REQUIRED coroutine thread container)

add_executable(hello-world hello_world/main.cpp)
target_link_libraries(hello-world PRIVATE foxy Boost::coroutine Boost::thread)

add_executable(ip-parser uri_combinators/ip_parser.cpp)
target_link_libraries(ip-parser PRIVATE foxy Boost::coroutine Boost::thread)

add_executable(client allocators/client.cpp)
target_link_libraries(client PRIVATE foxy Boost::container Boost::thread)

add_executable(server server/main.cpp)
target_link_libraries(server PRIVATE foxy Boost::container Boost::thread Boost::coroutine)

add_executable(https-server https_server/main.cpp)
target_link_libraries(
  https-server
  PRIVATE
    foxy
    test_utils
    Boost::container
    Boost::thread
    Boost::coroutine
)

add_executable(listener listener/main.cpp)
target_link_libraries(listener PRIVATE foxy test_utils Boost::thread Boost::coroutine)
