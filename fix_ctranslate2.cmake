if(WIN32)
    set(ProgramFilesx86 "ProgramFiles(x86)")
    include_directories($ENV{${ProgramFilesx86}}/oneDNN/include)
    message(STATUS "INCLUDED DIRECTORIES HERE: ${INCLUDE_DIRECTORIES}")
endif()