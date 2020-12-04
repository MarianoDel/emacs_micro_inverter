# Inverter Etapa C
micro STM32F030K6T6

Before begin check micro selection and configs.
-----------------------------------------------

* work area size y flash image
>stm32f0_flash.cfg

* work area size y flash image
>stm32f0_flash_lock.cfg

* work area size
>stm32f0_gdb.cfg

* work area size
>stm32f0_reset.cfg

* end of ram; stack size; memory lenght
>./cmsis_boot/startup/stm32_flash.ld

* check processor definition on line 68 -> but modify on Makefile
>./cmsis_boot/stm32f0xx.h
>.Makefile



