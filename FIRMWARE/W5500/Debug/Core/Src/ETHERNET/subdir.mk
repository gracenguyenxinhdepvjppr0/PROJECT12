################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ETHERNET/socket.c \
../Core/Src/ETHERNET/w5500_spi.c \
../Core/Src/ETHERNET/wizchip_conf.c 

OBJS += \
./Core/Src/ETHERNET/socket.o \
./Core/Src/ETHERNET/w5500_spi.o \
./Core/Src/ETHERNET/wizchip_conf.o 

C_DEPS += \
./Core/Src/ETHERNET/socket.d \
./Core/Src/ETHERNET/w5500_spi.d \
./Core/Src/ETHERNET/wizchip_conf.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/ETHERNET/%.o Core/Src/ETHERNET/%.su Core/Src/ETHERNET/%.cyclo: ../Core/Src/ETHERNET/%.c Core/Src/ETHERNET/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I"C:/Users/Admin/OneDrive/Documents/Giam_sat_tu_dien/FIRMWARE/W5500/Core/Src/ETHERNET/W5500" -I"C:/Users/Admin/OneDrive/Documents/Giam_sat_tu_dien/FIRMWARE/W5500/Core/Src/ETHERNET" -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-ETHERNET

clean-Core-2f-Src-2f-ETHERNET:
	-$(RM) ./Core/Src/ETHERNET/socket.cyclo ./Core/Src/ETHERNET/socket.d ./Core/Src/ETHERNET/socket.o ./Core/Src/ETHERNET/socket.su ./Core/Src/ETHERNET/w5500_spi.cyclo ./Core/Src/ETHERNET/w5500_spi.d ./Core/Src/ETHERNET/w5500_spi.o ./Core/Src/ETHERNET/w5500_spi.su ./Core/Src/ETHERNET/wizchip_conf.cyclo ./Core/Src/ETHERNET/wizchip_conf.d ./Core/Src/ETHERNET/wizchip_conf.o ./Core/Src/ETHERNET/wizchip_conf.su

.PHONY: clean-Core-2f-Src-2f-ETHERNET

