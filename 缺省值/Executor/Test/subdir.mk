################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Executor/Test/Executor.cpp 

OBJS += \
./Executor/Test/Executor.o 

CPP_DEPS += \
./Executor/Test/Executor.d 


# Each subdirectory must supply rules for building sources it contributes
Executor/Test/%.o: ../Executor/Test/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


