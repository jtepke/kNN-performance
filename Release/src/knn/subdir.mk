################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/knn/BPQ.cpp \
../src/knn/KnnProcessor.cpp \
../src/knn/Metrics.cpp \
../src/knn/NaiveKnn.cpp 

OBJS += \
./src/knn/BPQ.o \
./src/knn/KnnProcessor.o \
./src/knn/Metrics.o \
./src/knn/NaiveKnn.o 

CPP_DEPS += \
./src/knn/BPQ.d \
./src/knn/KnnProcessor.d \
./src/knn/Metrics.d \
./src/knn/NaiveKnn.d 


# Each subdirectory must supply rules for building sources it contributes
src/knn/%.o: ../src/knn/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O3 -DNDEBUG -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


