
set(SRC_LIST
	${CMAKE_CURRENT_LIST_DIR}/main.cpp

	${CMAKE_CURRENT_LIST_DIR}/HashGenerator/IHashGenerator.h
	${CMAKE_CURRENT_LIST_DIR}/HashGenerator/HashGeneratorBoostImpl.h
	${CMAKE_CURRENT_LIST_DIR}/HashGenerator/HashGeneratorBoostImpl.cpp

	${CMAKE_CURRENT_LIST_DIR}/Parser/CommandLineParser.h
	${CMAKE_CURRENT_LIST_DIR}/Parser/CommandLineParser.cpp

	${CMAKE_CURRENT_LIST_DIR}/Reader/BlockStreamReader.h
	${CMAKE_CURRENT_LIST_DIR}/Reader/BlockStreamReader.cpp

	${CMAKE_CURRENT_LIST_DIR}/ThreadPool/SimpleThreadPool.h
	${CMAKE_CURRENT_LIST_DIR}/ThreadPool/SimpleThreadPool.cpp

	${CMAKE_CURRENT_LIST_DIR}/Writer/AsyncHashStreamWriter.h
	${CMAKE_CURRENT_LIST_DIR}/Writer/AsyncHashStreamWriter.cpp
	)

add_executable(Project ${SRC_LIST})

find_package(Boost REQUIRED COMPONENTS program_options)

target_link_libraries(Project PRIVATE Boost::program_options)

target_include_directories(Project PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(Project PRIVATE ${CMAKE_CURRENT_LIST_DIR}/..)
