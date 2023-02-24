include(FetchContent)

FetchContent_Declare(
  argtable3
  GIT_REPOSITORY https://github.com/argtable/argtable3.git
  GIT_TAG f25c624e3a5a65f4d1897aa7d51c897f8f8098d0
)

set(ARGTABLE3_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ARGTABLE3_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)


FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

enable_testing()

FetchContent_MakeAvailable(googletest argtable3)