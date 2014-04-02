################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Catalog/Catalog.cpp \
../Catalog/Column.cpp \
../Catalog/Partitioner.cpp \
../Catalog/ProjectionBinding.cpp \
../Catalog/table.cpp 

OBJS += \
./Catalog/Catalog.o \
./Catalog/Column.o \
./Catalog/Partitioner.o \
./Catalog/ProjectionBinding.o \
./Catalog/table.o 

CPP_DEPS += \
./Catalog/Catalog.d \
./Catalog/Column.d \
./Catalog/Partitioner.d \
./Catalog/ProjectionBinding.d \
./Catalog/table.d 


# Each subdirectory must supply rules for building sources it contributes
Catalog/%.o: ../Catalog/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


