################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/sensors/MLX90640/MLX90640_API.c \
../Core/Src/sensors/MLX90640/MLX90640_I2C_Driver.c 

OBJS += \
./Core/Src/sensors/MLX90640/MLX90640_API.o \
./Core/Src/sensors/MLX90640/MLX90640_I2C_Driver.o 

C_DEPS += \
./Core/Src/sensors/MLX90640/MLX90640_API.d \
./Core/Src/sensors/MLX90640/MLX90640_I2C_Driver.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/sensors/MLX90640/%.o Core/Src/sensors/MLX90640/%.su Core/Src/sensors/MLX90640/%.cyclo: ../Core/Src/sensors/MLX90640/%.c Core/Src/sensors/MLX90640/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-sensors-2f-MLX90640

clean-Core-2f-Src-2f-sensors-2f-MLX90640:
	-$(RM) ./Core/Src/sensors/MLX90640/MLX90640_API.cyclo ./Core/Src/sensors/MLX90640/MLX90640_API.d ./Core/Src/sensors/MLX90640/MLX90640_API.o ./Core/Src/sensors/MLX90640/MLX90640_API.su ./Core/Src/sensors/MLX90640/MLX90640_I2C_Driver.cyclo ./Core/Src/sensors/MLX90640/MLX90640_I2C_Driver.d ./Core/Src/sensors/MLX90640/MLX90640_I2C_Driver.o ./Core/Src/sensors/MLX90640/MLX90640_I2C_Driver.su

.PHONY: clean-Core-2f-Src-2f-sensors-2f-MLX90640

