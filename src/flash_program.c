//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### FLASH_PROGRAM.C #######################
//---------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "flash_program.h"
#include "stm32f0xx.h"
#include <string.h>



/* Externals variables ---------------------------------------------------------*/
extern parameters_typedef param_struct;
//extern mem_bkp_typedef memory_backup;
//extern filesystem_typedef files;
//extern unsigned int v_bkp [];


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/



/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//unsigned int * pmem = (unsigned int *)0x08007000;	//Sector 7 Page 28


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
//unsigned char ReadMem (void)
//{
//	if (*pmem == 0xffffffff)
//		return 1;
//	else
//		return 0;
//}

unsigned char EraseAllMemory_FLASH(void)
{
	unsigned char resp = 0;

	ErasePage(PAGE31,1);
	//reviso si borre

	if (*(unsigned int *) PAGE31 == 0xFFFFFFFF)
		resp++;

	if (resp)
		return PASSED;
	else
		return FAILED;
}

/**
  * @brief    After Reset, the Flash memory Program/Erase Controller is locked.
  * To unlock it, the FLASH_Unlock function is used. Before programming the
  * desired addresses, an erase operation is performed using the flash erase
  * page feature. The erase procedure starts with the calculation of the number
  * of pages to be used. Then all these pages will be erased one by one by
  * calling FLASH_ErasePage function.
  * Once this operation is finished, the programming operation will be performed by
  * using the FLASH_ProgramWord function. The written data is then checked and the
  * result of the programming operation is stored into the MemoryProgramStatus variable.
  * @param  None
  * @retval None
  */

void BackupPage(unsigned int * p, unsigned int * page_addr)
{
	unsigned short i;

	for (i = 0; i < 255; i++)
	{
		*(p + i) = *(page_addr + i);
	}
}

void ErasePage(uint32_t p_addr, unsigned char with_lock)
{
	FLASH_Unlock();
	FLASH_ErasePage(p_addr);
	if (with_lock)
		FLASH_Lock();
}

/**
  * @brief  Unlocks the FLASH control register and program memory access.
  * @param  None
  * @retval None
  */
void FLASH_Unlock(void)
{
  if((FLASH->CR & FLASH_CR_LOCK) != RESET)
  {
    /* Unlocking the program memory access */
    FLASH->KEYR = FLASH_FKEY1;
    FLASH->KEYR = FLASH_FKEY2;
  }
}

/**
  * @brief  Erases a specified page in program memory.
  * @note   To correctly run this function, the FLASH_Unlock() function must be called before.
  * @note   Call the FLASH_Lock() to disable the flash memory access (recommended
  *         to protect the FLASH memory against possible unwanted operation)
  * @param  Page_Address: The page address in program memory to be erased.
  * @note   A Page is erased in the Program memory only if the address to load
  *         is the start address of a page (multiple of 1024 bytes).
  * @retval FLASH Status: The returned value can be:
  *         FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
FLASH_Status FLASH_ErasePage(uint32_t Page_Address)
{
  FLASH_Status status = FLASH_COMPLETE;

  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);

  if(status == FLASH_COMPLETE)
  {
    /* If the previous operation is completed, proceed to erase the page */
    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR  = Page_Address;
    FLASH->CR |= FLASH_CR_STRT;

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);

    /* Disable the PER Bit */
    FLASH->CR &= ~FLASH_CR_PER;
  }

  /* Return the Erase Status */
  return status;
}

/**
  * @brief  Locks the Program memory access.
  * @param  None
  * @retval None
  */
void FLASH_Lock(void)
{
  /* Set the LOCK Bit to lock the FLASH control register and program memory access */
  FLASH->CR |= FLASH_CR_LOCK;
}

/**
  * @brief  Programs a word at a specified address.
  * @note   To correctly run this function, the FLASH_Unlock() function must be called before.
  * @note   Call the FLASH_Lock() to disable the flash memory access (recommended
  *         to protect the FLASH memory against possible unwanted operation)
  * @param  Address: specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
FLASH_Status FLASH_ProgramWord(uint32_t Address, uint32_t Data)
{
  FLASH_Status status = FLASH_COMPLETE;
  __IO uint32_t tmp = 0;

    /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);

  if(status == FLASH_COMPLETE)
  {
    /* If the previous operation is completed, proceed to program the new first
    half word */
    FLASH->CR |= FLASH_CR_PG;

    *(__IO uint16_t*)Address = (uint16_t)Data;

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);

    if(status == FLASH_COMPLETE)
    {
      /* If the previous operation is completed, proceed to program the new second
      half word */
      tmp = Address + 2;

      *(__IO uint16_t*) tmp = Data >> 16;

      /* Wait for last operation to be completed */
      status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);

      /* Disable the PG Bit */
      FLASH->CR &= ~FLASH_CR_PG;
    }
    else
    {
      /* Disable the PG Bit */
      FLASH->CR &= ~FLASH_CR_PG;
    }
  }

  /* Return the Program Status */
  return status;
}

