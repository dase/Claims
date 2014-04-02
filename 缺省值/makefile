################################################################################
# 自动生成的文件。不要编辑！
################################################################################

-include ../makefile.init

RM := rm

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include utility/subdir.mk
-include utility/Test/subdir.mk
-include types/Test/subdir.mk
-include types/subdir.mk
-include try/subdir.mk
-include storage/subdir.mk
-include iterator/subdir.mk
-include common/subdir.mk
-include Test/subdir.mk
-include Serialization/subdir.mk
-include Schema/subdir.mk
-include Schema/Test/subdir.mk
-include Scheduler/test/subdir.mk
-include Scheduler/subdir.mk
-include Resource/subdir.mk
-include PerformanceTest/subdir.mk
-include PerformanceMonitor/subdir.mk
-include Parsetree/subdir.mk
-include Parsetree/Test/subdir.mk
-include LogicalQueryPlan/Test/subdir.mk
-include LogicalQueryPlan/subdir.mk
-include Loader/subdir.mk
-include IndexManager/subdir.mk
-include Executor/Test/subdir.mk
-include Executor/subdir.mk
-include Default/utility/subdir.mk
-include Default/utility/Test/subdir.mk
-include Default/types/Test/subdir.mk
-include Default/types/subdir.mk
-include Default/try/subdir.mk
-include Default/subdir.mk
-include Default/storage/subdir.mk
-include Default/iterator/subdir.mk
-include Default/common/subdir.mk
-include Default/Test/subdir.mk
-include Default/Serialization/subdir.mk
-include Default/Schema/subdir.mk
-include Default/Schema/Test/subdir.mk
-include Default/Scheduler/test/subdir.mk
-include Default/Scheduler/subdir.mk
-include Default/Resource/subdir.mk
-include Default/PerformanceTest/subdir.mk
-include Default/PerformanceMonitor/subdir.mk
-include Default/Parsetree/subdir.mk
-include Default/Parsetree/Test/subdir.mk
-include Default/LogicalQueryPlan/Test/subdir.mk
-include Default/LogicalQueryPlan/subdir.mk
-include Default/Loader/subdir.mk
-include Default/IndexManager/subdir.mk
-include Default/Executor/Test/subdir.mk
-include Default/Executor/subdir.mk
-include Default/Daemon/Test/subdir.mk
-include Default/Daemon/subdir.mk
-include Default/Client/Test/subdir.mk
-include Default/Client/subdir.mk
-include Default/Catalog/subdir.mk
-include Default/Catalog/stat/subdir.mk
-include Default/Catalog/Test/subdir.mk
-include Default/BufferManager/subdir.mk
-include Default/BlockStreamIterator/ParallelBlockStreamIterator/Test/subdir.mk
-include Default/BlockStreamIterator/ParallelBlockStreamIterator/subdir.mk
-include Default/BlockStreamIterator/subdir.mk
-include Default/BlockIterator/subdir.mk
-include Default/Block/subdir.mk
-include Daemon/Test/subdir.mk
-include Daemon/subdir.mk
-include Client/Test/subdir.mk
-include Client/subdir.mk
-include Catalog/subdir.mk
-include Catalog/stat/subdir.mk
-include Catalog/Test/subdir.mk
-include BufferManager/subdir.mk
-include BlockStreamIterator/ParallelBlockStreamIterator/Test/subdir.mk
-include BlockStreamIterator/ParallelBlockStreamIterator/subdir.mk
-include BlockStreamIterator/subdir.mk
-include BlockIterator/subdir.mk
-include Block/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# 所有目标
all: 

# 工具调用
: $(OBJS) $(USER_OBJS)
	@echo '正在构建目标： $@'
	@echo '正在调用： GNU C++ 链接器'
	g++  -o $(OBJS) $(USER_OBJS) $(LIBS)
	@echo '已结束构建目标： $@'
	@echo ' '

# 其他目标
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) 
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
