include_directories(${PROJECT_SOURCE_DIR} ${HUSKY_EXTERNAL_INCLUDE})

file(GLOB rr-src-files
  rapid_reassignment.cpp
  sender_rr.cpp
  job_manager_rr.cpp
  #sender_rr_test.cpp
  #worker_group_rr_test.cpp
)

add_library(rr-objs OBJECT ${rr-src-files})
set_property(TARGET rr-objs PROPERTY CXX_STANDARD 11)
add_dependencies(rr-objs ${external_project_dependencies})
