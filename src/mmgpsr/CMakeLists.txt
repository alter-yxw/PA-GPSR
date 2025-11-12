# MM-GPSR routing protocol module for ns-3

set(source_files
    model/mmgpsr-ptable.cc
    model/mmgpsr-rqueue.cc
    model/mmgpsr-packet.cc
    model/mmgpsr.cc
    model/mmgpsr-Ttable.cc
    helper/mmgpsr-helper.cc
)

set(header_files
    model/mmgpsr-ptable.h
    model/mmgpsr-Ttable.h
    model/mmgpsr-rqueue.h
    model/mmgpsr-packet.h
    model/mmgpsr.h
    helper/mmgpsr-helper.h
)

build_lib(
  LIBNAME mmgpsr
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
