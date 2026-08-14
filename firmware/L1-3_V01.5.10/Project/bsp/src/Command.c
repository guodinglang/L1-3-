/****************************************************************************************
*
*	模块名称 : BSP(Board Support Package)
*	文件名称 : 
*	版    本 : V0.1
*	说    明 : BSP 板级支持包(Board Support Package)
*	修改记录 :
*   版 本 号 :  
*   日    期 :     
*   作    者 :   
*	说    明 :
*
*****************************************************************************************/
#include "project.h"
#include "Command.h"
#include "CommandList.h"
#include "tmc2209.h"

/***************************************************************************
** 函数名称:  	
** 功能描述:   大写转小写	 
** 输入参数: 	 无
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2014-4-29
** 修改原因：
** 说    明:    
;***************************************************************************/
int gLowerCaseString(char *str)
{
    unsigned char i,len;
    
    len=strlen(str);
    for(i=0;i<len;i++)
    {
	    if((str[i]>='A')&&(str[i]<='Z'))
	    {
		    str[i]=str[i]+0x20; 
		}
	 }
	 return 0;
}

/***************************************************************************
** 函数名称:  rxCommand	
** 功能描述:  	 
** 输入参数: 	 无
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2014-4-29
** 修改原因：
** 说    明:    字数--ASCII;  0--30; 9--39;
;***************************************************************************/
void rxCommand(void)
{
	uint8_t n,c=0xff;
	char Tstr[12]={0,0,0,0,0,0,0,0,0,0,0,0};	

	if(pUART->len >= RX_MAX_FIFO)
	{	//超出长度
		memset(pUART->rxBuf,0,RX_MAX_FIFO);
		pUART->len = 0;
		pUART->rxn = 0;
		pUART->reTx = 0;
	}
//--------返回每一个字符处理----------------------------------------------------    
	if(pUART->feelback==true)
	{
		while(pUART->len > pUART->reTx) 
		{printx(pUART->rxBuf[pUART->reTx++]);}	
		if(pUART->rxn)
		{printfx("\r\n");}
	}
//--------指令判断--------------------------------------------------------------  
	if(pUART->rxn > 0)
	{
		if(pUART->len > 0)
		{
			BUSY(1);	//busying
			
			for(n=0;n<8;n++)
			{
				if((pUART->rxBuf[n] == ' ')||(pUART->rxBuf[n] == '\0'))
				{break;}
				else
				{Tstr[n]=pUART->rxBuf[n];}
			}
			gLowerCaseString(Tstr);
			for(n=0;n<cmdStrLen;n++)  ///匹配命令编号
			{						
				if(strcmp(Tstr,cmdMessage[n].cmdStr)==0)
				{
					c = n;
					break;
				}
				else if(n==6)
				{//*sw001
					if(strncmp(Tstr,cmdMessage[n].cmdStr,3)==0)
					{
						c = n;
						break;
					}
				}
			}
			if(n >= cmdStrLen)
			{				
			   memset(pUART->rxBuf,0,RX_MAX_FIFO);
			   printfx("command illegal\r\n"); //输入的命令非法 
			}
			else if(c!=0xff)
			{	//执行函数
				cmdMessage[c].func(pUART->rxBuf);
				memset(pUART->rxBuf,0,RX_MAX_FIFO);				
			} 
			pUART->len = 0;		
			BUSY(0);	//exit busy 
		}
		else
		{printfx("\r\n@_@\r\n");}
		
		pUART->reTx=0;
		pUART->rxn=0;
	}	
}

/***************************************************************************
** 函数名称:  	help
** 功能描述:  	 
** 输入参数: 	 无
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2019-9-16
** 修改原因：
** 说    明:    字数--ASCII;  0--30; 9--39;
;***************************************************************************/
char help(void *p,...)
{
	uint8_t i;
	for(i=0; i<cmdStrLen; i++)
	{
		printfx((char *)cmdMessage[i].rChar);	
	}	

	return 0;
}

