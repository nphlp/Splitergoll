# Compile uniquement le diagnostic demandé, sans modifier les dépendances figées.
if(CONFIG_SPLITERGOLL_USB_TRACE)
  if(CONFIG_USB_DEVICE_STACK_NEXT OR CONFIG_LTO OR NOT CONFIG_SOC_NRF52840)
    message(FATAL_ERROR "USB trace requires the legacy nRF52840 USB stack without LTO")
  endif()

  zephyr_library()
  zephyr_library_sources(usb_trace.c usb_trace_store.c usb_os_classifier.c)
  zephyr_link_libraries(
    -Wl,--wrap=usb_dc_ep_set_callback
    -Wl,--wrap=usb_dc_ep_read
    -Wl,--wrap=usb_dc_set_status_callback
  )
endif()