/**
  * @brief  Waits for a FLASH operation to complete or a TIMEOUT to occur.
  * @param  Timeout: FLASH programming Timeout
  * @retval FLASH Status: The returned value can be: FLASH_BUSY,
  *         FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout)
{
  FLASH_Status status = FLASH_COMPLETE;

  /* Check for the FLASH Status */
  status = FLASH_GetStatus();

  /* Wait for a FLASH operation to complete or a TIMEOUT to occur */
  while((status == FLASH_BUSY) && (Timeout != 0x00))
  {
    status = FLASH_GetStatus();
    Timeout--;
  }

  if(Timeout == 0x00 )
  {
    status = FLASH_TIMEOUT;
  }
  /* Return the operation status */
  return status;
}

/**
  * @brief  Returns the FLASH Status.
  * @param  None
  * @retval FLASH Status: The returned value can be:
  *         FLASH_BUSY, FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP or FLASH_COMPLETE.
  */
FLASH_Status FLASH_GetStatus(void)
{
  FLASH_Status FLASHstatus = FLASH_COMPLETE;

  if((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY)
  {
    FLASHstatus = FLASH_BUSY;
  }
  else
  {
    if((FLASH->SR & (uint32_t)FLASH_FLAG_WRPERR)!= (uint32_t)0x00)
    {
      FLASHstatus = FLASH_ERROR_WRP;
    }
    else
    {
      if((FLASH->SR & (uint32_t)(FLASH_SR_PGERR)) != (uint32_t)0x00)
      {
        FLASHstatus = FLASH_ERROR_PROGRAM;
      }
      else
      {
        FLASHstatus = FLASH_COMPLETE;
      }
    }
  }
  /* Return the FLASH Status */
  return FLASHstatus;
}

unsigned char WritePage(unsigned int * p, uint32_t p_addr, unsigned char with_lock)
{
	unsigned short i;
	unsigned int * p_verif;

	p_verif = (unsigned int *) p_addr;

	for (i = 0; i < 255; i++)
	{
		FLASH_ProgramWord(p_addr, *(p + i));
		p_addr += 4;
	}

	if (with_lock)
		FLASH_Lock();

	//verifico memoria
	for (i = 0; i < 255; i++)
	{
		if (*(p_verif + i) != *(p + i))
			return FAILED;
	}
	return PASSED;
}


unsigned char WriteConfigurations (parameters_typedef * p_param)
{
	ErasePage(PAGE63,0);

	//update en memoria
	//p_param = &param_struct;

	if (WriteFlash((unsigned int *) p_param, PAGE63, 1, (sizeof(parameters_typedef) >> 2)) == FAILED)
		return FAILED;

	return PASSED;
}

unsigned char WriteFlash(unsigned int * p, uint32_t p_addr, unsigned char with_lock, unsigned char len_in_4)
{
	unsigned short i;
	unsigned int * p_verif;

	p_verif = (unsigned int *) p_addr;

	for (i = 0; i < len_in_4; i++)
	{
		FLASH_ProgramWord(p_addr, *(p + i));
		p_addr += 4;
	}

	if (with_lock)
		FLASH_Lock();

	//verifico memoria
	for (i = 0; i < len_in_4; i++)
	{
		if (*(p_verif + i) != *(p + i))
			return FAILED;
	}
	return PASSED;
}

void GetFlashConf (parameters_typedef * p_ram)
{
	//memoria no vacia
	strncpy( p_ram->num_reportar,
				((parameters_typedef *) (char *) PAGE63)->num_reportar,
				sizeof(param_struct.num_reportar));

	p_ram->acumm_w2s = ((parameters_typedef *) (unsigned int *) PAGE63)->acumm_w2s;
	p_ram->acumm_w2s_index = ((parameters_typedef *) (unsigned short *) PAGE63)->acumm_w2s_index;
	p_ram->timer_reportar = ((parameters_typedef *) (unsigned char *) PAGE63)->timer_reportar;

}
