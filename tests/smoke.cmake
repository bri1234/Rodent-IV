# Exercise the public protocol from outside the engine/data directories.
file(MAKE_DIRECTORY "${TEST_DIR}")
set(commands "uci\nisready\nbench 8\nsetoption name Verbose value true\n")
foreach(personality IN ITEMS Deborah Spassky Strangler)
    string(APPEND commands "setoption name Personality value ${personality}\n")
endforeach()
string(APPEND commands "isready\nquit\n")
file(WRITE "${TEST_DIR}/input.txt" "${commands}")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
        --unset=RODENT4HOME --unset=RODENT4BOOKS --unset=RODENT4PERSONALITIES
        "${ENGINE}"
    INPUT_FILE "${TEST_DIR}/input.txt"
    WORKING_DIRECTORY "${TEST_DIR}"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE errors
    RESULT_VARIABLE result
    TIMEOUT 50
)
file(WRITE "${TEST_DIR}/output.txt" "${output}${errors}")
if(NOT result STREQUAL "0")
    message(FATAL_ERROR "Engine failed: ${result}\n${output}\n${errors}")
endif()
foreach(expected IN ITEMS "uciok" "readyok" "557513 nodes searched"
        "reading personality 'deborah.txt' (success)"
        "reading personality 'spassky.txt' (success)"
        "reading personality 'strangler.txt' (success)"
        "reading book file 'guide.bin' (success/"
        "reading book file 'players/ph-spassky2.bin' (success/"
        "reading book file 'guide/strangler.bin' (success/")
    string(FIND "${output}" "${expected}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Missing '${expected}' in engine output:\n${output}")
    endif()
endforeach()
if(output MATCHES "no 'basic.ini'|\\(failure\\)")
    message(FATAL_ERROR "Runtime data could not be loaded:\n${output}")
endif()

# Validate all shipped references, including personalities outside the default set.
file(STRINGS "${DATA_DIR}/personalities/basic.ini" aliases REGEX "^[^#;].*=.*\\.txt$")
foreach(alias IN LISTS aliases)
    string(REGEX REPLACE "^[^=]*=" "" filename "${alias}")
    if(NOT EXISTS "${DATA_DIR}/personalities/${filename}")
        message(FATAL_ERROR "Missing personality: ${filename}")
    endif()
endforeach()
file(GLOB personalities "${DATA_DIR}/personalities/*.txt")
foreach(personality IN LISTS personalities)
    file(STRINGS "${personality}" books REGEX "^setoption name (GuideBookFile|MainBookFile) value ")
    foreach(book IN LISTS books)
        string(REGEX REPLACE "^setoption name [^ ]+ value " "" filename "${book}")
        if(NOT EXISTS "${DATA_DIR}/books/${filename}")
            message(FATAL_ERROR "Missing book in ${personality}: ${filename}")
        endif()
    endforeach()
endforeach()
