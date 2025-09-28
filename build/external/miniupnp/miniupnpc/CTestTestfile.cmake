# CMake generated Testfile for 
# Source directory: /home/mb/QSF/external/miniupnp/miniupnpc
# Build directory: /home/mb/QSF/build/external/miniupnp/miniupnpc
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(validateminixml "/home/mb/QSF/build/external/miniupnp/miniupnpc/minixmlvalid")
set_tests_properties(validateminixml PROPERTIES  _BACKTRACE_TRIPLES "/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;256;add_test;/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;0;")
add_test(validateminiwget "/home/mb/QSF/testminiwget.sh")
set_tests_properties(validateminiwget PROPERTIES  ENVIRONMENT "TESTSERVER=/home/mb/QSF/build/minihttptestserver;TESTMINIWGET=/home/mb/QSF/build/testminiwget" _BACKTRACE_TRIPLES "/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;258;add_test;/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;0;")
add_test(validateupnpreplyparse "/home/mb/QSF/testupnpreplyparse.sh")
set_tests_properties(validateupnpreplyparse PROPERTIES  ENVIRONMENT "TESTUPNPREPLYPARSE=/home/mb/QSF/build/testupnpreplyparse" WORKING_DIRECTORY "/home/mb/QSF" _BACKTRACE_TRIPLES "/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;266;add_test;/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;0;")
add_test(validateportlistingparse "/home/mb/QSF/build/external/miniupnp/miniupnpc/testportlistingparse")
set_tests_properties(validateportlistingparse PROPERTIES  _BACKTRACE_TRIPLES "/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;272;add_test;/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;0;")
add_test(validateigddescparse1 "/home/mb/QSF/build/external/miniupnp/miniupnpc/testigddescparse" "new_LiveBox_desc.xml" "new_LiveBox_desc.values")
set_tests_properties(validateigddescparse1 PROPERTIES  WORKING_DIRECTORY "/home/mb/QSF/testdesc" _BACKTRACE_TRIPLES "/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;274;add_test;/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;0;")
add_test(validateigddescparse2 "/home/mb/QSF/build/external/miniupnp/miniupnpc/testigddescparse" "linksys_WAG200G_desc.xml" "linksys_WAG200G_desc.values")
set_tests_properties(validateigddescparse2 PROPERTIES  WORKING_DIRECTORY "/home/mb/QSF/testdesc" _BACKTRACE_TRIPLES "/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;277;add_test;/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;0;")
add_test(validateaddr_is_reserved "/home/mb/QSF/build/external/miniupnp/miniupnpc/testaddr_is_reserved")
set_tests_properties(validateaddr_is_reserved PROPERTIES  _BACKTRACE_TRIPLES "/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;280;add_test;/home/mb/QSF/external/miniupnp/miniupnpc/CMakeLists.txt;0;")
