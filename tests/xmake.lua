add_deps("cutestl")

target("test_vector")
    set_kind("binary")
    add_files("test_vector.cpp")

target("test_list")
    set_kind("binary")
    add_files("test_list.cpp")