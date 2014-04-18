################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Block/Block.cpp \
../Block/BlockContainer.cpp \
../Block/BlockFix.cpp \
../Block/BlockReadable.cpp \
../Block/BlockReadableFix.cpp \
../Block/BlockReadableFixBuffer.cpp \
../Block/BlockStream.cpp \
../Block/BlockStreamBuffer.cpp \
../Block/BlockWritable.cpp \
../Block/BlockWritableFix.cpp \
../Block/BlockWritableFixBuffer.cpp \
../Block/DynamicBlockBuffer.cpp \
../Block/PartitionedBlockBuffer.cpp \
../Block/PartitionedBlockContainer.cpp \
../Block/ResultSet.cpp 

OBJS += \
./Block/Block.o \
./Block/BlockContainer.o \
./Block/BlockFix.o \
./Block/BlockReadable.o \
./Block/BlockReadableFix.o \
./Block/BlockReadableFixBuffer.o \
./Block/BlockStream.o \
./Block/BlockStreamBuffer.o \
./Block/BlockWritable.o \
./Block/BlockWritableFix.o \
./Block/BlockWritableFixBuffer.o \
./Block/DynamicBlockBuffer.o \
./Block/PartitionedBlockBuffer.o \
./Block/PartitionedBlockContainer.o \
./Block/ResultSet.o 

CPP_DEPS += \
./Block/Block.d \
./Block/BlockContainer.d \
./Block/BlockFix.d \
./Block/BlockReadable.d \
./Block/BlockReadableFix.d \
./Block/BlockReadableFixBuffer.d \
./Block/BlockStream.d \
./Block/BlockStreamBuffer.d \
./Block/BlockWritable.d \
./Block/BlockWritableFix.d \
./Block/BlockWritableFixBuffer.d \
./Block/DynamicBlockBuffer.d \
./Block/PartitionedBlockBuffer.d \
./Block/PartitionedBlockContainer.d \
./Block/ResultSet.d 


# Each subdirectory must supply rules for building sources it contributes
Block/%.o: ../Block/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


