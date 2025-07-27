#
# read-csv
#
find_package(Threads REQUIRED)
set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost COMPONENTS program_options REQUIRED)

add_executable(tof-read-csv tof-read-csv.cpp)

target_link_libraries(
    tof-read-csv
    tofcore 
    Threads::Threads 
    Boost::program_options
    dbg_out
    po_count
)

if (WIN32)
    set_target_properties(tof-read-csv PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()
