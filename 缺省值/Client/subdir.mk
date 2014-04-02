################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Client/ClaimsServer.cpp \
../Client/Client.cpp \
../Client/ClientResponse.cpp 

OBJS += \
./Client/ClaimsServer.o \
./Client/Client.o \
./Client/ClientResponse.o 

CPP_DEPS += \
./Client/ClaimsServer.d \
./Client/Client.d \
./Client/ClientResponse.d 


# Each subdirectory must supply rules for building sources it contributes
Client/%.o: ../Client/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


