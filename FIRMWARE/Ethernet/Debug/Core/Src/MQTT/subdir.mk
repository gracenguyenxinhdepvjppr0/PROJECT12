################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/MQTT/MQTT_port.c 

OBJS += \
./Core/Src/MQTT/MQTT_port.o 

C_DEPS += \
./Core/Src/MQTT/MQTT_port.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/MQTT/%.o Core/Src/MQTT/%.su Core/Src/MQTT/%.cyclo: ../Core/Src/MQTT/%.c Core/Src/MQTT/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I"C:/Users/Admin/OneDrive/Documents/Giam_sat_tu_dien/FIRMWARE/Ethernet/MQTT" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-MQTT

clean-Core-2f-Src-2f-MQTT:
	-$(RM) ./Core/Src/MQTT/MQTT_port.cyclo ./Core/Src/MQTT/MQTT_port.d ./Core/Src/MQTT/MQTT_port.o ./Core/Src/MQTT/MQTT_port.su

.PHONY: clean-Core-2f-Src-2f-MQTT

