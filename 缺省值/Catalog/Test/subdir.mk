################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Catalog/Test/Partitioner_test.cpp \
../Catalog/Test/statistic_manager_test.cpp 

OBJS += \
./Catalog/Test/Partitioner_test.o \
./Catalog/Test/statistic_manager_test.o 

CPP_DEPS += \
./Catalog/Test/Partitioner_test.d \
./Catalog/Test/statistic_manager_test.d 


# Each subdirectory must supply rules for building sources it contributes
Catalog/Test/%.o: ../Catalog/Test/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


