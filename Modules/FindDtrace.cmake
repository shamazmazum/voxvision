# My own module to find dtrace executable
# Vasily Postnicov, 2017

find_program(DTRACE_EXECUTABLE "dtrace")
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Dtrace REQUIRED_VARS DTRACE_EXECUTABLE)
mark_as_advanced(DTRACE_EXECUTABLE)
