################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Parsetree/\ semantic_analysis.cpp \
../Parsetree/ExecuteLogicalQueryPlan.cpp \
../Parsetree/function.cpp \
../Parsetree/parsetree2logicalplan.cpp \
../Parsetree/runparsetree.cpp \
../Parsetree/sql.tab.cpp \
../Parsetree/wc2tb.cpp 

C_SRCS += \
../Parsetree/lex.yy.c 

OBJS += \
./Parsetree/\ semantic_analysis.o \
./Parsetree/ExecuteLogicalQueryPlan.o \
./Parsetree/function.o \
./Parsetree/lex.yy.o \
./Parsetree/parsetree2logicalplan.o \
./Parsetree/runparsetree.o \
./Parsetree/sql.tab.o \
./Parsetree/wc2tb.o 

C_DEPS += \
./Parsetree/lex.yy.d 

CPP_DEPS += \
./Parsetree/\ semantic_analysis.d \
./Parsetree/ExecuteLogicalQueryPlan.d \
./Parsetree/function.d \
./Parsetree/parsetree2logicalplan.d \
./Parsetree/runparsetree.d \
./Parsetree/sql.tab.d \
./Parsetree/wc2tb.d 


# Each subdirectory must supply rules for building sources it contributes
Parsetree/\ semantic_analysis.o: ../Parsetree/\ semantic_analysis.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"Parsetree/ semantic_analysis.d" -MT"Parsetree/\ semantic_analysis.d" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '

Parsetree/%.o: ../Parsetree/%.cpp
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C++ 编译器'
	g++ -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '

Parsetree/%.o: ../Parsetree/%.c
	@echo '正在构建文件： $<'
	@echo '正在调用： GNU C 编译器'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo '已结束构建： $<'
	@echo ' '


