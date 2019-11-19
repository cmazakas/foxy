find_package(Catch2 CONFIG REQUIRED)
find_package(Boost 1.71 REQUIRED coroutine thread container)

function(foxy_add_test test_name)
  add_executable(${test_name} ${ARGN})

  target_link_libraries(
    ${test_name}
    PRIVATE
      test_utils
      Catch2::Catch2
      Boost::coroutine
      Boost::thread
      Boost::container
  )

  include(Catch)
  catch_discover_tests(${test_name})
endfunction()

foxy_add_test(allocator_client_test allocator_client_test.cpp)
foxy_add_test(client_session_test client_session_test.cpp)
foxy_add_test(code_point_view_test code_point_view_test.cpp)
foxy_add_test(export_connect_fields_test export_connect_fields_test.cpp)
foxy_add_test(iterator_test iterator_test.cpp)
foxy_add_test(listener_test listener_test.cpp)
foxy_add_test(parse_uri_test parse_uri_test.cpp)
foxy_add_test(pct_decode_test pct_decode_test.cpp)
foxy_add_test(pct_encode_test pct_encode_test.cpp)
foxy_add_test(proxy_test proxy_test.cpp)
foxy_add_test(proxy_test2 proxy_test2.cpp)
foxy_add_test(relay_test relay_test.cpp)
foxy_add_test(server_session_test server_session_test.cpp)
foxy_add_test(session_test session_test.cpp)
foxy_add_test(speak_test speak_test.cpp)
foxy_add_test(ssl_client_session_test ssl_client_session_test.cpp)
foxy_add_test(timed_op_wrapper_v3 timed_op_wrapper_v3.cpp)
foxy_add_test(unicode_uri_test unicode_uri_test.cpp)
foxy_add_test(uri_test uri_test.cpp)
foxy_add_test(utility_test utility_test.cpp)

file(COPY root-cas.pem DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
