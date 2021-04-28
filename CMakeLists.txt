cmake_minimum_required(VERSION 3.12)

project(FLTLIB)

set(FLT_SRCS
  buffer.c
  fileio.c
  records.c
  stack.c
  writer.c
  math.c
  zmalloc.c
  util.c
  )

include(CheckLibraryExists)
check_library_exists(m cos "" HAVE_M_LIBRARY)
if (HAVE_M_LIBRARY)
  set(M_LIBRARY m)
endif (HAVE_M_LIBRARY)

add_library(fltlib SHARED ${FLT_SRCS})
if (M_LIBRARY)
  target_link_libraries(fltlib ${M_LIBRARY})
endif (M_LIBRARY)

add_executable(flt flt.c)
target_link_libraries(flt fltlib)

add_executable(fltw fltw.c)
target_link_libraries(fltw fltlib)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
