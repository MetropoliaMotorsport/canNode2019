################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../startup/startup_stm32f103xb.s 

OBJS += \
./startup/startup_stm32f103xb.o 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.s
	arm-none-eabi-gcc -mcpu=cortex-m3 -g3 -c -I"C:/Users/user/Desktop/Programming/stm32s/canNode2019/Drivers/STM32F1xx_HAL_Driver/Src" -I"C:/Users/user/Desktop/Programming/stm32s/canNode2019/Drivers/STM32F1xx_HAL_Driver/Inc" -Wa,-W -x assembler-with-cpp --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"

