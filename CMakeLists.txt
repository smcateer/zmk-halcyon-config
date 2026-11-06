# Add the Kyria battery-typing behaviour to the dongle build only.
if(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_KYRIA_BATTERY app PRIVATE src/behavior_kyria_battery.c)
endif()
