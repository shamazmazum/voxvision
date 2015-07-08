FIND_PATH(GCD_INCLUDE_DIR NAMES dispatch/dispatch.h)
MARK_AS_ADVANCED(GCD_INCLUDE_DIR)
 
FIND_LIBRARY(GCD_LIBRARY NAMES
dispatch
libdispatch
dispatchlib
)
MARK_AS_ADVANCED(GCD_LIBRARY)
 
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GCD
   "Could not find GCD. voxrnd will still be working, but slow.
   If you are using FreeBSD, consider installing devel/libdispatch
   and devel/compiler-rt ports."
GCD_LIBRARY GCD_INCLUDE_DIR)
 
IF(GCD_FOUND)
SET(GCD_LIBRARIES ${GCD_LIBRARY})
SET(GCD_INCLUDE_DIRS ${GCD_INCLUDE_DIR})
ENDIF(GCD_FOUND)
