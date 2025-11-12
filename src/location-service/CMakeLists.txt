# Location Service module for ns-3

set(source_files
    model/location-service.cc
    model/god.cc
)

set(header_files
    model/location-service.h
    model/god.h
)

build_lib(
  LIBNAME location-service
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${libnetwork}
)
