################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MQTT/MQTTClient-C/MQTTClient.c 

OBJS += \
./MQTT/MQTTClient-C/MQTTClient.o 

C_DEPS += \
./MQTT/MQTTClient-C/MQTTClient.d 


# Each subdirectory must supply rules for building sources it contributes
MQTT/MQTTClient-C/%.o MQTT/MQTTClient-C/%.su MQTT/MQTTClient-C/%.cyclo: ../MQTT/MQTTClient-C/%.c MQTT/MQTTClient-C/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I"C:/Users/Admin/OneDrive/Documents/Giam_sat_tu_dien/FIRMWARE/Giam_sat_tu_dien/MQTT/MQTTClient-C" -I"C:/Users/Admin/OneDrive/Documents/Giam_sat_tu_dien/FIRMWARE/Giam_sat_tu_dien/MQTT/MQTTPacket" -I"C:/Users/Admin/OneDrive/Documents/Giam_sat_tu_dien/FIRMWARE/Giam_sat_tu_dien/Drivers/DSP/Include" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-MQTT-2f-MQTTClient-2d-C

clean-MQTT-2f-MQTTClient-2d-C:
	-$(RM) ./MQTT/MQTTClient-C/MQTTClient.cyclo ./MQTT/MQTTClient-C/MQTTClient.d ./MQTT/MQTTClient-C/MQTTClient.o ./MQTT/MQTTClient-C/MQTTClient.su

.PHONY: clean-MQTT-2f-MQTTClient-2d-C

