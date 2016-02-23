################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cfg/getConfit.cpp 

OBJS += \
./src/cfg/getConfit.o 

CPP_DEPS += \
./src/cfg/getConfit.d 


# Each subdirectory must supply rules for building sources it contributes
src/cfg/%.o: ../src/cfg/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/hadoop/hadoop-2.7.1/include -O0 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


