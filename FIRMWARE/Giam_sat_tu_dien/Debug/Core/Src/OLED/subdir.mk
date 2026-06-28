################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/OLED/SH1106.c \
../Core/Src/OLED/fonts.c 

OBJS += \
./Core/Src/OLED/SH1106.o \
./Core/Src/OLED/fonts.o 

C_DEPS += \
./Core/Src/OLED/SH1106.d \
./Core/Src/OLED/fonts.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/OLED/%.o Core/Src/OLED/%.su Core/Src/OLED/%.cyclo: ../Core/Src/OLED/%.c Core/Src/OLED/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I"C:/Users/Admin/OneDrive/Documents/Giam_sat_tu_dien/FIRMWARE/Giam_sat_tu_dien/MQTT/MQTTClient-C" -I"C:/Users/Admin/OneDrive/Documents/Giam_sat_tu_dien/FIRMWARE/Giam_sat_tu_dien/MQTT/MQTTPacket" -I"C:/Users/Admin/OneDrive/Documents/Giam_sat_tu_dien/FIRMWARE/Giam_sat_tu_dien/Drivers/DSP/Include" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-OLED

clean-Core-2f-Src-2f-OLED:
	-$(RM) ./Core/Src/OLED/SH1106.cyclo ./Core/Src/OLED/SH1106.d ./Core/Src/OLED/SH1106.o ./Core/Src/OLED/SH1106.su ./Core/Src/OLED/fonts.cyclo ./Core/Src/OLED/fonts.d ./Core/Src/OLED/fonts.o ./Core/Src/OLED/fonts.su

.PHONY: clean-Core-2f-Src-2f-OLED

