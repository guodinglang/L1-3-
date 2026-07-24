
#include "Function.h"

//--------------------------------------------------------------------------
SwitchStruct  swMess;



/***************************************************************************
;** ��������:  	WorkStateLED
;** ��������:  	
;** �������: 	��
;** �� �� ֵ: 	
;** �� �� ����  
;** ȫ�ֱ���: 
;** ����ģ��: 
;** ����  ��:   M-L-A
;** �ա�  ��:   2022-5-26
;** �޸�ԭ��
;***************************************************************************/
void WorkStateLED(void)
{
	static uint16_t count;
	
	if(count > 65500)
	{SLED;count=0;}
	else
	{count++;}
}

/***************************************************************************
;** ��������:  EmpowerCheck
;** ��������: ����Ȩ���
;** �������: 	empower
;** �� �� ֵ: 
;** ȫ�ֱ���: 
;** ����ģ��: 
;** ����  ��:   M-L-A
;** �ա�  ��:   2019-12-12
;** �޸�ԭ��
;** ˵    ��:  

;***************************************************************************/
void EmpowerCheck(void)
{

}
/***************************************************************************
;** ��������:  SwitchMessage
;** ��������: 
;** �������: 
;** �� �� ֵ: 
;** ȫ�ֱ���: 
;** ����ģ��: 
;** ����  ��:  M-L-A 
;** �ա�  ��:  2022-2-14
;** �޸�ԭ�� AT32F403A ����������Ŀ
;** ˵    ��: 
;***************************************************************************/
void SwitchMessage(void)
{
	if(swMess.parallel==true)
	{
		BitStructure16 t,o;	
		//0x0336=001100110110
		t.regis = swMess.buff;
		o.regis = 0;
		o.bit.b0 = t.bit.b5; // PB5
		o.bit.b1 = t.bit.b8; // PB8
		o.bit.b2 = t.bit.b2; // PA6
		o.bit.b3 = t.bit.b1; // PA5
		o.bit.b4 = t.bit.b9; // PB9
		o.bit.b5 = t.bit.b4; // PB4
		if(!EP_MODE)
		{//
			delay_us(1);
			if(!EP_MODE)
			{
				o.regis &= params.pps;
				if(o.regis <= params.nCH)
				{
					swMess.ch = o.regis + 1;	//0 -> CH1
                    move_to_point((PositionPoint)swMess.ch);
					sprintfx("\r\nCHAN:%03d\r\n",swMess.ch);
				}
				else
				{printfx("\r\nCHAN:err\r\n");}	
			}
		}

		BUSY(0);
		swMess.parallel=false;
	}
}






