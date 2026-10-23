# Aucun objet ni en-tête supplémentaire hors de l’option diagnostique.
if(CONFIG_SPLITERGOLL_POWER_TRACE)
  zephyr_library()
  zephyr_library_sources(power_trace.c power_store.c)
  zephyr_include_directories(include)
  zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/../../os-signature/firmware)
endif()
