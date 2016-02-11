################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../performance-test/grid_performance.cpp \
../performance-test/kNN_test.cpp

OBJS += \
./performance-test/grid_performance.o \
./performance-test/kNN_test.o 

CPP_DEPS += \
./performance-test/grid_performance.d \
./performance-test/kNN_test.d


# Each subdirectory must supply rules for building sources it contributes
performance-test/%.o: ../performance-test/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O3 -DNDEBUG -Wall -c -g -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
