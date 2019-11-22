target_sources(
  foxy

  PRIVATE
    log.cpp
    proxy.cpp
    parse_uri.cpp
    utility.cpp

    # TODO: someday make this work
    #
    # uri.cpp
)

set(FOXY_FAST_BUILD ON CACHE BOOL "Builds Foxy with separately compiled Beast and Asio")
if(FOXY_FAST_BUILD)
  target_compile_definitions(
    foxy

    PUBLIC
      BOOST_ASIO_SEPARATE_COMPILATION=1
      BOOST_BEAST_SEPARATE_COMPILATION=1
  )

  target_sources(foxy PRIVATE asio.cpp beast.cpp)
endif()
