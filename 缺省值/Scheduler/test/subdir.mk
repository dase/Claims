################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Scheduler/test/QueryDagTest.cpp \
../Scheduler/test/SchedulerTest.cpp 

OBJS += \
./Scheduler/test/QueryDagTest.o \
./Scheduler/test/SchedulerTest.o 

CPP_DEPS += \
./Scheduler/test/QueryDagTest.d \
./Scheduler/test/SchedulerTest.d 


# Each subdirectory must supply rules for building sources it contributes
Scheduler/test/%.o: ../Scheduler/test/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


