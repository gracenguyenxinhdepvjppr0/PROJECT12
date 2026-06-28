################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MQTT/MQTTPacket/MQTTConnectClient.c \
../MQTT/MQTTPacket/MQTTConnectServer.c \
../MQTT/MQTTPacket/MQTTDeserializePublish.c \
../MQTT/MQTTPacket/MQTTFormat.c \
../MQTT/MQTTPacket/MQTTPacket.c \
../MQTT/MQTTPacket/MQTTSerializePublish.c \
../MQTT/MQTTPacket/MQTTSubscribeClient.c \
../MQTT/MQTTPacket/MQTTSubscribeServer.c \
../MQTT/MQTTPacket/MQTTUnsubscribeClient.c \
../MQTT/MQTTPacket/MQTTUnsubscribeServer.c 

OBJS += \
./MQTT/MQTTPacket/MQTTConnectClient.o \
./MQTT/MQTTPacket/MQTTConnectServer.o \
./MQTT/MQTTPacket/MQTTDeserializePublish.o \
./MQTT/MQTTPacket/MQTTFormat.o \
./MQTT/MQTTPacket/MQTTPacket.o \
./MQTT/MQTTPacket/MQTTSerializePublish.o \
./MQTT/MQTTPacket/MQTTSubscribeClient.o \
./MQTT/MQTTPacket/MQTTSubscribeServer.o \
./MQTT/MQTTPacket/MQTTUnsubscribeClient.o \
./MQTT/MQTTPacket/MQTTUnsubscribeServer.o 

C_DEPS += \
./MQTT/MQTTPacket/MQTTConnectClient.d \
./MQTT/MQTTPacket/MQTTConnectServer.d \
./MQTT/MQTTPacket/MQTTDeserializePublish.d \
./MQTT/MQTTPacket/MQTTFormat.d \
./MQTT/MQTTPacket/MQTTPacket.d \
./MQTT/MQTTPacket/MQTTSerializePublish.d \
./MQTT/MQTTPacket/MQTTSubscribeClient.d \
./MQTT/MQTTPacket/MQTTSubscribeServer.d \
./MQTT/MQTTPacket/MQTTUnsubscribeClient.d \
./MQTT/MQTTPacket/MQTTUnsubscribeServer.d 


# Each subdirectory must supply rules for building sources it contributes
MQTT/MQTTPacket/%.o MQTT/MQTTPacket/%.su MQTT/MQTTPacket/%.cyclo: ../MQTT/MQTTPacket/%.c MQTT/MQTTPacket/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I"C:/Users/Admin/OneDrive/Documents/Giam_sat_tu_dien/FIRMWARE/Giam_sat_tu_dien/MQTT/MQTTClient-C" -I"C:/Users/Admin/OneDrive/Documents/Giam_sat_tu_dien/FIRMWARE/Giam_sat_tu_dien/MQTT/MQTTPacket" -I"C:/Users/Admin/OneDrive/Documents/Giam_sat_tu_dien/FIRMWARE/Giam_sat_tu_dien/Drivers/DSP/Include" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-MQTT-2f-MQTTPacket

clean-MQTT-2f-MQTTPacket:
	-$(RM) ./MQTT/MQTTPacket/MQTTConnectClient.cyclo ./MQTT/MQTTPacket/MQTTConnectClient.d ./MQTT/MQTTPacket/MQTTConnectClient.o ./MQTT/MQTTPacket/MQTTConnectClient.su ./MQTT/MQTTPacket/MQTTConnectServer.cyclo ./MQTT/MQTTPacket/MQTTConnectServer.d ./MQTT/MQTTPacket/MQTTConnectServer.o ./MQTT/MQTTPacket/MQTTConnectServer.su ./MQTT/MQTTPacket/MQTTDeserializePublish.cyclo ./MQTT/MQTTPacket/MQTTDeserializePublish.d ./MQTT/MQTTPacket/MQTTDeserializePublish.o ./MQTT/MQTTPacket/MQTTDeserializePublish.su ./MQTT/MQTTPacket/MQTTFormat.cyclo ./MQTT/MQTTPacket/MQTTFormat.d ./MQTT/MQTTPacket/MQTTFormat.o ./MQTT/MQTTPacket/MQTTFormat.su ./MQTT/MQTTPacket/MQTTPacket.cyclo ./MQTT/MQTTPacket/MQTTPacket.d ./MQTT/MQTTPacket/MQTTPacket.o ./MQTT/MQTTPacket/MQTTPacket.su ./MQTT/MQTTPacket/MQTTSerializePublish.cyclo ./MQTT/MQTTPacket/MQTTSerializePublish.d ./MQTT/MQTTPacket/MQTTSerializePublish.o ./MQTT/MQTTPacket/MQTTSerializePublish.su ./MQTT/MQTTPacket/MQTTSubscribeClient.cyclo ./MQTT/MQTTPacket/MQTTSubscribeClient.d ./MQTT/MQTTPacket/MQTTSubscribeClient.o ./MQTT/MQTTPacket/MQTTSubscribeClient.su ./MQTT/MQTTPacket/MQTTSubscribeServer.cyclo ./MQTT/MQTTPacket/MQTTSubscribeServer.d ./MQTT/MQTTPacket/MQTTSubscribeServer.o ./MQTT/MQTTPacket/MQTTSubscribeServer.su ./MQTT/MQTTPacket/MQTTUnsubscribeClient.cyclo ./MQTT/MQTTPacket/MQTTUnsubscribeClient.d ./MQTT/MQTTPacket/MQTTUnsubscribeClient.o ./MQTT/MQTTPacket/MQTTUnsubscribeClient.su ./MQTT/MQTTPacket/MQTTUnsubscribeServer.cyclo ./MQTT/MQTTPacket/MQTTUnsubscribeServer.d ./MQTT/MQTTPacket/MQTTUnsubscribeServer.o ./MQTT/MQTTPacket/MQTTUnsubscribeServer.su

.PHONY: clean-MQTT-2f-MQTTPacket

