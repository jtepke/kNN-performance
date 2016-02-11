################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/naive-map-reduce/NaiveMapReduce.cpp

OBJS += \
./src/naive-map-reduce/NaiveMapReduce.o

CPP_DEPS += \
./src/naive-map-reduce/NaiveMapReduce.d


# Each subdirectory must supply rules for building sources it contributes
src/naive-map-reduce/%.o: ../src/naive-map-reduce/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O3 -DNDEBUG -Wall -c -g -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	echo 'Finished building: $<'
	echo ' '


