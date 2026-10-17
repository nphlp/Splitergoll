# Le stockage et le lecteur restent dans le diagnostic, l’observateur est partagé.
if(CONFIG_SPLITERGOLL_USB_TRACE)
  zephyr_library()
  zephyr_library_sources(usb_trace.c usb_trace_store.c)
endif()
