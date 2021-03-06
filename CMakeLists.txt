project(nepomuk-widgets)
# set_package_properties appeared in cmake 2.8.6
# TODO: Remove when kdelibs >= 4.10 will be required
cmake_minimum_required(VERSION 2.8.6 FATAL_ERROR)

set(CMAKE_NEPOMUK_WIDGETS_VERSION_MAJOR 0)
set(CMAKE_NEPOMUK_WIDGETS_VERSION_MINOR 1)
set(CMAKE_NEPOMUK_WIDGETS_VERSION_RELEASE 0)
set(CMAKE_NEPOMUK_WIDGETS_VERSION_STRING "${CMAKE_NEPOMUK_WIDGETS_VERSION_MAJOR}.${CMAKE_NEPOMUK_WIDGETS_VERSION_MINOR}.${CMAKE_NEPOMUK_WIDGETS_VERSION_RELEASE}")

# Search for the required packages
# ================================
find_package(KDE4 4.9.4 REQUIRED)
include(KDE4Defaults)

find_package(Soprano)
set_package_properties(Soprano PROPERTIES DESCRIPTION "Qt-based RDF storage and parsing solution" URL "http://soprano.sourceforge.net" TYPE REQUIRED)

find_package(NepomukCore)
set_package_properties(NepomukCore PROPERTIES DESCRIPTION "The core Nepomuk libraries" URL "http://nepomuk.kde.org" TYPE REQUIRED PURPOSE "Required for running Nepomuk")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

# Some definitions
# ================
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_CORE_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/ui
  ${CMAKE_SOURCE_DIR}/utils
)


# generate NepomukWidgetsConfig.cmake for easy utilisation of the package by other cmake build systems
# ===============================================================================================
configure_file(NepomukWidgetsConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/NepomukWidgetsConfig.cmake @ONLY)

# Generate NepomukWidgetsConfigVersion.cmake
macro_write_basic_cmake_version_file(${CMAKE_CURRENT_BINARY_DIR}/NepomukWidgetsConfigVersion.cmake
                                     ${CMAKE_NEPOMUK_WIDGETS_VERSION_MAJOR}
                                     ${CMAKE_NEPOMUK_WIDGETS_VERSION_MINOR}
                                     ${CMAKE_NEPOMUK_WIDGETS_VERSION_RELEASE}
                                     )
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/NepomukWidgetsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/NepomukWidgetsConfigVersion.cmake
    DESTINATION ${LIB_INSTALL_DIR}/cmake/NepomukWidgets)

set(nepomuk_ui_SRCS
  ui/kblocklayout.cpp
  ui/tagwidget.cpp
  ui/kedittagsdialog.cpp
  ui/tagcheckbox.cpp
  ui/filemetadatawidget.cpp
  ui/filemetadataconfigwidget.cpp
  ui/filemetadataprovider.cpp
  ui/resourceloader.cpp
  ui/indexeddataretriever.cpp
  ui/kcommentwidget.cpp
  ui/knfotranslator.cpp
  ui/metadatafilter.cpp
  ui/widgetfactory.cpp
)

set(nepomukutils_SRCS
  utils/resourcemodel.cpp
)

set(nepomukwidgets_SRCS ${nepomuk_ui_SRCS} ${nepomukutils_SRCS})

kde4_add_library(nepomukwidgets SHARED ${LIBRARY_TYPE} ${nepomukwidgets_SRCS})

target_link_libraries(nepomukwidgets
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${SOPRANO_LIBRARIES}
  ${NEPOMUK_CORE_LIBRARY}
)

target_link_libraries(nepomukwidgets
  LINK_INTERFACE_LIBRARIES
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${SOPRANO_LIBRARIES}
)

set_target_properties(nepomukwidgets PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS nepomukwidgets EXPORT NepomukWidgetsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  nepomukwidgets_export.h
  ui/tagwidget.h
  ui/filemetadatawidget.h
  ui/filemetadataconfigwidget.h
  utils/resourcemodel.h

  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk2 COMPONENT Devel
)

add_subdirectory(test)

# install the file with the exported targets
install(EXPORT NepomukWidgetsLibraryTargets
        DESTINATION ${LIB_INSTALL_DIR}/cmake/NepomukWidgets
        FILE NepomukWidgetsTargetsWithPrefix.cmake
        )
