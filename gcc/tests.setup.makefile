
YW_TEST_DIR = $(YW_DIR)/test

YW_INCLUDES += -I$(YW_TEST_DIR)/yw-db-tests
YW_INCLUDES += -I$(YW_TEST_DIR)/yw-test

CPPUTEST_LIBS = -lCppUTestExt -lCppUTest -L$(CPPUTEST_HOME)/lib

CPPFLAGS += -DCPPUTEST 
CPPFLAGS += -I$(CPPUTEST_HOME)/include
