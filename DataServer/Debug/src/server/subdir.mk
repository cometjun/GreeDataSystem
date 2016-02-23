################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/server/BufferedFileName.cpp \
../src/server/GetSysMac.cpp \
../src/server/HDFSReadWrite.cpp \
../src/server/LOG.cpp \
../src/server/ListWrapper.cpp \
../src/server/MessageClient.cpp \
../src/server/UTIL.cpp \
../src/server/data_frame.cpp \
../src/server/tsdb_server.cpp \
../src/server/tsdb_session.cpp 

OBJS += \
./src/server/BufferedFileName.o \
./src/server/GetSysMac.o \
./src/server/HDFSReadWrite.o \
./src/server/LOG.o \
./src/server/ListWrapper.o \
./src/server/MessageClient.o \
./src/server/UTIL.o \
./src/server/data_frame.o \
./src/server/tsdb_server.o \
./src/server/tsdb_session.o 

CPP_DEPS += \
./src/server/BufferedFileName.d \
./src/server/GetSysMac.d \
./src/server/HDFSReadWrite.d \
./src/server/LOG.d \
./src/server/ListWrapper.d \
./src/server/MessageClient.d \
./src/server/UTIL.d \
./src/server/data_frame.d \
./src/server/tsdb_server.d \
./src/server/tsdb_session.d 


# Each subdirectory must supply rules for building sources it contributes
src/server/%.o: ../src/server/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/hadoop/hadoop-2.7.1/include -O0 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


