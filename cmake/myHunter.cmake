
if (USE_HUNTER)
    include("cmake/HunterGate.cmake")
    HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.19.52.tar.gz"
        SHA1 "6d3a8135ba62726d810fd8c8c7d97980fa6c3b4a"
    )
endif()


# adds package if not found on the system
function(myhunter_add_package arg)
    if (USE_HUNTER)
        find_package( Protobuf QUIET )
        if (NOT PROTOBUF_FOUND )
            hunter_add_package(Protobuf)
        endif()
    endif()

endfunction()