kdeconnect_add_plugin(kdeconnect_shutdowntimer SOURCES shutdowntimer.cpp shutdowntimerplugin.cpp)

if(WIN32)
    target_sources(kdeconnect_shutdowntimer PRIVATE powermanagement_win.cpp)
    target_link_libraries(kdeconnect_shutdowntimer PowrProf)
elseif(APPLE)
    target_sources(kdeconnect_shutdowntimer PRIVATE powermanagement_mac.cpp)
else()
    target_sources(kdeconnect_shutdowntimer PRIVATE powermanagement_linux.cpp)
    target_link_libraries(kdeconnect_shutdowntimer Qt::DBus)
endif()

target_link_libraries(kdeconnect_shutdowntimer
    kdeconnectcore
    KF6::CoreAddons
    KF6::I18n
)

install(FILES "kdeconnect_shutdowntimer_config.qml" DESTINATION ${KDE_INSTALL_DATADIR}/kdeconnect)
