################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../try/Epoll.cpp \
../try/IteratorTreeSerialization.cpp \
../try/Socket.cpp \
../try/SocketSender.cpp \
../try/data_generator.cpp \
../try/select.cpp 

OBJS += \
./try/Epoll.o \
./try/IteratorTreeSerialization.o \
./try/Socket.o \
./try/SocketSender.o \
./try/data_generator.o \
./try/select.o 

CPP_DEPS += \
./try/Epoll.d \
./try/IteratorTreeSerialization.d \
./try/Socket.d \
./try/SocketSender.d \
./try/data_generator.d \
./try/select.d 


# Each subdirectory must supply rules for building sources it contributes
try/%.o: ../try/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


