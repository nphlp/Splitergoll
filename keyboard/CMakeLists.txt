# Compile l’observateur partagé seulement pour auto ou le diagnostic USB explicite.
if(CONFIG_SPLITERGOLL_USB_OBSERVER)
  if(CONFIG_USB_DEVICE_STACK_NEXT OR CONFIG_LTO OR NOT CONFIG_SOC_NRF52840)
    message(FATAL_ERROR "USB observation requires the legacy nRF52840 USB stack without LTO")
  endif()
  zephyr_library()
  zephyr_library_include_directories(include config/keymaps ${APPLICATION_SOURCE_DIR}/include)
  zephyr_library_sources(src/os_detection/usb_observer.c src/os_detection/usb_os_classifier.c)
  zephyr_include_directories(include)
  zephyr_link_libraries(
    -Wl,--wrap=usb_dc_ep_set_callback
    -Wl,--wrap=usb_dc_ep_read
    -Wl,--wrap=usb_dc_set_status_callback
  )
  if(CONFIG_SPLITERGOLL_OS_AUTO)
    zephyr_library_sources(src/os_detection/usb_auto_state.c src/os_detection/usb_auto.c)
    zephyr_link_libraries(-Wl,--wrap=zmk_event_manager_raise -Wl,--wrap=zmk_keymap_layer_to)
  endif()
endif()
