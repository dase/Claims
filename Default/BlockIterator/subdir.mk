################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../BlockIterator/BlockIteratorFix.cpp \
../BlockIterator/CopyBasedFilterIteratorBlockFix.cpp \
../BlockIterator/SingleColumnScanBlockIterator.cpp 

OBJS += \
./BlockIterator/BlockIteratorFix.o \
./BlockIterator/CopyBasedFilterIteratorBlockFix.o \
./BlockIterator/SingleColumnScanBlockIterator.o 

CPP_DEPS += \
./BlockIterator/BlockIteratorFix.d \
./BlockIterator/CopyBasedFilterIteratorBlockFix.d \
./BlockIterator/SingleColumnScanBlockIterator.d 


# Each subdirectory must supply rules for building sources it contributes
BlockIterator/%.o: ../BlockIterator/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/imdb/tools/Theron-5.01.01/Include -O0 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


