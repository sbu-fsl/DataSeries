#
# (c) Copyright 2008, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# cmake rules for the src/raw-collection directory

ADD_SUBDIRECTORY(lsf)
ADD_SUBDIRECTORY(network-driverdump)

IF(LINUX_IF_PACKET_ENABLED AND PCAP_ENABLED)
    ADD_EXECUTABLE(lindump-mmap lindump-mmap.c)
    INSTALL(TARGETS lindump-mmap DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
    LINTEL_POD2MAN(lindump-mmap.c 1 ${DATASERIES_POD2MAN_RELEASE} "-" lindump-mmap.1)
ENDIF(LINUX_IF_PACKET_ENABLED AND PCAP_ENABLED)

