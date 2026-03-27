#include "stmflash.h"
#include "stm32f4xx_flash.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////////	 
					  
////////////////////////////////////////////////////////////////////////////////// 

 
 
//��ȡָ����ַ�ĵ��ֽ�(8λ����) 
uint8_t STMFLASH_ReadByte(uint32_t faddr)
{
	return *(__IO uint8_t*)faddr; 
}  
//��ȡָ����ַ��˫�ֽ�(16λ����) 
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr)
{
	return *(__IO uint16_t*)faddr; 
}  
//��ȡָ����ַ�����ֽ�(32λ����) 
uint32_t STMFLASH_ReadWord(uint32_t faddr)
{
	return *(__IO uint32_t*)faddr; 
}  



//��ȡĳ����ַ���ڵ�flash����
//addr:flash��ַ
//����ֵ:0~11,��addr���ڵ�����
uint16_t STMFLASH_GetFlashSector(uint32_t addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_10; 
	return FLASH_Sector_11;	
}

void STMFLASH_Write(uint32_t WriteAddr, uint32_t *pBuffer, uint32_t NumToWrite)
{
    FLASH_Status status = FLASH_COMPLETE;
    uint32_t addrx = 0;
    uint32_t endaddr = 0;
    if (WriteAddr < STM32_FLASH_BASE || WriteAddr % 4)
        return;                  // �Ƿ���ַ
    FLASH_Unlock();              // ����
    FLASH_DataCacheCmd(DISABLE); // FLASH�����ڼ�,�����ֹ���ݻ���

    addrx = WriteAddr;                    // д�����ʼ��ַ
    endaddr = WriteAddr + NumToWrite * 4; // д��Ľ�����ַ
    if (addrx < 0X1FFF0000)               // ֻ�����洢��,����Ҫִ�в�������!!
    {
        while (addrx < endaddr) // ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���)
        {
            if (STMFLASH_ReadWord(addrx) != 0XFFFFFFFF) // �з�0XFFFFFFFF�ĵط�,Ҫ�����������
            {
                status = FLASH_EraseSector(STMFLASH_GetFlashSector(addrx), VoltageRange_3); // VCC=2.7~3.6V֮��!!
                if (status != FLASH_COMPLETE)
                    break; // ����������
            }
            else
                addrx += 4;
        }
    }
    if (status == FLASH_COMPLETE)
    {

        while (WriteAddr < endaddr) // д����
        {
            if (FLASH_ProgramWord(WriteAddr, *pBuffer) != FLASH_COMPLETE) // д������
            {                                                             // printf("!FLASH_COMPLETE\n");
                break;                                                    // д���쳣
            }
            WriteAddr += 4;
            pBuffer++;
        }
    }
    FLASH_DataCacheCmd(ENABLE); // FLASH��������,�������ݻ���
    FLASH_Lock();               // ����
}

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToRead:��(4λ)��
void STMFLASH_Read(uint32_t ReadAddr,uint32_t *pBuffer,uint32_t NumToRead)   	
{
	uint32_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//��ȡ4���ֽ�.
		ReadAddr+=4;//ƫ��4���ֽ�.	
	}
}


int STMFLASH_Write_With_Erase(uint32_t WriteAddr, uint32_t *pBuffer, uint32_t NumToWrite)
{
	FLASH_Status status = FLASH_COMPLETE;
	uint32_t addrx = 0;
	uint32_t endaddr = 0;
	if (WriteAddr < STM32_FLASH_BASE || WriteAddr % 4)
		return -1 ;					 // �Ƿ���ַ
	FLASH_Unlock();				 // ����
	FLASH_DataCacheCmd(DISABLE); // FLASH�����ڼ�,�����ֹ���ݻ���

	addrx = WriteAddr;					  // д�����ʼ��ַ
	endaddr = WriteAddr + NumToWrite * 4; // д��Ľ�����ַ
	if (addrx < 0X1FFF0000)				  // ֻ�����洢��,����Ҫִ�в�������!!
	{
		while (addrx < endaddr) // ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���)
		{
			if (STMFLASH_ReadWord(addrx) != 0XFFFFFFFF) // �з�0XFFFFFFFF�ĵط�,Ҫ�����������
			{
				status = FLASH_EraseSector(STMFLASH_GetFlashSector(addrx), VoltageRange_3); // VCC=2.7~3.6V֮��!!
				if (status != FLASH_COMPLETE)
					// break; // ����������
					return -1;
			}
			else
				addrx += 4;
		}
	}
	if (status == FLASH_COMPLETE)
	{
		while (WriteAddr < endaddr) // д����
		{
			if (FLASH_ProgramWord(WriteAddr, *pBuffer) != FLASH_COMPLETE) // д������
			{
				// break; // д���쳣
				return -1;
			}
			WriteAddr += 4;
			pBuffer++;
		}
	}
	FLASH_DataCacheCmd(ENABLE); // FLASH��������,�������ݻ���
	FLASH_Lock();				// ����
	return 0;
}













