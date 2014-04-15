################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../utility/Expression.cpp \
../utility/SQLExpression.cpp \
../utility/ThreadSafe.cpp \
../utility/hashtable2.cpp 

OBJS += \
./utility/Expression.o \
./utility/SQLExpression.o \
./utility/ThreadSafe.o \
./utility/hashtable2.o 

CPP_DEPS += \
./utility/Expression.d \
./utility/SQLExpression.d \
./utility/ThreadSafe.d \
./utility/hashtable2.d 


# Each subdirectory must supply rules for building sources it contributes
utility/%.o: ../utility/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


