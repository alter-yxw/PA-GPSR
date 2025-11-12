# PA-GPSR routing protocol module for ns-3

set(source_files
    model/pagpsr-rtable.cc
    model/pagpsr-rst-table.cc
    model/pagpsr-ptable.cc
    model/pagpsr-rqueue.cc
    model/pagpsr-packet.cc
    model/pagpsr.cc
    helper/pagpsr-helper.cc
)

set(header_files
    model/pagpsr-rtable.h
    model/pagpsr-rst-table.h
    model/pagpsr-ptable.h
    model/pagpsr-rqueue.h
    model/pagpsr-packet.h
    model/pagpsr.h
    helper/pagpsr-helper.h
)

build_lib(
  LIBNAME pagpsr
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
