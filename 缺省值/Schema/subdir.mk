################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Schema/LogicalProjection.cpp \
../Schema/Schema.cpp \
../Schema/SchemaFix.cpp \
../Schema/SchemaVar.cpp \
../Schema/TupleConvertor.cpp 

OBJS += \
./Schema/LogicalProjection.o \
./Schema/Schema.o \
./Schema/SchemaFix.o \
./Schema/SchemaVar.o \
./Schema/TupleConvertor.o 

CPP_DEPS += \
./Schema/LogicalProjection.d \
./Schema/Schema.d \
./Schema/SchemaFix.d \
./Schema/SchemaVar.d \
./Schema/TupleConvertor.d 


# Each subdirectory must supply rules for building sources it contributes
Schema/%.o: ../Schema/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