/***************************************************************************
** 函数名称:  	RebackCharacter
** 功能描述:  	 
** 输入参数: 	 无
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2019-6-8
** 修改原因：
** 说    明:    字数--ASCII;  0--30; 9--39;
;***************************************************************************/
char RebackCharacter(void *p,...)
{
	int s;
	
	sscanf(p, "%*s %d",&s);	
	if(s==0)
	{
		pUART->feelback = false;
		pUART->reTx=0;
		printfx(p);
		printfx(" pass\r\n");
	}
	else if(s==1)
	{
		pUART->feelback = true;
		pUART->reTx=pUART->len;
		printfx(p);
		printfx(" pass\r\n");
	}
	else
	{
		printfx(p);
		printfx(" fail\r\n");
	}
	
	return 0;
}
/***************************************************************************
** 函数名称:  	ReadVersion
** 功能描述:  	 
** 输入参数: 	 无
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2019-9-16
** 修改原因：
** 说    明:    字数--ASCII;  0--30; 9--39;
;***************************************************************************/
char ReadVersion(void *p,...)
{
//	printfx("\r\nPCB:B3-1,PGR:V29\r\n");	
	char i,c,n,j,str[25]={0};
	
	memset(str,0,sizeof(str));
	for(i=0,j=0,n=0;c!='\0';i++)
	{
		c =((char *)p)[i];
		if(c=='_' || c==' ')
		{n++;}
		else if(n > 0)
		{str[j++]=c;}
		if(n > 1)
		{break;}
	}
	gLowerCaseString(str);
	
	if(strcmp(str,"?")==0)
	{
		printfx("\r\n");
		printfx((char *)params.ver);
		printfx("\r\n"); 		
	}
	else if(strcmp(str,"w")==0)
	{
		memset(str,0,sizeof(str));
		for(i=0,j=0,n=0;c!='\0';i++)
		{
			c =((char *)p)[i];
			if((c=='_' || c==' ')&&(n < 2))
			{n++;}
			else if(n > 1)
			{str[j++]=c;}
		}
		n=strlen(str);
		if(n>0 && n<24)
		{
			memcpy(params.ver,str,sizeof(params.ver));
			W25Q128_SaveParamet(FLASH_SECTION_PARAMETER,(uint8_t*)&params);//SaveParamet();
			printfx(p);printfx(" pass\r\n");
		}
		else
		{printfx(p);printfx(" fail\r\n");}
	}
	else
	{printfx(p);printfx(" fail\r\n");}
	
	return 0;
	
}
/***************************************************************************
** 函数名称:  	SetPassWord
** 功能描述:  	 
** 输入参数: 	 无
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2020-1-29
** 修改原因：
** 说    明:    
;***************************************************************************/
char ReadWriteBoardIDN(void *p,...)
{
	int k=0;
	char str[48]={0};//,str2[25]={0};
	
	memset(str,0,sizeof(str));
	sscanf(p,"%*s %s",str);
	
	gLowerCaseString(str);
	if((strcmp(str,"?")==0)||(strlen(str)<=0))
	{
		printfx("\r\n");
		printfx((char *)params.IDN);
		printfx("\r\n"); 		
	}
	else if(strcmp(str,"w")==0)
	{
		memset(str,0,sizeof(str));
		sscanf(p,"%*s %*s %[^'\']",str);
		k = strlen(str);	
		if((k > 0)&&(k < 24))
		{
			memset(params.IDN,0,sizeof(params.IDN));
			memcpy(params.IDN,str,k);
			W25Q128_SaveParamet(FLASH_SECTION_PARAMETER,(uint8_t*)&params);//SaveParamet();
			printfx(p);
			printfx(" pass\r\n");
		}
		else
		{printfx(p);printfx(" fail\r\n");}  
	}
	else
	{printfx(p);printfx(" fail\r\n");}
	
	return 0;
}
/***************************************************************************
** 函数名称:  	SetPassWord
** 功能描述:  	 
** 输入参数: 	 无
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2020-1-29
** 修改原因：
** 说    明:    
;***************************************************************************/
char ReadWriteBoardPN(void *p,...)
{
	int k=0;
	char str[48]={0};//,str2[17]={0};
	
	memset(str,0,sizeof(str));
	sscanf(p,"%*s %s",str);
	gLowerCaseString(str);
	if((strcmp(str,"?")==0)||(strlen(str)<=0))
	{
		printfx("\r\n");
		printfx((char *)params.PN);
		printfx("\r\n"); 
	}
	else if(strcmp(str,"w")==0)
	{
		memset(str,0,sizeof(str));
		sscanf(p,"%*s %*s %[^'\']",str);
		k = strlen(str);			
		if((k > 0)&&(k <= 16))
		{
			memset(params.PN,0,17);
			memcpy(params.PN,str,k);
			W25Q128_SaveParamet(FLASH_SECTION_PARAMETER,(uint8_t*)&params);//SaveParamet();
			printfx(p);
			printfx(" pass\r\n");
		}
		else
		{printfx(p);printfx(" fail\r\n");}  
	}
	else
	{printfx(p);printfx(" fail\r\n");}
	
	return 0;
}

