find_package(Qt6 REQUIRED COMPONENTS Test)

set(SHARED_FLOW_SOURCES
)

add_executable(dji_tests
    ${SHARED_FLOW_SOURCES}
    tst_main.cpp
    tst_crc.cpp
    tst_message.cpp
    tst_connect_flow.cpp
)

target_link_libraries(dji_tests PRIVATE
    dji
    Qt6::Test
)

qt_add_executable(dji_demo
    ${SHARED_FLOW_SOURCES}
    dji_demo.cpp
)

target_link_libraries(dji_demo PRIVATE
    dji
    Qt6::Core
    Qt6::Bluetooth
    Qt6::Network
)

add_test(NAME dji_tests COMMAND dji_tests)
