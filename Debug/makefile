################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init
-include config.mk

RM := rm -rf

# All of the sources participating in the build are defined here
O_SRCS := 
CPP_SRCS := 
C_UPPER_SRCS := 
C_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
CXX_SRCS := 
C++_SRCS := 
CC_SRCS := 
C++_DEPS := 
OBJS := 
C_DEPS := 
CC_DEPS := 
CPP_DEPS := 
EXECUTABLES := 
CXX_DEPS := 
C_UPPER_DEPS := 



# Every subdirectory with source files must be described here
SUBDIRS := \
src \

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/DicomPreProcessor.cpp 

OBJS += \
./src/DicomPreProcessor.o 

CPP_DEPS += \
./src/DicomPreProcessor.d 

#INCLUDE_PATH = -I"../lib/dcmtk-3.6.1/include-ubuntu"

##TO MODIFY FOR OLD VERSIONS OF g++
#CXX_FLAG = -stdlib=libstdc++

#INCLUDE_PATH = -I"../lib/dcmtk-3.6.1/include-cygwin"
#LIBB = -L"../lib/dcmtk-3.6.1/lib-cygwin"

# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ $(INCLUDE_PATH) -O0 -g3 -Wall -c -fmessage-length=0 $(CXX_FLAG) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


USER_OBJS :=

LIBS := -pthread -lcharls -ldcmdata -ldcmdsig -ldcmimage -ldcmimgle -ldcmjpeg -ldcmjpls -ldcmnet -ldcmpstat -ldcmqrdb -ldcmsr -ldcmtls -ldcmwlm -lijg12 -lijg16 -lijg8 -li2d -loflog -lofstd -lz $(WINLIB)
#-lcharset -liconv
#-pthread
#LIBB = -L"../lib/dcmtk-3.6.1/lib-ubuntu" 
#-L"/usr/lib/x86_64-linux-gnu"

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: DicomPreProcessor

# Tool invocations
DicomPreProcessor: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MacOS X C++ Linker'
	g++ $(LIBB) $(CXX_FLAG) -o "DicomPreProcessor" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C++_DEPS)$(OBJS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) DicomPreProcessor*
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
