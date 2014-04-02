################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Catalog/stat/Analyzer.cpp \
../Catalog/stat/AttributeStatistics.cpp \
../Catalog/stat/Estimation.cpp \
../Catalog/stat/StatManager.cpp \
../Catalog/stat/Statistic.cpp \
../Catalog/stat/TableStatistic.cpp 

OBJS += \
./Catalog/stat/Analyzer.o \
./Catalog/stat/AttributeStatistics.o \
./Catalog/stat/Estimation.o \
./Catalog/stat/StatManager.o \
./Catalog/stat/Statistic.o \
./Catalog/stat/TableStatistic.o 

CPP_DEPS += \
./Catalog/stat/Analyzer.d \
./Catalog/stat/AttributeStatistics.d \
./Catalog/stat/Estimation.d \
./Catalog/stat/StatManager.d \
./Catalog/stat/Statistic.d \
./Catalog/stat/TableStatistic.d 


# Each subdirectory must supply rules for building sources it contributes
Catalog/stat/%.o: ../Catalog/stat/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