/***************************************************************************
** 函数名称:  	SetPassWord
** 功能描述:  	 
** 输入参数: 	 无
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2020-1-29
** 修改原因：
** 说    明:    
;***************************************************************************/
char ReadWriteBoardSN(void *p,...)
{
	int k=0;
	char str[48]={0};//,str2[17]={0};
	
	sscanf(p,"%*s %s",str);
	gLowerCaseString(str);
	if((strcmp(str,"?")==0)||(strlen(str)<=0))
	{
		printfx("\r\n");
		printfx((char *)params.SN);
		printfx("\r\n"); 
	}
	else if(strcmp(str,"w")==0)
	{
		memset(str,0,sizeof(str));
		sscanf(p,"%*s %*s %[^'\']",str);
		k = strlen(str);			
		if((k > 0)&&(k <16))
		{
			memset(params.SN,0,sizeof(params.SN));
			memcpy(params.SN,str,k);
			W25Q128_SaveParamet(FLASH_SECTION_PARAMETER,(uint8_t*)&params);//SaveParamet();
			printfx(p);
			printfx(" pass\r\n");
		}
		else
		{printfx(p);printfx(" fail\r\n");} 
	}
	else
	{printfx(p);printfx(" fail\r\n");}	
	return 0;
}

/***************************************************************************
** 函数名称:  	
** 功能描述:  	 
** 输入参数: 	 无
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2019-9-16
** 修改原因：
** 说    明:    字数--ASCII;  0--30; 9--39;
;***************************************************************************/
char ReadSelectSwitch(void *p,...)
{
	int s;
    int ret = sscanf(p, "%*s %d", &s);
    if (ret == 1) 
    {
       if(s <= params.nCH)
        {
            DriverError err = move_to_point((PositionPoint)s);
            if (err != DRV_OK) 
            {
                sprintfx("%s %d", (char*)p, err);
                printfx("fail\r\n");
            }
            else
            {
//                sprintfx("%s",(char*)p);
                printfx("pass\r\n");
            }
            
        }
        else
        {
            sprintfx("%s",(char*)p);
            printfx("fail\r\n");
        }
    } 
    else 
    {
        sprintfx("\r\nCHAN:%02d\r\n",motor_status.channelNo);
    }
	
	return 0;
}

/***************************************************************************
** 函数名称:  	ReadParameter
** 功能描述:  	 
** 输入参数: 	 无
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2022-3-1
** 修改原因：
** 说    明:    字数--ASCII;  0--30; 9--39;
;***************************************************************************/
char ReadWriteMaxCH(void *p,...)
{
	int k=0;
	char str[24]={0};
	
	memset(str,0,sizeof(str));
	sscanf(p,"%*s %s",str);	
	gLowerCaseString(str);
	if((strcmp(str,"?")==0)||(strlen(str)<=0))
	{
		sprintfx("\r\nmax:%d\r\n",params.nCH);
	}
	else if(strcmp(str,"w")==0)
	{
		sscanf(p,"%*s %*s %d",&k);
		if(k <= MAX_POINTS)
		{
			params.nCH = k;
			W25Q128_SaveParamet(FLASH_SECTION_PARAMETER,(uint8_t*)&params);//SaveParamet();
			printfx(p);
			printfx(" pass\r\n");
		}
		else 
		{printfx(p);printfx(" fail\r\n");}			
	}	
	else 
	{printfx(p);printfx(" fail\r\n");}
	
	return 0;
}

