# Copyright (C) 2012-2026  CEA, EDF, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

INCLUDE(UseQtExt)
INCLUDE_DIRECTORIES(
  ${QT_INCLUDES}
  ${QWT_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/src/ExtensionManager
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets Network REQUIRED)

set(SOURCES
    ExtensionManagerDock.cpp
    ExtensionModel.cpp
    ExtensionDelegate.cpp
    ExtensionFetcher.cpp
    MockHttpServer.cpp
)

set(HEADERS
    ExtensionManagerDock.h
    ExtensionModel.h
    ExtensionDelegate.h
    ExtensionFetcher.h
    MockHttpServer.h
    Extension.h
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${QT_DEFINITIONS}
  ${QWT_DEFINITIONS}
)

SET(_link_LIBRARIES
  ${QT_LIBRARIES}
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
)
  
add_library(SalomeExtensionManager ${SOURCES})
target_link_libraries(SalomeExtensionManager ${_link_LIBRARIES})
INSTALL(TARGETS SalomeExtensionManager  EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

