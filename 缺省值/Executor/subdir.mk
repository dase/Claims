################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Executor/Coordinator.cpp \
../Executor/ExchangeTracker.cpp \
../Executor/ExpanderTracker.cpp \
../Executor/IteratorExecutorMaster.cpp \
../Executor/IteratorExecutorSlave.cpp 

OBJS += \
./Executor/Coordinator.o \
./Executor/ExchangeTracker.o \
./Executor/ExpanderTracker.o \
./Executor/IteratorExecutorMaster.o \
./Executor/IteratorExecutorSlave.o 

CPP_DEPS += \
./Executor/Coordinator.d \
./Executor/ExchangeTracker.d \
./Executor/ExpanderTracker.d \
./Executor/IteratorExecutorMaster.d \
./Executor/IteratorExecutorSlave.d 


# Each subdirectory must supply rules for building sources it contributes
Executor/%.o: ../Executor/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