/***************************************************************************
** 函数名称:  	
** 功能描述:  	 
** 输入参数: 	 无
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2019-9-16
** 修改原因：
** 说    明:    字数--ASCII;  0--30; 9--39;
;***************************************************************************/
char LevelValueDispose(void *p,...)
{
//lvd r 1 ?áí¨μà?ù×??μ
//lvd w 1 756  D′í¨μà?ù×??μ
//lvd s 0 max D′è?°?±?,×?′ó?￠2??μ,ê±??,lvd s 1 ?áè?
//lvd c 1 ??3yí¨μà?ù×??μ
//lvd ? 1 ?ì2é?3í¨μàê?·?ò?D￡×?
//    int32_t val;
    uint32_t sumsteps;
    uint32_t dir;
    int i=0;
    int chn=0;
    char str[4]={0,0,0,0};//,str1[128]={0};
    uint32_t buff[4]={0};
    uint32_t data;
    sscanf(p,"%*s %s %d",str,&chn);
    gLowerCaseString(str);
    if (strcmp(str,"?")==0)
    {
        if (chn==0)
        {
            for(i=1;i<=params.nCH;i++)
            {
               if (position_config.points[i].calibrated)
                   sprintfx("%d:TRUE:%d \r\n",i,position_config.points[i].step_count);
               else
                   sprintfx("%d:FALSE:%d \r\n",i,position_config.points[i].step_count);
            }
             printfx(" pass\r\n");
        }
        else if(chn <= params.nCH)
        {
            if (position_config.points[chn].calibrated)
               sprintfx("%d:TRUE:%d \r\n",chn,position_config.points[chn].step_count);
           else
               sprintfx("%d:FALSE:%d \r\n",chn,position_config.points[chn].step_count);
           printfx(" pass\r\n");
        }
        else
        {
              sprintfx("%s",(char*)p);
              printfx(" fail\r\n"); 
        }
    }
    else if(strcmp(str,"w") == 0)
    {
        if(chn <= params.nCH)
        {
            
            sscanf(p,"%*s %*s %*s %d",&data);
            if (data>=motor_status.position) //?y×a
            {
                sumsteps=data-motor_status.position;
                dir=DIR_CW;
            }
            else //·′×a
            {
                sumsteps=motor_status.position-data;
                dir=DIR_CCW;
            }
            if ((sumsteps>=1)&&(sumsteps<=position_config.max_steps_num))
            {

//                tmc2209_move_steps(sumsteps, MOVE_SPEED, (motor_direction_t)dir);
				wait_for_compensation_cnt = 0;	
				tmc2209_move_steps_ramp(sumsteps, MOVE_SPEED, (motor_direction_t)dir);
//				tmc2209_calculate_deviation();
            }
            position_config.points[chn].step_count = data;
            position_config.points[chn].calibrated = true;
            position_config.points[chn].calibration_time = 0;
            save_config_to_flash();
            printfx(" pass\r\n");
        }
        else
        {
            sprintfx("%s",(char*)p);
            printfx(" fail\r\n"); 
        }
    }
    else if(strcmp(str,"r")==0)
    {
        if (chn==0)
        {
            for(i=1;i<=params.nCH;i++)
            {
                if (position_config.points[i].calibrated)
                {
                    sprintfx("\r\n%d:%d\r\n",i,position_config.points[i].step_count);  
                }
            }
            printfx("\r\npass\r\n");
        }
        else if(chn <=params.nCH)
        {
            sprintfx("%d:%d pass\r\n",chn,position_config.points[chn].step_count);
        }
        else
        {
            sprintfx("%s",(char*)p);
            printfx(" fail\r\n");
        }
    }
    else if(strcmp(str,"s")==0)
    {
        if (chn==0) // 设置马达的最大步数
        {
            //sscanf(p,"%*s %*s %*s %d %d %d",&buff[0],&buff[1],&buff[2]);
            sscanf(p,"%*s %*s %*s %d",&buff[1]);
            position_config.version=1;//buff[0];
            position_config.max_steps_num=buff[1];
            position_config.last_save_time=1;//buff[2];
            save_config_to_flash();
            sprintfx("%s",(char*)p);
            printfx(" pass\r\n");
        }
        else if (chn==1) // 读取马达的最大步数
        {
            sprintfx("max_steps:%d ",position_config.max_steps_num);
            printfx("pass\r\n");
        }
        else
        {
            sprintfx("%s",(char*)p);
            printfx(" fail\r\n");
        }
    }
    else if(strcmp(str,"c")==0)
    {
        if (chn==0)
        {
            clear_calibration();
            sprintfx("%s",(char*)p);
            printfx(" pass\r\n");
        }
        else if(chn <=params.nCH)
        {
            position_config.points[chn].calibrated = false;
            position_config.points[chn].step_count = 0;
            position_config.points[chn].calibration_time = 0;
            save_position_config();
            sprintfx("%s",(char*)p);
            printfx(" pass\r\n");
        }
        else
        {
            sprintfx("%s",(char*)p);
            printfx(" fail\r\n");
        }
    }
    else
    {
        sprintfx("%s",(char*)p);
        printfx(" fail\r\n");
    }
    return 0;
}
/***************************************************************************
** 函数名称:  	OffsetChannel
** 功能描述:  	 
** 输入参数: 	 
** 返 回 值: 	 
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2022-5-26
** 修改原因：
** 说    明:   字数--ASCII;  0--30; 9--39;
;***************************************************************************/
char OffsetChannel(void *p,...)
{
    int sumsteps;
//    uint32_t steps;
	uint32_t step;
//    uint32_t freq;
    uint32_t dir;
	int ret;
   // int s;
    ret = sscanf(p,"%*s %d %d",&step,&dir);
    if (ret == 2)
    {
//        sprintfx("ret:%d\r\n",ret);
//        sprintfx("step:%d,freq:%d,dir:%d\r\n",step,freq,dir);
//        steps=get_point_steps(motor_status.channelNo);
//        sprintfx("steps:%d,direction:%d\r\n",steps,motor_status.direction);
        if (motor_busy)
        {
            sprintfx("%s",(char*)p);
            printfx("motor_busy fail\r\n");
            return 1;
        }
        if (step <= position_config.max_steps_num)
        {
            // sprintfx("motor_status.position:%d\r\n",motor_status.position);
            if (motor_status.direction == dir)//·??ò?àí?
            {
               // printfx("í??ò\r\n");
                if (dir == DIR_CW) //?y×a
                {
                    sumsteps = motor_status.position + step;
                }
                else //·′×a
                {
                    sumsteps = motor_status.position - step;
                }
                if ((sumsteps >= 1)&&(sumsteps <= position_config.max_steps_num))
                {
                    //sprintfx("pass:%d\r\n",sumsteps);
//                    tmc2209_move_steps(step, MOVE_SPEED, (motor_direction_t)dir);
					wait_for_compensation_cnt = 0;
					tmc2209_move_steps_ramp(step, MOVE_SPEED, (motor_direction_t)dir);
//                    printfx(p);
					printfx(" pass\r\n");
//					tmc2209_calculate_deviation();
                }
                else
                {
                    sprintfx("fail:%d\r\n",sumsteps);
                    printfx(p);printfx(" fail\r\n");
                }
            }
            else
            {
               // printfx("·′?ò\r\n");
                if (dir==DIR_CW) //?y×a
                {
                    sumsteps = motor_status.position + step;
                }
                else //·′×a
                {
                    sumsteps = motor_status.position - step;
                }
                if ((sumsteps >= 1)&&(sumsteps <= position_config.max_steps_num))
                {                    
                    //sprintfx("pass:%d\r\n",sumsteps);
//                    tmc2209_move_steps(step, MOVE_SPEED, (motor_direction_t)dir);
					wait_for_compensation_cnt = 0;		
					tmc2209_move_steps_ramp(step, MOVE_SPEED, (motor_direction_t)dir);
//                    printfx(p);
					printfx(" pass\r\n");
//					tmc2209_calculate_deviation();
                }
                else
                {
                    //sprintfx("fail:%d\r\n",sumsteps);
                    printfx(p);printfx(" fail\r\n");
                }
            }        
        }
        else
        {
             sprintfx("%s",(char*)p);
             printfx("para step fail\r\n");
             sprintfx("max step :%d\r\n",position_config.max_steps_num);
        }
    }
	else if (ret == 1)
	{
		sumsteps = motor_status.position + step;
		if(step == 0)
		{
			motor_status.homing = 0;
			ret = move_to_home();
			if(ret != DRV_OK)
			{
				sprintfx("Homeing failed\r\n");
				tmc2209_disable_motor();
			}
			else 
			{
//				printfx("Homeing success\r\n");
				printfx(" pass\r\n");
			}
		}
		else if(sumsteps <= position_config.max_steps_num)
		{
			wait_for_compensation_cnt = 0;		
			tmc2209_move_steps_ramp(step, MOVE_SPEED, (motor_direction_t)dir);
			printfx(p);printfx(" pass\r\n");
//			tmc2209_calculate_deviation();
		}
		else
		{
			sprintfx("%s",(char*)p);
			printfx(" fail\r\n");
		}
	}
    else
    {
        sprintfx("steps:%d\r\n",motor_status.position);
        printfx("steps pass\r\n");
    }
	return 0;
}
/***************************************************************************
** 函数名称:  	
** 功能描述:  	 
** 输入参数: 	 无
** 返 回 值: 	 无
** 全局变量: 
** 调用模块: 
** 作　  者:   M-L-A
** 日　  期:   2022-5-26
** 修改原因：
** 说    明:    字数--ASCII;  0--30; 9--39;
;***************************************************************************/
char SetReadWrite(void *p,...)
{
	char str1[24]={0},str2[24]={0};
	int k=0;
	
	memset(str1,0,sizeof(str1));
	memset(str2,0,sizeof(str2));
    sscanf(p,"%*s %s %s",str1,str2);

	gLowerCaseString(str1);	
	gLowerCaseString(str2);
	if(strcmp(str1,"w")==0)
	{
		if(strcmp(str2,"ls")==0)
		{
			save_config_to_flash();
			printfx(p);
			printfx(" pass\r\n");
		}
		else if(strcmp(str2,"bs")==0)
		{
			sscanf(p,"%*s %*s %*s %d",&k);
			switch(k)
			{
				case 0:break;
				case 1:BOOST_SHDN(0);break; //BOOST OFF
				case 2:BOOST_SHDN(1);break;	//BOOST ON
				case 3:BOOST_FSET(0);break; //boost 650khz
				case 4:BOOST_FSET(1);break; //boost 1.1mhz
			}
			printfx(p);
			if(k>0)
			{printfx(" pass\r\n");}
			else
			{printfx(" fail\r\n");}
		}	
		else if(strcmp(str2,"pps")==0)
		{//Parallel port strobe
			memset(str1,0,sizeof(str1));
			sscanf(p,"%*s %*s %*s %d",&k);
			if(k>0)
			{
				params.pps=k;
				W25Q128_SaveParamet(FLASH_SECTION_PARAMETER,(uint8_t*)&params);//SaveParamet();
				printfx(p);printfx(" pass\r\n");
			}
			else
			{printfx(p);printfx(" fail\r\n");}
		}
		else
		{printfx(p);printfx(" fail\r\n");}
	}
	else if(strcmp(str1,"r")==0)
	{
		if(strcmp(str2,"ec")==0)
		{
			sprintfx("EC %d\r\n",erCode.regis);
		}
		else if(strcmp(str2,"temp")==0)
		{//temperature
			sprintfx("temp %0.2f\r\n",sTempere.tValue);
		}
		else if(strcmp(str2,"pps")==0)
		{//Parallel port strobe
			sprintfx("PPS 0x%02X\r\n",params.pps);
		}
		else
		{printfx(p);printfx(" fail\r\n");}  
	}	
	else
	{printfx(p);printfx(" fail\r\n");}  
	return 0;
}

