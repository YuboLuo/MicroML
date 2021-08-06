################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
decoder/%.obj: ../decoder/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/opt/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/bin/cl430" -vmspx --data_model=large --near_data=none --use_hw_mpy=F5 --include_path="/opt/ti/ccs1040/ccs/ccs_base/msp430/include" --include_path="/home/leonard/Desktop/budgetRNN_CNN" --include_path="/opt/ti/DSPLib_1_30_00_02/include" --include_path="/opt/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/include" --advice:power="all" --advice:hw_config="all" --define=__MSP430FR5994__ --define=_MPU_ENABLE --define=_MPU_MANUAL --define=_MPU_SEGB1=0x10000 --define=_MPU_SEGB2=0x10000 --define=_MPU_SAM1=13 --define=_MPU_SAM2=9 --define=_MPU_SAM3=11 --define=_MPU_SAM0=9 -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="decoder/$(basename $(<F)).d_raw" --obj_directory="decoder" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


