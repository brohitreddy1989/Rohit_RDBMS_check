################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../DBENGINE/conversions.cpp \
../DBENGINE/dbheader.cpp \
../DBENGINE/exprinfo.cpp \
../DBENGINE/globalstructures.cpp \
../DBENGINE/indextree.cpp \
../DBENGINE/schema.cpp 

OBJS += \
./DBENGINE/conversions.o \
./DBENGINE/dbheader.o \
./DBENGINE/exprinfo.o \
./DBENGINE/globalstructures.o \
./DBENGINE/indextree.o \
./DBENGINE/schema.o 

CPP_DEPS += \
./DBENGINE/conversions.d \
./DBENGINE/dbheader.d \
./DBENGINE/exprinfo.d \
./DBENGINE/globalstructures.d \
./DBENGINE/indextree.d \
./DBENGINE/schema.d 


# Each subdirectory must supply rules for building sources it contributes
DBENGINE/%.o: ../DBENGINE/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


