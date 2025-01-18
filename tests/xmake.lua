
add_deps("cutestl")

-- target("test_function")
--     set_kind("binary")
--     add_files("test_function.cpp")

-- target("test_mtx_queue")
--     set_kind("binary")
--     add_files("test_mtx_queue.cpp")

-- target("test_thread_pool")
--     set_kind("binary")
--     add_files("test_thread_pool.cpp")

target("test_vector")
    set_kind("binary")
    add_files("test_vector.cpp")