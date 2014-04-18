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
-include utility/Block/subdir.mk
-include types/Test/subdir.mk
-include types/subdir.mk
-include try/subdir.mk
-include storage/subdir.mk
-include iterator/subdir.mk
-include common/subdir.mk
-include ThirdParty/Test/subdir.mk
-include Test/subdir.mk
-include Test/TestSuit/subdir.mk
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
all: ClaimsForYu

# 工具调用
ClaimsForYu: $(OBJS) $(USER_OBJS)
	@echo '正在构建目标： $@'
	@echo '正在调用： GNU C++ 链接器'
	g++  -o"ClaimsForYu" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo '已结束构建目标： $@'
	@echo ' '

# 其他目标
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) ClaimsForYu
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
