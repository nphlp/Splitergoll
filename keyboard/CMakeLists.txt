# Compile seulement les fonctions demandées par la keymap ou un diagnostic explicite.
if(CONFIG_SPLITERGOLL_USB_OBSERVER OR CONFIG_SPLITERGOLL_BLE_OBSERVER OR CONFIG_SPLITERGOLL_WIRE_USB_ONLY OR CONFIG_SPLITERGOLL_HOST_IDENTITY)
  zephyr_library()
  zephyr_library_include_directories(include config/keymaps ${APPLICATION_SOURCE_DIR}/include)
  zephyr_include_directories(include)
endif()
if(CONFIG_SPLITERGOLL_HOST_IDENTITY)
  zephyr_library_sources(src/identity/host_identity.c src/identity/host_identity_adapter.c)
  set_source_files_properties(src/identity/host_identity.c src/identity/host_identity_adapter.c
                             PROPERTIES COMPILE_OPTIONS "-Werror")
  zephyr_link_libraries(-Wl,--wrap=usb_enable)
endif()
if(CONFIG_SPLITERGOLL_USB_PRIORITY)
  if(NOT CONFIG_SPLITERGOLL_HOST_IDENTITY OR NOT CONFIG_SPLITERGOLL_BLE_AUTO OR NOT CONFIG_ZMK_USB)
    message(FATAL_ERROR "USB priority requires host identity, BLE auto and native keyboard USB")
  endif()
  if(CONFIG_ZMK_POINTING)
    zephyr_link_libraries(-Wl,--wrap=zmk_endpoint_send_mouse_report)
  endif()
  # Le pilote conserve son statut physique natif. GET_REPORT et les envois reçoivent les modèles protégés.
  set_property(SOURCE ${APPLICATION_SOURCE_DIR}/src/usb_hid.c DIRECTORY ${APPLICATION_SOURCE_DIR}
    APPEND PROPERTY COMPILE_DEFINITIONS
      zmk_hid_get_keyboard_report=splitergoll_usb_keyboard_report
      zmk_hid_get_consumer_report=splitergoll_usb_consumer_report)
  if(CONFIG_ZMK_POINTING)
    set_property(SOURCE ${APPLICATION_SOURCE_DIR}/src/usb_hid.c DIRECTORY ${APPLICATION_SOURCE_DIR}
      APPEND PROPERTY COMPILE_DEFINITIONS zmk_hid_get_mouse_report=splitergoll_usb_mouse_report)
  endif()
  if(CONFIG_ZMK_USB_BOOT)
    set_property(SOURCE ${APPLICATION_SOURCE_DIR}/src/usb_hid.c DIRECTORY ${APPLICATION_SOURCE_DIR}
      APPEND PROPERTY COMPILE_DEFINITIONS zmk_hid_get_boot_report=splitergoll_usb_boot_report)
  endif()
  zephyr_library_sources(src/connections/usb_priority_state.c src/connections/usb_priority.c)
  set_source_files_properties(src/connections/usb_priority_state.c src/connections/usb_priority.c
    PROPERTIES COMPILE_OPTIONS "-Werror")
  zephyr_link_libraries(-Wl,--wrap=zmk_usb_is_hid_ready -Wl,--wrap=zmk_endpoint_send_report
    -Wl,--wrap=zmk_usb_hid_send_keyboard_report)
endif()
if(CONFIG_SPLITERGOLL_BLE_MEMORY)
  zephyr_library_sources(src/os_detection/ble_os_memory.c src/os_detection/ble_os_settings.c
                         src/os_detection/ble_acquisition.c)
  zephyr_library_link_libraries(mbedTLS)
endif()
if(CONFIG_SPLITERGOLL_BLE_OBSERVER)
  if(CONFIG_LTO OR CONFIG_BT_EATT OR NOT CONFIG_SOC_NRF52840)
    message(FATAL_ERROR "BLE observation requires the pinned nRF52840 stack without LTO or EATT")
  endif()
  zephyr_library_sources(src/os_detection/ble_observer.c src/os_detection/ble_os_classifier.c
                         src/os_detection/ble_att.c)
  zephyr_link_libraries(-Wl,--wrap=bt_l2cap_recv)
endif()
if(CONFIG_SPLITERGOLL_USB_OBSERVER)
  if(CONFIG_USB_DEVICE_STACK_NEXT OR CONFIG_LTO OR NOT CONFIG_SOC_NRF52840)
    message(FATAL_ERROR "USB observation requires the legacy nRF52840 USB stack without LTO")
  endif()
  zephyr_library_sources(src/os_detection/usb_observer.c src/os_detection/usb_os_classifier.c)
  zephyr_link_libraries(
    -Wl,--wrap=usb_dc_ep_set_callback
    -Wl,--wrap=usb_dc_ep_read
    -Wl,--wrap=usb_dc_set_status_callback
  )
endif()
if(CONFIG_SPLITERGOLL_OS_AUTO)
  zephyr_library_sources(src/os_detection/usb_auto_state.c)
  zephyr_link_libraries(-Wl,--wrap=zmk_event_manager_raise -Wl,--wrap=zmk_keymap_layer_to)
  if(CONFIG_SPLITERGOLL_BLE_AUTO)
    if(NOT CONFIG_SPLITERGOLL_BLE_RECONNECT)
      message(FATAL_ERROR "BLE auto requires the legacy known-host reconnect controller")
    endif()
    zephyr_library_sources(src/os_detection/ble_auto_state.c src/os_detection/ble_auto.c
                           src/os_detection/ble_device_state.c src/os_detection/ble_device_behavior.c)
  else()
    zephyr_library_sources(src/os_detection/usb_auto.c)
  endif()
endif()
if(CONFIG_SPLITERGOLL_BLE_RECONNECT)
  zephyr_library_sources(src/connections/host_window_policy.c
                         src/connections/host_preference.c src/connections/host_preference_settings.c
                         src/connections/ble_reconnect_state.c src/connections/ble_pairing_state.c
                         src/connections/ble_reconnect.c)
  zephyr_link_libraries(-Wl,--wrap=bt_le_adv_start -Wl,--wrap=bt_le_adv_stop
                       -Wl,--wrap=bt_conn_auth_cb_register -Wl,--wrap=bt_conn_auth_info_cb_register)
endif()

if(CONFIG_SPLITERGOLL_WIRE_USB_ONLY)
  if(CONFIG_SPLITERGOLL_BLE_RECONNECT OR CONFIG_BT_EXT_ADV OR CONFIG_LTO)
    message(FATAL_ERROR "Wire USB-only requires legacy advertising without BLE reconnect or LTO")
  endif()
  zephyr_library_sources(src/connections/wire_usb_only.c)
  zephyr_link_libraries(-Wl,--wrap=bt_le_adv_start -Wl,--wrap=bt_le_adv_stop)
endif()
