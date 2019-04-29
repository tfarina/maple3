set(ECHOD_SRCS
  ed_cmdline.c
  ed_config.c
  ed_daemon.c
  ed_instance.c
  ed_log.c
  ed_main.c
  ed_net.c
  ed_pid.c
  ed_printf.c
  ed_utils.c
)

add_executable(echod ${ECHOD_SRCS})
set_target_properties(echod PROPERTIES LINKER_LANGUAGE C)

set(INSTALL_SBIN_DIR "${CMAKE_INSTALL_PREFIX}/sbin" CACHE PATH "Installation directory for system admin executables")

install(TARGETS echod RUNTIME DESTINATION "${INSTALL_SBIN_DIR}")