cmake_minimum_required(VERSION 3.22)
# IDF's project macro resets its minimum policy version to 3.16. Keep the
# copied response-file toolchain variable during CMake compiler detection.
set(CMAKE_POLICY_DEFAULT_CMP0126 NEW)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(EXTRA_CMAKE_ARGS "-DCMAKE_POLICY_DEFAULT_CMP0126=NEW" APPEND)
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES CMAKE_TOOLCHAIN_FILE)
# The migration checkpoint preserves the old version; this is the MCP development build.
set(PROJECT_VER "0.9.10-ble-state")
project(anan_stopwatch)
# Activate the existing pinned lwIP responder on raw USB and SDK Wi-Fi netifs.
# Apply definitions globally: every component must use the same netif ABI.
idf_build_set_property(COMPILE_DEFINITIONS "LWIP_MDNS_RESPONDER=1" APPEND)
idf_build_set_property(COMPILE_DEFINITIONS "LWIP_MDNS_SEARCH=0" APPEND)
idf_component_get_property(stopwatch_lwip_lib lwip COMPONENT_LIB)
idf_component_get_property(stopwatch_lwip_dir lwip COMPONENT_DIR)
# The pinned upstream timer waits for both address families to send a probe.
# Keep IPv4 discovery usable when no IPv6 link-local address was created. The
# SDK stays untouched; compile one attributed adapter under ignored build/.
set(stopwatch_mdns_original "${stopwatch_lwip_dir}/lwip/src/apps/mdns/mdns.c")
file(SHA256 "${stopwatch_mdns_original}" stopwatch_mdns_hash)
if(NOT stopwatch_mdns_hash STREQUAL "2bd12a3bb8b3c8830c56c2b5c3b8073ab5f71654171602210f63cf6dafc8d5e9")
    message(FATAL_ERROR "Pinned lwIP mDNS source changed; review the IPv4-only adapter")
endif()
file(READ "${stopwatch_mdns_original}" stopwatch_mdns_source)
set(stopwatch_mdns_condition "if (mdns_send_probe(netif, &v6group) == ERR_OK)")
string(FIND "${stopwatch_mdns_source}" "${stopwatch_mdns_condition}" stopwatch_mdns_match)
if(stopwatch_mdns_match EQUAL -1)
    message(FATAL_ERROR "Pinned mDNS IPv6 probe condition missing")
endif()
string(REPLACE "${stopwatch_mdns_condition}"
    "if (!ip6_addr_isvalid(netif_ip6_addr_state(netif, 0)) || mdns_send_probe(netif, &v6group) == ERR_OK)"
    stopwatch_mdns_source "${stopwatch_mdns_source}")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/mdns-adapter")
file(WRITE "${CMAKE_BINARY_DIR}/mdns-adapter/mdns.c" "${stopwatch_mdns_source}")
target_sources(${stopwatch_lwip_lib} PRIVATE
    "${CMAKE_BINARY_DIR}/mdns-adapter/mdns.c"
    "${stopwatch_lwip_dir}/lwip/src/apps/mdns/mdns_out.c"
    "${stopwatch_lwip_dir}/lwip/src/apps/mdns/mdns_domain.c")
