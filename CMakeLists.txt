cmake_minimum_required( VERSION 3.6 )
project( speedosk )

add_definitions( -Wall )

configure_file( Config.h.in Config.h )

find_package( SDL2 REQUIRED )
find_package( X11 REQUIRED )

include_directories( ${SDL2_INCLUDE_DIRS} ${X11_INCLUDE_DIR} ${X11_Xtst_INCLUDE_DIR} ${CMAKE_BINARY_DIR} )

add_executable( speedosk main.cpp SpeedOSK.cpp SpeedOSKBoard.cpp )
target_link_libraries( speedosk ${SDL2_LIBRARIES} -lSDL2_ttf  ${X11_LIBRARIES} -lXtst )

install( TARGETS speedosk DESTINATION bin/ )
install( FILES LiberationSans-Bold.ttf DESTINATION share/speedosk/ )
install( FILES keymap_us.conf DESTINATION share/speedosk/ )
install( FILES de.speedprog.speedosk.desktop DESTINATION share/applications/ )
install( FILES de.speedprog.speedosk.svg DESTINATION share/icons/hicolor/scalable/apps/ )
install( FILES LiberationSans-LICENSE DESTINATION share/speedosk/ )
