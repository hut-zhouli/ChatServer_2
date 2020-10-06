#定义一个SRC_LIST变量，包含该目录下的所有的源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./redis REDIS_LIST)
aux_source_directory(./model MODEL_LIST)
#生产可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})

#指定可执行文件链接是需要依赖的lib包
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient pthread hiredis)