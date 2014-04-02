################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../AdaptiveEndPoint.cpp \
../Column_to_be_removed.cpp \
../Comparator.cpp \
../Environment.cpp \
../IDsGenerator.cpp \
../Logging.cpp \
../Message.cpp \
../PortManager.cpp \
../TableManager.cpp \
../TableManagerMaster.cpp \
../TableManagerSlave.cpp \
../TablePartition.cpp \
../TimeOutReceiver.cpp \
../boost.cpp \
../example.cpp \
../hash.cpp \
../hashtable.cpp \
../test.cpp 

OBJS += \
./AdaptiveEndPoint.o \
./Column_to_be_removed.o \
./Comparator.o \
./Environment.o \
./IDsGenerator.o \
./Logging.o \
./Message.o \
./PortManager.o \
./TableManager.o \
./TableManagerMaster.o \
./TableManagerSlave.o \
./TablePartition.o \
./TimeOutReceiver.o \
./boost.o \
./example.o \
./hash.o \
./hashtable.o \
./test.o 

CPP_DEPS += \
./AdaptiveEndPoint.d \
./Column_to_be_removed.d \
./Comparator.d \
./Environment.d \
./IDsGenerator.d \
./Logging.d \
./Message.d \
./PortManager.d \
./TableManager.d \
./TableManagerMaster.d \
./TableManagerSlave.d \
./TablePartition.d \
./TimeOutReceiver.d \
./boost.d \
./example.d \
./hash.d \
./hashtable.d \
./test.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


