################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LogicalQueryPlan/Aggregation.cpp \
../LogicalQueryPlan/Buffer.cpp \
../LogicalQueryPlan/DataflowPartition.cpp \
../LogicalQueryPlan/DataflowPartitionDescriptor.cpp \
../LogicalQueryPlan/EqualJoin.cpp \
../LogicalQueryPlan/Filter.cpp \
../LogicalQueryPlan/LogicalOperator.cpp \
../LogicalQueryPlan/LogicalQueryPlanRoot.cpp \
../LogicalQueryPlan/Project.cpp \
../LogicalQueryPlan/Requirement.cpp \
../LogicalQueryPlan/Scan.cpp \
../LogicalQueryPlan/Sort.cpp 

OBJS += \
./LogicalQueryPlan/Aggregation.o \
./LogicalQueryPlan/Buffer.o \
./LogicalQueryPlan/DataflowPartition.o \
./LogicalQueryPlan/DataflowPartitionDescriptor.o \
./LogicalQueryPlan/EqualJoin.o \
./LogicalQueryPlan/Filter.o \
./LogicalQueryPlan/LogicalOperator.o \
./LogicalQueryPlan/LogicalQueryPlanRoot.o \
./LogicalQueryPlan/Project.o \
./LogicalQueryPlan/Requirement.o \
./LogicalQueryPlan/Scan.o \
./LogicalQueryPlan/Sort.o 

CPP_DEPS += \
./LogicalQueryPlan/Aggregation.d \
./LogicalQueryPlan/Buffer.d \
./LogicalQueryPlan/DataflowPartition.d \
./LogicalQueryPlan/DataflowPartitionDescriptor.d \
./LogicalQueryPlan/EqualJoin.d \
./LogicalQueryPlan/Filter.d \
./LogicalQueryPlan/LogicalOperator.d \
./LogicalQueryPlan/LogicalQueryPlanRoot.d \
./LogicalQueryPlan/Project.d \
./LogicalQueryPlan/Requirement.d \
./LogicalQueryPlan/Scan.d \
./LogicalQueryPlan/Sort.d 


# Each subdirectory must supply rules for building sources it contributes
LogicalQueryPlan/%.o: ../LogicalQueryPlan/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


