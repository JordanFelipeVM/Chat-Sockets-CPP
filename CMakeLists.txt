cmake_minimum_required(VERSION 3.10)
project(ChatSalaComputo)

set(CMAKE_CXX_STANDARD 17)

# Buscar SQLite
find_package(SQLite3 REQUIRED)

# Servidor
add_executable(server
    server/server.cpp
    database/database.cpp
)

# Cliente
add_executable(client
    client/client.cpp
)

# Linkear librerías
target_link_libraries(server PRIVATE ${SQLite3_LIBRARIES} ws2_32)
target_link_libraries(client PRIVATE ws2_32)

# Forzar consola en Windows
if(WIN32)
    set_target_properties(server PROPERTIES LINK_FLAGS "-mconsole")
    set_target_properties(client PROPERTIES LINK_FLAGS "-mconsole")
endif()