cmake_minimum_required(VERSION 3.16)

project(IdlersMapEditor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Xml  # Added Xml
)

set(PROJECT_SOURCES
    src/additemcommand.cpp
    src/additemcommand.h
    src/binaryfile.cpp
    src/binaryfile.h
    src/bordersystem.cpp
    src/bordersystem.h
    src/bordersystemdialog.cpp
    src/bordersystemdialog.h
    src/brush.cpp
    src/brush.h
    src/brushmanager.cpp
    src/brushmanager.h
    src/clearitemscommand.cpp
    src/clearitemscommand.h
    src/clientversion.cpp
    src/clientversion.h
    src/creature.cpp
    src/creature.h
    src/creaturemanager.cpp
    src/creaturemanager.h
    src/creaturepropertyeditor.cpp
    src/creaturepropertyeditor.h
    src/deleteselectioncommand.cpp
    src/deleteselectioncommand.h
    src/eraserbrush.cpp
    src/eraserbrush.h
    src/floodfillbrush.cpp
    src/floodfillbrush.h
    src/gotopositiondialog.cpp
    src/gotopositiondialog.h
    src/helpsystem.cpp
    src/helpsystem.h
    src/item.cpp
    src/item.h
    src/itemmanager.cpp
    src/itemmanager.h
    src/itempropertyeditor.cpp
    src/itempropertyeditor.h
    src/layer.cpp
    src/layer.h
    src/layerwidget.cpp
    src/layerwidget.h
    src/main.cpp
    src/mainmenu.cpp
    src/mainmenu.h
    src/maintoolbar.cpp
    src/maintoolbar.h
    src/mainwindow.cpp
    src/mainwindow.h
    src/map.cpp
    src/map.h
    src/mapcommand.cpp
    src/mapcommand.h
    src/mapeditorpalette.cpp
    src/mapeditorpalette.h
    src/mapitembrush.cpp
    src/mapitembrush.h
    src/mapmouseevent.cpp
    src/mapmouseevent.h
    src/mappropertiesdialog.cpp
    src/mappropertiesdialog.h
    src/mapscene.cpp
    src/mapscene.h
    src/mapstatisticsdialog.cpp
    src/mapstatisticsdialog.h
    src/mapview.cpp
    src/mapview.h
    src/mapversion.cpp
    src/mapversion.h
    src/newmapdialog.cpp
    src/newmapdialog.h
    src/normalbrush.cpp
    src/normalbrush.h
    src/otbmfile.cpp
    src/otbmfile.h
    src/outputdialog.cpp
    src/outputdialog.h
    src/pasteselectioncommand.cpp
    src/pasteselectioncommand.h
    src/pencilbrush.cpp
    src/pencilbrush.h
    src/player.cpp
    src/player.h
    src/position.cpp
    src/position.h
    src/preferencesdialog.cpp
    src/preferencesdialog.h
    src/progressdialog.cpp
    src/progressdialog.h
    src/propertyeditor.cpp
    src/propertyeditor.h
    src/propertyeditordock.cpp
    src/propertyeditordock.h
    src/resourcemanager.cpp
    src/resourcemanager.h
    src/selectionbrush.cpp
    src/selectionbrush.h
    src/selectiontoolbar.cpp
    src/selectiontoolbar.h
    src/settings.cpp
    src/settings.h
    src/settingsmanager.cpp
    src/settingsmanager.h
    src/spawn.cpp
    src/spawn.h
    src/spritemanager.cpp
    src/spritemanager.h
    src/tibiafilehandler.cpp
    src/tibiafilehandler.h
    src/tile.cpp
    src/tile.h
    src/tilepropertyeditor.cpp
    src/tilepropertyeditor.h
    src/toolspanel.cpp
    src/toolspanel.h
    src/undostack.cpp
    src/undostack.h
    src/waypoint.cpp
    src/waypoint.h
    src/xmlfile.cpp
    src/xmlfile.h
    resources.qrc
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Xml  # Added Qt6::Xml
)

# Kopiowanie zasobów do katalogu build
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/images DESTINATION ${CMAKE_BINARY_DIR}) 