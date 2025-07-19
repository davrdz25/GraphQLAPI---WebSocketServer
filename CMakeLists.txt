cmake_minimum_required(VERSION 3.16)
project(GraphQLServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Detectar plataforma
if(WIN32)
    message(STATUS "Platform: Windows")
elseif(APPLE)
    message(STATUS "Platform: macOS")
elseif(UNIX)
    message(STATUS "Platform: Linux")
endif()

# Buscar librería ODBC
if(WIN32)
    find_library(ODBC_LIBRARY odbc32)
elseif(APPLE OR UNIX)
    find_library(ODBC_LIBRARY odbc)
endif()

if(NOT ODBC_LIBRARY)
    message(FATAL_ERROR "ODBC library not found")
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules(GRAPHQLPARSER REQUIRED libgraphqlparser)

find_package(Boost REQUIRED COMPONENTS system thread)

# Buscar Boost.JSON - puede no ser detectado por find_package, así que indicamos ruta manual
find_library(BOOST_JSON_LIB NAMES boost_json PATHS /usr/local/lib /usr/lib NO_DEFAULT_PATH)

if(NOT BOOST_JSON_LIB)
    message(FATAL_ERROR "Boost.JSON library not found")
endif()

find_library(SODIUM_LIB sodium REQUIRED)
if(NOT SODIUM_LIB)
    message(FATAL_ERROR "libsodium no encontrada")
endif()

find_package(Threads REQUIRED)

# Buscar librerías graphql
find_library(GRAPHQLRESPONSE_LIB NAMES graphqlresponse PATHS /usr/local/lib /usr/lib)
find_library(GRAPHQLPEG_LIB NAMES graphqlpeg PATHS /usr/local/lib /usr/lib)
find_library(GRAPHQLSERVICE_LIB NAMES graphqlservice PATHS /usr/local/lib /usr/lib)
find_library(GRAPHQLCLIENT_LIB NAMES graphqlclient PATHS /usr/local/lib /usr/lib)
find_library(GRAPHQLJSON_LIB NAMES graphqljson PATHS /usr/local/lib /usr/lib)

if(NOT GRAPHQLSERVICE_LIB OR NOT GRAPHQLRESPONSE_LIB OR NOT GRAPHQLPEG_LIB)
    message(FATAL_ERROR "No se encontraron todas las librerías de graphql necesarias")
endif()

set(GRAPHQL_GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/GraphQL/Generated)
set(GRAPHQL_RESOLVERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/GraphQL/Resolvers)


set(GRAPHQL_GENERATED_SOURCES
    ${GRAPHQL_GENERATED_DIR}/User/UserObject.cpp
    ${GRAPHQL_GENERATED_DIR}/User/AuthPayloadObject.cpp
    ${GRAPHQL_GENERATED_DIR}/User/UserPublicObject.cpp
    ${GRAPHQL_GENERATED_DIR}/User/UserSchema.cpp
    ${GRAPHQL_GENERATED_DIR}/User/QueryObject.cpp
    ${GRAPHQL_GENERATED_DIR}/User/MutationObject.cpp
    ${GRAPHQL_GENERATED_DIR}/User/UserQueryObject.cpp
    ${GRAPHQL_GENERATED_DIR}/User/UserMutationObject.cpp
)

set(GRAPHQL_CUSTOM_RESOLVERS
    ${GRAPHQL_RESOLVERS_DIR}/UserResolvers/UserResolver.hpp
    ${GRAPHQL_RESOLVERS_DIR}/UserResolvers/UserQueryResolver.hpp
    ${GRAPHQL_RESOLVERS_DIR}/UserResolvers/UserMutationResolver.hpp
)

file(GLOB_RECURSE PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Database/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Interfaces/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Repositories/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Services/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

list(APPEND PROJECT_SOURCES ${GRAPHQL_CUSTOM_RESOLVERS}
)

add_executable(GraphQLServer
    ${GRAPHQL_GENERATED_SOURCES}
    ${PROJECT_SOURCES}
)

target_include_directories(GraphQLServer PRIVATE
    ${GRAPHQLPARSER_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/Database
    ${CMAKE_CURRENT_SOURCE_DIR}/DTOs
    ${CMAKE_CURRENT_SOURCE_DIR}/Interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/Repositories
    ${CMAKE_CURRENT_SOURCE_DIR}/Services
    ${CMAKE_CURRENT_SOURCE_DIR}/Models
    ${GRAPHQL_GENERATED_DIR}
)

target_link_libraries(GraphQLServer PRIVATE
    ${ODBC_LIBRARY}
    ${GRAPHQLPARSER_LIBRARIES}
    ${Boost_LIBRARIES}
    Threads::Threads
    ${BOOST_JSON_LIB}
    ${SODIUM_LIB}
    ${GRAPHQLJSON_LIB}  
    ${GRAPHQLPEG_LIB}          
    ${GRAPHQLRESPONSE_LIB}      
    ${GRAPHQLSERVICE_LIB}       
    # Si necesitas el cliente
    # ${GRAPHQLCLIENT_LIB}
)

if(APPLE)
    target_link_libraries(GraphQLServer PRIVATE iconv)
    set_target_properties(GraphQLServer PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "/usr/local/lib"
    )
endif()