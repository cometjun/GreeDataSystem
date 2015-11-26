################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ClientServer.cpp \
../src/ClientSession.cpp \
../src/GetSysMac.cpp \
../src/LOG.cpp \
../src/ListWrapper.cpp \
../src/UTIL.cpp \
../src/data_frame.cpp \
../src/main.cpp 

OBJS += \
./src/ClientServer.o \
./src/ClientSession.o \
./src/GetSysMac.o \
./src/LOG.o \
./src/ListWrapper.o \
./src/UTIL.o \
./src/data_frame.o \
./src/main.o 

CPP_DEPS += \
./src/ClientServer.d \
./src/ClientSession.d \
./src/GetSysMac.d \
./src/LOG.d \
./src/ListWrapper.d \
./src/UTIL.d \
./src/data_frame.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


