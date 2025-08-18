add_deps("cutestl")

target("test_list", function()
    set_kind("binary")
    add_files("test_list.cpp")
end)

target("test_string", function()
    set_kind("binary")
    add_files("test_string.cpp")
end)
