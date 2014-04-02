################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../common/ExpressionCalculator.cpp \
../common/ExpressionItem.cpp \
../common/Expression_item.cpp \
../common/Mapping.cpp \
../common/TypeCast.cpp \
../common/TypePromotionMap.cpp 

OBJS += \
./common/ExpressionCalculator.o \
./common/ExpressionItem.o \
./common/Expression_item.o \
./common/Mapping.o \
./common/TypeCast.o \
./common/TypePromotionMap.o 

CPP_DEPS += \
./common/ExpressionCalculator.d \
./common/ExpressionItem.d \
./common/Expression_item.d \
./common/Mapping.d \
./common/TypeCast.d \
./common/TypePromotionMap.d 


# Each subdirectory must supply rules for building sources it contributes
common/%.o: ../common/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