char ResetDev(void *p,...)
{
    char str1[24]={0},str2[24]={0};
    int ret = sscanf(p,"%*s %s %s",str1,str2);
	memset(str1,0,sizeof(str1));
	memset(str2,0,sizeof(str2));   
   	gLowerCaseString(str1);	
	gLowerCaseString(str2);
    if (ret==0)
    {
        printfx("<RESET_OK>");
        delay_ms(50);
        NVIC_SystemReset();
    }
    else
    {
        if((strcmp(str1,"w")==0)&&(strcmp(str2,"ls")==0))
        {
            save_config_to_flash();
            printfx(p);printfx(" pass\r\n");
        }
        else if(strcmp(str1,"r")==0)
        {
            if(strcmp(str2,"ec")==0)
            {
                sprintfx("EC %d\r\n",erCode.regis);
            }
            else if(strcmp(str2,"temp")==0)
            {//temperature
                sprintfx("temp %0.2f\r\n",sTempere.tValue);
            }
            else if(strcmp(str2,"pps")==0)
            {//Parallel port strobe
                sprintfx("PPS 0x%02X\r\n",params.pps);
            }
            else
            {printfx(p);printfx(" fail\r\n");}  
        }
        else
        {
            printfx(p);printfx(" fail\r\n");
        }
    }
    return 0;
}

