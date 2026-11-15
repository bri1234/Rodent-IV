cmake_minimum_required(VERSION 3.24)
project(RodentIV VERSION 0.33 LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Use an out-of-source build: cmake -S . -B build/release")
endif()

include(GNUInstallDirs)
include(CTest)
option(RODENT_NATIVE "Optimize for the build machine's CPU (not portable)" OFF)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(rodentIV
    src/attacks.cpp
    src/bitboard.cpp
    src/book.cpp
    src/data.cpp
    src/edata.cpp
    src/eval.cpp
    src/eval_draw.cpp
    src/eval_patterns.cpp
    src/eval_pawn.cpp
    src/gen.cpp
    src/init.cpp
    src/legal.cpp
    src/magicmoves.cpp
    src/main.cpp
    src/mask.cpp
    src/movedo.cpp
    src/moveundo.cpp
    src/next.cpp
    src/params.cpp
    src/quiesce.cpp
    src/recognize.cpp
    src/rodenthome.cpp
    src/search.cpp
    src/setboard.cpp
    src/stringfunctions.cpp
    src/swap.cpp
    src/taunt.cpp
    src/trans.cpp
    src/tuning.cpp
    src/uci.cpp
    src/uci_options.cpp
    src/util.cpp
)
set_target_properties(rodentIV PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
target_link_libraries(rodentIV PRIVATE Threads::Threads)

# Find data even when the installation prefix is moved or overridden at install time.
file(RELATIVE_PATH rodent_data_from_bin
    "${CMAKE_INSTALL_FULL_BINDIR}" "${CMAKE_INSTALL_FULL_DATADIR}/rodentIV")
target_compile_definitions(rodentIV PRIVATE
    "$<$<CONFIG:Debug>:DEBUG>"
    "$<$<PLATFORM_ID:Android>:ANDROID>"
    "RODENT_INSTALL_DATA_SUBDIR=\"${rodent_data_from_bin}\""
    "RODENT_INSTALL_DATADIR=\"${CMAKE_INSTALL_FULL_DATADIR}/rodentIV\""
)

if(MSVC)
    target_compile_options(rodentIV PRIVATE /W4)
    target_compile_definitions(rodentIV PRIVATE _CRT_SECURE_NO_WARNINGS NO_MM_POPCNT)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(rodentIV PRIVATE -Wall -Wextra -Wno-unknown-pragmas)
endif()

if(RODENT_NATIVE)
    if(CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "RODENT_NATIVE cannot be used when cross-compiling")
    elseif(MSVC)
        message(FATAL_ERROR "RODENT_NATIVE requires GCC or Clang; set MSVC architecture flags explicitly")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(rodentIV PRIVATE -march=native)
    else()
        message(FATAL_ERROR "RODENT_NATIVE is not supported by this compiler")
    endif()
endif()

# Refresh data even if only a personality or book changed, without relinking.
add_custom_target(rodent_data
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${PROJECT_SOURCE_DIR}/books" "$<TARGET_FILE_DIR:rodentIV>/books"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${PROJECT_SOURCE_DIR}/personalities" "$<TARGET_FILE_DIR:rodentIV>/personalities"
    COMMENT "Updating opening books and personalities"
    VERBATIM
)
add_dependencies(rodentIV rodent_data)

install(TARGETS rodentIV RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(DIRECTORY books personalities DESTINATION "${CMAKE_INSTALL_DATADIR}/rodentIV")
install(FILES README.md LICENSE docs/FinalRodent.pdf docs/personality-editor.html
    DESTINATION "${CMAKE_INSTALL_DOCDIR}")

if(BUILD_TESTING)
    add_test(NAME smoke
        COMMAND "${CMAKE_COMMAND}"
            "-DENGINE=$<TARGET_FILE:rodentIV>"
            "-DDATA_DIR=${PROJECT_SOURCE_DIR}"
            "-DTEST_DIR=${PROJECT_BINARY_DIR}/tests/$<CONFIG>"
            -P "${PROJECT_SOURCE_DIR}/tests/smoke.cmake")
    set_tests_properties(smoke PROPERTIES TIMEOUT 60)
endif()
