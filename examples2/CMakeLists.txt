include_directories(${PROJECT_SOURCE_DIR} ${HUSKY_EXTERNAL_INCLUDE})

# Data Distributor
add_executable(DataDist data_dist.cpp)
target_link_libraries(DataDist flexps)
target_link_libraries(DataDist ${HUSKY_EXTERNAL_LIB})
set_property(TARGET DataDist PROPERTY CXX_STANDARD 11)
add_dependencies(DataDist ${external_project_dependencies})

# BasicExample2
add_executable(BasicExample2 basic_example2.cpp)
target_link_libraries(BasicExample2 flexps)
target_link_libraries(BasicExample2 ${HUSKY_EXTERNAL_LIB})
set_property(TARGET BasicExample2 PROPERTY CXX_STANDARD 11)
add_dependencies(BasicExample2 ${external_project_dependencies})

# BasicExample
add_executable(BasicExample basic_example.cpp)
target_link_libraries(BasicExample flexps)
target_link_libraries(BasicExample ${HUSKY_EXTERNAL_LIB})
set_property(TARGET BasicExample PROPERTY CXX_STANDARD 11)
add_dependencies(BasicExample ${external_project_dependencies})

# SparseSSPExample
#add_executable(SparseSSPExample sparse_ssp_example.cpp)
#target_link_libraries(SparseSSPExample flexps)
#target_link_libraries(SparseSSPExample profiler)  # compile with profiler
#target_link_libraries(SparseSSPExample ${HUSKY_EXTERNAL_LIB})
#set_property(TARGET SparseSSPExample PROPERTY CXX_STANDARD 11)
#add_dependencies(SparseSSPExample ${external_project_dependencies})

# KVStorePerformanceExample
add_executable(KVStorePerformanceExample kvstore_performance_example.cpp)
target_link_libraries(KVStorePerformanceExample flexps)
target_link_libraries(KVStorePerformanceExample ${HUSKY_EXTERNAL_LIB})
set_property(TARGET KVStorePerformanceExample PROPERTY CXX_STANDARD 11)
add_dependencies(KVStorePerformanceExample ${external_project_dependencies})

# ChannelExample
add_executable(ChannelExample channel_example.cpp)
target_link_libraries(ChannelExample flexps)
target_link_libraries(ChannelExample ${HUSKY_EXTERNAL_LIB})
set_property(TARGET ChannelExample PROPERTY CXX_STANDARD 11)
add_dependencies(ChannelExample ${external_project_dependencies})

# HdfsReadExample
#if(LIBHDFS3_FOUND)
add_executable(HdfsReadExample hdfs_read_example.cpp)
target_link_libraries(HdfsReadExample flexps)
target_link_libraries(HdfsReadExample ${HUSKY_EXTERNAL_LIB})
set_property(TARGET HdfsReadExample PROPERTY CXX_STANDARD 11)
add_dependencies(HdfsReadExample ${external_project_dependencies})
#endif(LIBHDFS3_FOUND)


#if(LIBHDFS3_FOUND)
add_executable(HdfsManagerExample hdfs_manager_example.cpp)
target_link_libraries(HdfsManagerExample flexps)
target_link_libraries(HdfsManagerExample ${HUSKY_EXTERNAL_LIB})
set_property(TARGET HdfsManagerExample PROPERTY CXX_STANDARD 11)
add_dependencies(HdfsManagerExample ${external_project_dependencies})
#endif(LIBHDFS3_FOUND)


# LRExample
#if(LIBHDFS3_FOUND)
add_executable(LRExample logistic_regression.cpp)
target_link_libraries(LRExample flexps)
target_link_libraries(LRExample ${HUSKY_EXTERNAL_LIB})
set_property(TARGET LRExample PROPERTY CXX_STANDARD 11)
add_dependencies(LRExample ${external_project_dependencies})
#endif(LIBHDFS3_FOUND)

# GBDTExample
add_subdirectory(gbdt)

# KMeansExample
add_subdirectory(kmeans)

# Rapid Reassignment
add_subdirectory(rapid_reassignment)

# SGD
#add_subdirectory(sgd)

#LR Example
add_executable(LRTest lrtest.cpp)
target_link_libraries(LRTest flexps)
target_link_libraries(LRTest ${HUSKY_EXTERNAL_LIB})
set_property(TARGET LRTest PROPERTY CXX_STANDARD 11)
add_dependencies(LRTest ${external_project_dependencies})
