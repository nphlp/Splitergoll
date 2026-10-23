cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(splitergoll_activity)
if(NOT DEFINED ZMK_APP_SOURCE OR NOT EXISTS "${ZMK_APP_SOURCE}/src/activity.c")
  message(FATAL_ERROR "Provide the pinned ZMK_APP_SOURCE from the prepared workspace")
endif()
get_filename_component(REPOSITORY "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
target_include_directories(app PRIVATE stubs "${REPOSITORY}" "${REPOSITORY}/keyboard/include"
  "${ZMK_APP_SOURCE}/src"
  "${REPOSITORY}/diagnostics/power/firmware/include")
target_compile_definitions(app PRIVATE CONFIG_ZMK_IDLE_TIMEOUT=10 CONFIG_ZMK_IDLE_SLEEP_TIMEOUT=60
  CONFIG_ZMK_SLEEP=1 CONFIG_ZMK_LOG_LEVEL=0)
target_sources(app PRIVATE src/activity_test.c "${REPOSITORY}/keyboard/src/connections/host_preference.c"
  "${REPOSITORY}/keyboard/src/connections/host_window_policy.c"
  "${REPOSITORY}/diagnostics/power/firmware/power_store.c")
