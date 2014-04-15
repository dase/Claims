################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Loader/Hdfsconnector.cpp \
../Loader/Hdfsloader.cpp \
../Loader/test_load.cpp 

OBJS += \
./Loader/Hdfsconnector.o \
./Loader/Hdfsloader.o \
./Loader/test_load.o 

CPP_DEPS += \
./Loader/Hdfsconnector.d \
./Loader/Hdfsloader.d \
./Loader/test_load.d 


# Each subdirectory must supply rules for building sources it contributes
Loader/%.o: ../Loader/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


