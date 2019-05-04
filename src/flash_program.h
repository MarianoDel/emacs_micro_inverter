//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F103
// ##
// #### FLASH_PROGRAM.H ################################
//---------------------------------------------
#ifndef _FLASH_PROGRAM_H_
#define _FLASH_PROGRAM_H_


#include "stm32f0xx.h"


// Define the STM32F10x FLASH Page Size depending on the used STM32 device
// si es mayor a 128K la pagina es de 2KB, sino 1KB

//#define FLASH_PAGE_SIZE    ((uint16_t)0x400)
#define FLASH_PAGE_SIZE			1024
#define FLASH_PAGE_SIZE_DIV2	512
#define FLASH_PAGE_SIZE_DIV4	256

#define PAGE28_START	0
#define PAGE29_START	256
#define PAGE30_START	512
#define PAGE31_START	768
#define PAGE31_END		1023

#define PAGE27			((uint32_t)0x08006C00)
#define PAGE28			((uint32_t)0x08007000)
#define PAGE29			((uint32_t)0x08007400)
#define PAGE30			((uint32_t)0x08007800)
#define PAGE31			((uint32_t)0x08007C00)

#define PAGE63			((uint32_t)0x0800FC00)


//de libreria st las tiene #include "stm32f0xx_flash.h"
/**
  * @brief  FLASH Status
  */
typedef enum
{
  FLASH_BUSY = 1,
  FLASH_ERROR_WRP,
  FLASH_ERROR_PROGRAM,
  FLASH_COMPLETE,
  FLASH_TIMEOUT
}FLASH_Status;

/** @defgroup FLASH_Timeout_definition
  * @{
  */
#define FLASH_ER_PRG_TIMEOUT         ((uint32_t)0x000B0000)

/** @defgroup FLASH_Flags
  * @{
  */

#define FLASH_FLAG_BSY                 FLASH_SR_BSY     /*!< FLASH Busy flag */
#define FLASH_FLAG_PGERR               FLASH_SR_PGERR   /*!< FLASH Programming error flag */
#define FLASH_FLAG_WRPERR              FLASH_SR_WRPERR  /*!< FLASH Write protected error flag */
#define FLASH_FLAG_EOP                 FLASH_SR_EOP     /*!< FLASH End of Programming flag */

#define IS_FLASH_CLEAR_FLAG(FLAG) ((((FLAG) & (uint32_t)0xFFFFFFCB) == 0x00000000) && ((FLAG) != 0x00000000))

#define IS_FLASH_GET_FLAG(FLAG)  (((FLAG) == FLASH_FLAG_BSY) || ((FLAG) == FLASH_FLAG_PGERR) || \
                                  ((FLAG) == FLASH_FLAG_WRPERR) || ((FLAG) == FLASH_FLAG_EOP))

 //OJO esta estructuras deben estar alineadas en 4 bytes
 typedef struct parameters {

 	char num_reportar [24];			//24
 	char imei [24];					//48
 	char num_propio [24];			//72

 	unsigned int acumm_wh;				//76
 	unsigned int acumm_w2s;				//80
 	unsigned short acumm_w2s_index;	//82

 	unsigned char timer_reportar;	//83

	unsigned char send_energy_flag;	//84
 	//dummys para completar
 // 	unsigned char dummy1;			//84
 	// unsigned char dummy2;			//83
 	// unsigned char dummy3;			//84

 } parameters_typedef;


#define timer_rep			param_struct.timer_reportar
#define num_tel_rep		param_struct.num_reportar

#define send_energy			(param_struct.send_energy_flag & 0x01)
#define send_energy_set		(param_struct.send_energy_flag |= 0x01)
#define send_energy_reset	(param_struct.send_energy_flag &= 0xFE)
#define send_sms_ok			(param_struct.send_energy_flag & 0x02)
#define send_sms_ok_set		(param_struct.send_energy_flag |= 0x02)
#define send_sms_ok_reset	(param_struct.send_energy_flag &= 0xFD)

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

//-------- Functions -------------

unsigned char ReadMem (void);
unsigned char EraseAllMemory_FLASH(void);
void BackupPage(unsigned int *, unsigned int *);
void ErasePage(uint32_t , unsigned char );

//de libreria st las tiene #include "stm32f0xx_flash.h"
void FLASH_Unlock(void);
FLASH_Status FLASH_ErasePage(uint32_t Page_Address);
void FLASH_Lock(void);
FLASH_Status FLASH_ProgramWord(uint32_t Address, uint32_t Data);
FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout);
FLASH_Status FLASH_GetStatus(void);

unsigned char WriteConfigurations (parameters_typedef *);
unsigned char WriteFlash(unsigned int * p, uint32_t p_addr, unsigned char with_lock, unsigned char len_in_4);
void GetFlashConf (parameters_typedef * );

#endif    /* _FLASH_PROGRAM_H_ */

//--- end of file ---//
