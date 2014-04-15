################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LogicalQueryPlan/Test/Aggregation_test.cpp \
../LogicalQueryPlan/Test/ResultCollect_test.cpp \
../LogicalQueryPlan/Test/getOptimalQueryPlan.cpp \
../LogicalQueryPlan/Test/query_optmization_based_on_statisitic_test.cpp \
../LogicalQueryPlan/Test/testGenerateIteratorTree.cpp \
../LogicalQueryPlan/Test/testGetDataflow.cpp \
../LogicalQueryPlan/Test/testIn.cpp \
../LogicalQueryPlan/Test/testProject.cpp \
../LogicalQueryPlan/Test/testProject_wl.cpp \
../LogicalQueryPlan/Test/testSort.cpp 

OBJS += \
./LogicalQueryPlan/Test/Aggregation_test.o \
./LogicalQueryPlan/Test/ResultCollect_test.o \
./LogicalQueryPlan/Test/getOptimalQueryPlan.o \
./LogicalQueryPlan/Test/query_optmization_based_on_statisitic_test.o \
./LogicalQueryPlan/Test/testGenerateIteratorTree.o \
./LogicalQueryPlan/Test/testGetDataflow.o \
./LogicalQueryPlan/Test/testIn.o \
./LogicalQueryPlan/Test/testProject.o \
./LogicalQueryPlan/Test/testProject_wl.o \
./LogicalQueryPlan/Test/testSort.o 

CPP_DEPS += \
./LogicalQueryPlan/Test/Aggregation_test.d \
./LogicalQueryPlan/Test/ResultCollect_test.d \
./LogicalQueryPlan/Test/getOptimalQueryPlan.d \
./LogicalQueryPlan/Test/query_optmization_based_on_statisitic_test.d \
./LogicalQueryPlan/Test/testGenerateIteratorTree.d \
./LogicalQueryPlan/Test/testGetDataflow.d \
./LogicalQueryPlan/Test/testIn.d \
./LogicalQueryPlan/Test/testProject.d \
./LogicalQueryPlan/Test/testProject_wl.d \
./LogicalQueryPlan/Test/testSort.d 


# Each subdirectory must supply rules for building sources it contributes
LogicalQueryPlan/Test/%.o: ../LogicalQueryPlan/Test/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


