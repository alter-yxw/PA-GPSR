# GPSR routing protocol module for ns-3

set(source_files
    model/gpsr-ptable.cc
    model/gpsr-rqueue.cc
    model/gpsr-packet.cc
    model/gpsr.cc
    helper/gpsr-helper.cc
)

set(header_files
    model/gpsr-ptable.h
    model/gpsr-rqueue.h
    model/gpsr-packet.h
    model/gpsr.h
    helper/gpsr-helper.h
)

build_lib(
  LIBNAME gpsr
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${liblocation-service}
    ${libinternet}
    ${libwifi}
    ${libapplications}
    ${libmesh}
    ${libpoint-to-point}
    ${libvirtual-net-device}
)
