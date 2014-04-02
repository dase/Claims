################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../BlockStreamIterator/BlockStreamExchangeBase.cpp \
../BlockStreamIterator/BlockStreamExchangeLowerBase.cpp \
../BlockStreamIterator/BlockStreamFilter.cpp \
../BlockStreamIterator/BlockStreamIteratorBase.cpp \
../BlockStreamIterator/BlockStreamPrint.cpp \
../BlockStreamIterator/BlockStreamRandomMemAccess.cpp \
../BlockStreamIterator/BlockStreamResultCollector.cpp \
../BlockStreamIterator/BlockStreamSingleColumnScan.cpp \
../BlockStreamIterator/ExpandableBlockStreamIteratorBase.cpp 

OBJS += \
./BlockStreamIterator/BlockStreamExchangeBase.o \
./BlockStreamIterator/BlockStreamExchangeLowerBase.o \
./BlockStreamIterator/BlockStreamFilter.o \
./BlockStreamIterator/BlockStreamIteratorBase.o \
./BlockStreamIterator/BlockStreamPrint.o \
./BlockStreamIterator/BlockStreamRandomMemAccess.o \
./BlockStreamIterator/BlockStreamResultCollector.o \
./BlockStreamIterator/BlockStreamSingleColumnScan.o \
./BlockStreamIterator/ExpandableBlockStreamIteratorBase.o 

CPP_DEPS += \
./BlockStreamIterator/BlockStreamExchangeBase.d \
./BlockStreamIterator/BlockStreamExchangeLowerBase.d \
./BlockStreamIterator/BlockStreamFilter.d \
./BlockStreamIterator/BlockStreamIteratorBase.d \
./BlockStreamIterator/BlockStreamPrint.d \
./BlockStreamIterator/BlockStreamRandomMemAccess.d \
./BlockStreamIterator/BlockStreamResultCollector.d \
./BlockStreamIterator/BlockStreamSingleColumnScan.d \
./BlockStreamIterator/ExpandableBlockStreamIteratorBase.d 


# Each subdirectory must supply rules for building sources it contributes
BlockStreamIterator/%.o: ../BlockStreamIterator/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


