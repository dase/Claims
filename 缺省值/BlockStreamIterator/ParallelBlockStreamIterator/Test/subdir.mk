################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../BlockStreamIterator/ParallelBlockStreamIterator/Test/Buffer.cpp \
../BlockStreamIterator/ParallelBlockStreamIterator/Test/Expanded_iterators_test.cpp \
../BlockStreamIterator/ParallelBlockStreamIterator/Test/Project_test.cpp \
../BlockStreamIterator/ParallelBlockStreamIterator/Test/Sort_test.cpp \
../BlockStreamIterator/ParallelBlockStreamIterator/Test/TopN_test.cpp \
../BlockStreamIterator/ParallelBlockStreamIterator/Test/in_iterator_test.cpp \
../BlockStreamIterator/ParallelBlockStreamIterator/Test/iterator_test.cpp \
../BlockStreamIterator/ParallelBlockStreamIterator/Test/projectionScan.cpp 

OBJS += \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/Buffer.o \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/Expanded_iterators_test.o \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/Project_test.o \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/Sort_test.o \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/TopN_test.o \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/in_iterator_test.o \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/iterator_test.o \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/projectionScan.o 

CPP_DEPS += \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/Buffer.d \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/Expanded_iterators_test.d \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/Project_test.d \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/Sort_test.d \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/TopN_test.d \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/in_iterator_test.d \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/iterator_test.d \
./BlockStreamIterator/ParallelBlockStreamIterator/Test/projectionScan.d 


# Each subdirectory must supply rules for building sources it contributes
BlockStreamIterator/ParallelBlockStreamIterator/Test/%.o: ../BlockStreamIterator/ParallelBlockStreamIterator/Test/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


