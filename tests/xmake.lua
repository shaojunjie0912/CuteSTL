add_deps("cutestl")

-- target("test_vector", function()
--     set_kind("binary")
--     add_files("test_vector.cpp")
-- end)

-- target("test_list", function()
--     set_kind("binary")
--     add_files("test_list.cpp")
-- end)

-- target("test_simple_shared_ptr", function()
--     set_kind("binary")
--     add_files("test_simple_shared_ptr.cpp")
-- end)

target("test_thread_safe_shared_ptr", function()
    set_kind("binary")
    add_files("test_thread_safe_shared_ptr.cpp")
end)
