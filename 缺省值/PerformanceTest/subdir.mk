################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PerformanceTest/DeepExchangeTree.cpp \
../PerformanceTest/DeepIteratorNoExchange.cpp \
../PerformanceTest/Selectivity_vs_Distribution.cpp \
../PerformanceTest/ThreeLevelExchangeEagerTransform.cpp \
../PerformanceTest/block_exchange_multiple_ways.cpp \
../PerformanceTest/block_iterator.cpp \
../PerformanceTest/block_stream_iterator_test.cpp \
../PerformanceTest/blockstreamaggregationTest.cpp \
../PerformanceTest/blockstreambroadcastTest.cpp \
../PerformanceTest/blockstreamcombineTest.cpp \
../PerformanceTest/blockstreamjoinTest.cpp \
../PerformanceTest/blockstreamjoinTest_____poc.cpp \
../PerformanceTest/expandable_block_stream_test.cpp \
../PerformanceTest/expandable_block_stream_test_two_level.cpp 

OBJS += \
./PerformanceTest/DeepExchangeTree.o \
./PerformanceTest/DeepIteratorNoExchange.o \
./PerformanceTest/Selectivity_vs_Distribution.o \
./PerformanceTest/ThreeLevelExchangeEagerTransform.o \
./PerformanceTest/block_exchange_multiple_ways.o \
./PerformanceTest/block_iterator.o \
./PerformanceTest/block_stream_iterator_test.o \
./PerformanceTest/blockstreamaggregationTest.o \
./PerformanceTest/blockstreambroadcastTest.o \
./PerformanceTest/blockstreamcombineTest.o \
./PerformanceTest/blockstreamjoinTest.o \
./PerformanceTest/blockstreamjoinTest_____poc.o \
./PerformanceTest/expandable_block_stream_test.o \
./PerformanceTest/expandable_block_stream_test_two_level.o 

CPP_DEPS += \
./PerformanceTest/DeepExchangeTree.d \
./PerformanceTest/DeepIteratorNoExchange.d \
./PerformanceTest/Selectivity_vs_Distribution.d \
./PerformanceTest/ThreeLevelExchangeEagerTransform.d \
./PerformanceTest/block_exchange_multiple_ways.d \
./PerformanceTest/block_iterator.d \
./PerformanceTest/block_stream_iterator_test.d \
./PerformanceTest/blockstreamaggregationTest.d \
./PerformanceTest/blockstreambroadcastTest.d \
./PerformanceTest/blockstreamcombineTest.d \
./PerformanceTest/blockstreamjoinTest.d \
./PerformanceTest/blockstreamjoinTest_____poc.d \
./PerformanceTest/expandable_block_stream_test.d \
./PerformanceTest/expandable_block_stream_test_two_level.d 


# Each subdirectory must supply rules for building sources it contributes
PerformanceTest/%.o: ../PerformanceTest/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


