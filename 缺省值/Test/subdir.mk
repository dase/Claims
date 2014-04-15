################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Test/Environment_test.cpp \
../Test/MultiToMulti.cpp \
../Test/OneTOTwoToThree.cpp \
../Test/SerializeWideDependency.cpp \
../Test/TestMain.cpp \
../Test/ThreeLevelExchangeEager.cpp \
../Test/VerticalExchange.cpp \
../Test/block_stream_iterator_serilaization_test.cpp \
../Test/block_stream_test.cpp \
../Test/iterator_remote_execute.cpp \
../Test/main.cpp \
../Test/test_adaptiveendpoint.cpp \
../Test/test_coordinator.cpp \
../Test/test_iterator.cpp \
../Test/test_iterator_serialization.cpp \
../Test/test_scdong.cpp 

OBJS += \
./Test/Environment_test.o \
./Test/MultiToMulti.o \
./Test/OneTOTwoToThree.o \
./Test/SerializeWideDependency.o \
./Test/TestMain.o \
./Test/ThreeLevelExchangeEager.o \
./Test/VerticalExchange.o \
./Test/block_stream_iterator_serilaization_test.o \
./Test/block_stream_test.o \
./Test/iterator_remote_execute.o \
./Test/main.o \
./Test/test_adaptiveendpoint.o \
./Test/test_coordinator.o \
./Test/test_iterator.o \
./Test/test_iterator_serialization.o \
./Test/test_scdong.o 

CPP_DEPS += \
./Test/Environment_test.d \
./Test/MultiToMulti.d \
./Test/OneTOTwoToThree.d \
./Test/SerializeWideDependency.d \
./Test/TestMain.d \
./Test/ThreeLevelExchangeEager.d \
./Test/VerticalExchange.d \
./Test/block_stream_iterator_serilaization_test.d \
./Test/block_stream_test.d \
./Test/iterator_remote_execute.d \
./Test/main.d \
./Test/test_adaptiveendpoint.d \
./Test/test_coordinator.d \
./Test/test_iterator.d \
./Test/test_iterator_serialization.d \
./Test/test_scdong.d 


# Each subdirectory must supply rules for building sources it contributes
Test/%.o: ../Test/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


