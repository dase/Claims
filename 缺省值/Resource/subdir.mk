################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Resource/NodeTracker.cpp \
../Resource/ResourceInfo.cpp \
../Resource/ResourceManagerMaster.cpp \
../Resource/ResourceManagerSlave.cpp 

OBJS += \
./Resource/NodeTracker.o \
./Resource/ResourceInfo.o \
./Resource/ResourceManagerMaster.o \
./Resource/ResourceManagerSlave.o 

CPP_DEPS += \
./Resource/NodeTracker.d \
./Resource/ResourceInfo.d \
./Resource/ResourceManagerMaster.d \
./Resource/ResourceManagerSlave.d 


# Each subdirectory must supply rules for building sources it contributes
Resource/%.o: ../Resource/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


