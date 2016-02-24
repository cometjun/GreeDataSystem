################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/BufferedFileName.cpp \
../src/HDFSReadWrite.cpp \
../src/HdfsConnectionPool.cpp \
../src/LOG.cpp \
../src/UTIL.cpp \
../src/getConfit.cpp \
../src/main.cpp \
../src/uploadService.cpp 

OBJS += \
./src/BufferedFileName.o \
./src/HDFSReadWrite.o \
./src/HdfsConnectionPool.o \
./src/LOG.o \
./src/UTIL.o \
./src/getConfit.o \
./src/main.o \
./src/uploadService.o 

CPP_DEPS += \
./src/BufferedFileName.d \
./src/HDFSReadWrite.d \
./src/HdfsConnectionPool.d \
./src/LOG.d \
./src/UTIL.d \
./src/getConfit.d \
./src/main.d \
./src/uploadService.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/hadoop/hadoop-2.7.1/include -O0 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


