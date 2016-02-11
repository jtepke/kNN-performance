################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/model/MBR.cpp \
../src/model/PointAccessor.cpp \
../src/model/PointArrayAccessor.cpp \
../src/model/PointContainer.cpp \
../src/model/PointVectorAccessor.cpp 

OBJS += \
./src/model/MBR.o \
./src/model/PointAccessor.o \
./src/model/PointArrayAccessor.o \
./src/model/PointContainer.o \
./src/model/PointVectorAccessor.o 

CPP_DEPS += \
./src/model/MBR.d \
./src/model/PointAccessor.d \
./src/model/PointArrayAccessor.d \
./src/model/PointContainer.d \
./src/model/PointVectorAccessor.d 


# Each subdirectory must supply rules for building sources it contributes
src/model/%.o: ../src/model/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O3 -DNDEBUG -Wall -c -g -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


