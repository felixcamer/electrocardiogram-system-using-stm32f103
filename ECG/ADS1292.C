#include "ADS1292.h"
//#include "ADS1x9x_ECG_Processing.h"
#include "spi.h"
#include "delay.h"
#include "Timer.h"
#include "lcd.h"
#include "spi.h"
#include "delay.h"


#define DEBUG_ADS1292	//printf����


// ADS_DRDY	  PAin(4)
// ADS_CS			PCout(3)		
// ADS_RESET 	PCout(1)
// ADS_START		PCout(2)	//CONNECT TO  3.3V




u8 ADS1292_REG[12];		//ads1292�Ĵ�������
ADS1292_CONFIG1 	Ads1292_Config1		={DATA_RATE};																				//CONFIG1
ADS1292_CONFIG2 	Ads1292_Config2		={PDB_LOFF_COMP,PDB_REFBUF,VREF,CLK_EN,INT_TEST};		//CONFIG2
ADS1292_CHSET 		Ads1292_Ch1set		={CNNNLE1_POWER,CNNNLE1_GAIN,CNNNLE1_MUX};					//CH1SET
ADS1292_CHSET 		Ads1292_Ch2set		={CNNNLE2_POWER,CNNNLE2_GAIN,CNNNLE2_MUX};					//CH2SET
ADS1292_RLD_SENS	Ads1292_Rld_Sens	={PDB_RLD,RLD_LOFF_SENSE,RLD2N,RLD2P,RLD1N,RLD1P};	//RLD_SENS
ADS1292_LOFF_SENS	Ads1292_Loff_Sens	={FLIP2,FLIP1,LOFF2N,LOFF2P,LOFF1N,LOFF1P};					//LOFF_SENS
ADS1292_RESP1			Ads1292_Resp1			={RESP_DEMOD_EN1,RESP_MOD_EN,RESP_PH,RESP_CTRL};		//RSP1
ADS1292_RESP2			Ads1292_Resp2			={CALIB,FREQ,RLDREF_INT};														//RSP2



//ADS1292��IO�ڳ�ʼ��	
void ADS1292_Init(void) 
{		
		GPIO_InitTypeDef 	GPIO_InitStructure;	
		EXTI_InitTypeDef 	EXTI_InitStructure;
		NVIC_InitTypeDef	NVIC_InitStructure;
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE); 	 
	 //SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
		SPI1_Init();//��ʼ��SPI����	
									//250ns Ƶ��4.5M	���Ͱ���ʱ����Ҫ23 us	
									//125ns Ƶ��9M��		���Ͱ���ʱ����Ҫ14 us	
									//55ns Ƶ��18M		���Ͱ���ʱ����Ҫ9.2us
									//30ns 36M				���Ͱ���ʱ����Ҫ9.2us
									//�ֲ�10ҳ��д����Сʱ������Ϊ50ns			
	  //DRDY
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//��������
		GPIO_Init(GPIOA, &GPIO_InitStructure);	
  
		
		//CS
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//�������
		GPIO_Init(GPIOC, &GPIO_InitStructure);	
		
		
		//RESRT
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	  GPIO_Init(GPIOC, &GPIO_InitStructure);	
		GPIO_SetBits(GPIOC,GPIO_Pin_1);
		
		
		//START
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOC, &GPIO_InitStructure);	
		GPIO_ResetBits(GPIOC,GPIO_Pin_2);
		

		//DRDY�жϳ�ʼ��
	  EXTI_ClearITPendingBit(EXTI_Line4);//����жϱ�־
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource4);//ѡ��ܽ�
  	EXTI_InitStructure.EXTI_Line=EXTI_Line4;						 //ѡ���ж���·
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	 //����Ϊ�ж����󣬷��¼�����
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½��ش���
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;						 //�ⲿ�ж�ʹ��
  	EXTI_Init(&EXTI_InitStructure);	 
		
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;					//ѡ���ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//��ռ���ȼ� 2
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;				//�����ȼ�   1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;						//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure); 
		
		
		//EXTI0_Init();
		//EXTI->IMR &= ~(EXTI_Line4);//�����ⲿ�ж�4	
		ADS_CS=1;	
		ADS1292_PowerOnInit();//�ϵ縴λ���������ģʽ	
		
				while(Set_ADS1292_Collect(0))//0 �����ɼ� //1 1mV 1Hz�ڲ������ź�
		{	
			printf("1292 reg err\r\n");
		
			delay_ms(1000);			
		}	
		stop_ads1292();
			
}


volatile u8 ads1292_recive_flag=0;	//���ݶ�ȡ��ɱ�־
volatile u8 ads1292_Cache[9];	//���ݻ�����
volatile u32  point_ = 0;
volatile u8  QRS_Heart_Rate1 = 0;
volatile u8  HR_HRAvg_disp_flag = 0; //display the hr, hravg etc...
volatile u8  ecg_tab[3750]={0};  //store the ecg measured



//��ȡ72λ������1100+LOFF_STAT[4:0]+GPIO[1:0]+13��0+2CHx24λ����9�ֽ�
//	1100	LOFF_STAT[4			3			2			1			0	]	//����������ص���Ϣ��LOFF_STAT�Ĵ�����
//									RLD		1N2N	1N2P	1N1N	1N1P	
//	��	C0 00 00 FF E1 1A FF E1 52	

u8 ADS1292_Read_Data(u8 *data)//72Mʱ���º�����ʱ��Լ10us  8Mʱ���� ������ʱ 100us
{		
		u8 i;	
		
		ADS_CS=0;//��9���ֽڵ�����
		//delay_us(10);
		for(i=0;i<9;i++)
		{	
				*data=ADS1292_SPI(0X00);	
				data++;
		}
		//delay_us(10);
		ADS_CS=1;		
		return 0;
}


//���üĴ�������
void ADS1292_SET_REGBUFF(void)
{
	
	ADS1292_REG[ID] =	ADS1292_DEVICE;//IDֻ��
	 
	ADS1292_REG[CONFIG1] =	0x00;		//0000 0aaa	[7] 0����ת��ģʽ  [6:3] ����Ϊ0 
	ADS1292_REG[CONFIG1] |=	Ads1292_Config1.Data_Rate;//[2:0] aaa ���������ò�����

	ADS1292_REG[CONFIG2] =	0x00;		//1abc d0e1	[7] ����Ϊ1  [2] ����Ϊ0  [0] ���ò����ź�Ϊ1HZ����1mV���� 
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Pdb_Loff_Comp<<6;	//[6]a ��������Ƚ����Ƿ����
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Pdb_Refbuf<<5;		//[5]b �ڲ��ο��������Ƿ����
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Vref<<4;					//[4]c �ڲ��ο���ѹ���ã�Ĭ��2.42V
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Clk_EN<<3;				//[3]d CLK�������ʱ�����壿
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Int_Test<<1;			//[1]e �Ƿ���ڲ������ź�,
	ADS1292_REG[CONFIG2] |=	0x81;//����Ĭ��λ
	
	ADS1292_REG[LOFF] =	0xF4;//[7:5]	���õ�������Ƚ�����ֵ [4]	����Ϊ1 		[3:2] �������������ֵ		[1]	����Ϊ0	[0]	���������ⷽʽ 0 DC 1 AC 

	ADS1292_REG[CH1SET] =	0x00;	 //abbb cccc
	ADS1292_REG[CH1SET] |=Ads1292_Ch1set.PD<<7;		//[7]  a 		ͨ��1�ϵ磿
	ADS1292_REG[CH1SET] |=Ads1292_Ch1set.GAIN<<4;	//[6:4]bbb	����PGA����
	ADS1292_REG[CH1SET] |=Ads1292_Ch1set.MUX;			//[3:0]cccc	����ͨ��1���뷽ʽ

	ADS1292_REG[CH2SET] =	0x00;	//abbb cccc
	ADS1292_REG[CH2SET] |=Ads1292_Ch2set.PD<<7;		//[7]  a 		ͨ��2�ϵ磿
	ADS1292_REG[CH2SET] |=Ads1292_Ch2set.GAIN<<4;	//[6:4]bbb	����PGA����
	ADS1292_REG[CH2SET] |=Ads1292_Ch2set.MUX;			//[3:0]cccc	����ͨ��2���뷽ʽ
	
	ADS1292_REG[RLD_SENS] = 0X00; //00ab cdef	[7:6] 00 PGAն��Ƶ��	fMOD/16 
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Pdb_Rld<<5;					//[5]a	��λ����RLD�����Դ״̬
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld_Loff_Sense<<4;	//[4]b	��λʹ��RLD���������⹦��
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld2N<<3;						//[3]c	���λ����ͨ��2������	�����������������
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld2P<<2;						//[2]d	��λ����ͨ��2������		�����������������
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld1N<<1;						//[1]e	���λ����ͨ��1������	�����������������
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld1P;							//[0]f	��λ����ͨ��1������		�����������������	
	
	ADS1292_REG[LOFF_SENS] = 0X00;  //00ab cdef	[7:6] ����Ϊ0
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Flip2<<5;		//[5]a	���λ���ڿ��Ƶ���������ͨ��2�ĵ����ķ���
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Flip1<<4;		//[4]b	���λ�������ڵ���������ͨ��1�ĵ����ķ���
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Loff2N<<3;	//[3]c	��λ����ͨ��2������˵ĵ���������
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Loff2P<<2;	//[2]d	��λ����ͨ��2������˵ĵ���������
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Loff1N<<1;	//[1]e	��λ����ͨ��1������˵ĵ���������
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Loff1P;			//[0]f	��λ����ͨ��1������˵ĵ���������
	
	ADS1292_REG[LOFF_STAT] =	0x00;		//[6]0 ����fCLK��fMOD֮���ģ��Ƶ�� fCLK=fMOD/4  [4:0]ֻ������������͵缫����״̬
	
	ADS1292_REG[RESP1] = 0X00;//abcc cc1d
	ADS1292_REG[RESP1] |=Ads1292_Resp1.RESP_DemodEN<<7;//[7]a		���λ���úͽ���ͨ��1�ϵĽ����·		
	ADS1292_REG[RESP1] |=Ads1292_Resp1.RESP_modEN<<6;	//[6]b		���λ���úͽ���ͨ��1�ϵĵ��Ƶ�·	
	ADS1292_REG[RESP1] |=Ads1292_Resp1.RESP_ph<<2;			//[5:2]c	��Щλ���ƺ�����������źŵ���λ	
	ADS1292_REG[RESP1] |=Ads1292_Resp1.RESP_Ctrl;			//[0]d		���λ���ú�����·��ģʽ
	ADS1292_REG[RESP1] |=	0x02;//����Ĭ��λ	
	
	ADS1292_REG[RESP2] = 0x00; //a000 0bc1	[6:3]����Ϊ0 [0]����Ϊ1
	ADS1292_REG[RESP2] |=	Ads1292_Resp2.Calib<<7;				//[7]a ����ͨ��ƫ��У����
	ADS1292_REG[RESP2] |=	Ads1292_Resp2.freq<<2;				//[2]b ����Ƶ������
	ADS1292_REG[RESP2] |=	Ads1292_Resp2.Rldref_Int<<1;	//[1]c RLDREF�ź�Դ�ⲿ���磿
	ADS1292_REG[RESP2] |= 0X01;		//����Ĭ��λ	
 
	ADS1292_REG[GPIO] =	0x0C;			//GPIO��Ϊ����		[7:4]����Ϊ0	 [3:2]11 GPIOΪ���� [1:0] ��������ʱ��ָʾ���ŵ�ƽ���������ʱ�������ŵ�ƽ
}

//ͨ��SPI������ADS1292ͨ��
u8 ADS1292_SPI(u8 com)
{	
		return SPI1_ReadWriteByte(com);
}
//д����
void ADS1292_Send_CMD(u8 data)
{
		ADS_CS=0;
		delay_us(10);
		ADS1292_SPI(data);		
		delay_us(10);	
		ADS_CS=1;
}


/*ADS1291��ADS1292��ADS1292R���нӿ����ֽ���ʽ���������Ҫ4��tCLK�����������ִ��.
��ˣ��ڷ��Ͷ��ֽ�����ʱ��4 tCLK���ڱ��뽫һ���ֽ�(�������)�Ľ�������һ���ֽ�(�������)�ֿ���
����CLK��ʱ�ӣ�Ϊ512 kHz����tSDECODE (4 tCLK)Ϊ7.8125 us��
��SCLK���������ʣ�Ϊ16mhzʱ��һ���ֽڿ�����500ns�д��䣬���ֽڴ���ʱ�䲻����tSDECODE�淶;
��ˣ��������һ���ӳ٣��Ա�ڶ����ֽڵ�ĩβ����7.3125us���
���SCLKΪ1 MHz������8u���ڴ���һ���ֽڡ����ڴ˴���ʱ�䳬��tSDECODE�淶�����������Բ��ӳٵط��ͺ����ֽڡ�
�ں���ĳ����У����ԶԴ��ж˿ڽ��б�̣�ʹ���ÿ��ѭ���ĵ��ֽڴ���ת�Ƶ�����ֽ�*/

//��д����Ĵ���
void ADS1292_WR_REGS(u8 reg,u8 len,u8 *data)
{
		u8 i;
		ADS_CS=0;	
		delay_us(10);
		ADS1292_SPI(reg);
		delay_us(10);
		ADS1292_SPI(len-1);
		if(reg&0x40) //д
		{
				for(i=0;i<len;i++)
				{
						delay_us(10);		
						ADS1292_SPI(*data);
						data++;				
				}			
		}
		else //��		
		{
				for(i=0;i<len;i++)
				{
						delay_us(10);		
						*data = ADS1292_SPI(0);
						data++;
				}
		}			
		delay_us(10);	
		ADS_CS=1;
}


//�Ĵ�������д��Ĵ���
u8 ADS1292_WRITE_REGBUFF(void)
{
		u8 i,res=0;
		u8 REG_Cache[12];	//�洢�Ĵ�������
		ADS1292_SET_REGBUFF();//���üĴ�������		
		ADS1292_WR_REGS(WREG|CONFIG1,11,ADS1292_REG+1);//�������д��Ĵ���
		delay_ms(1);		
		ADS1292_WR_REGS(RREG|ID,12,REG_Cache);//���Ĵ���
		delay_ms(1);	
		
	#ifdef DEBUG_ADS1292	
		printf("WRITE REG:\r\n");
		for(i=0;i<12;i++	)//Ҫд������								
				printf("%d %x\r\n",i,ADS1292_REG[i]);	
		printf("READ REG:\r\n");
	#endif	
	
		for(i=0;i<12;i++	)	//���Ĵ���	
		{						
				if(ADS1292_REG[i] != REG_Cache[i])
				{
						if(i!= 0 && i!=8 && i != 11)	//0 8 ��11��ID ���������GPIO���
								res=1;
						else
								continue;
				}					
			#ifdef DEBUG_ADS1292
				printf("%d %x\r\n",i,REG_Cache[i]); //����������			
			#endif
		}	
		#ifdef DEBUG_ADS1292	
			if(res == 0)
					printf("REG write success\r\n");
			else		
					printf("REG write err\r\n");
		#endif
		return res;				
}


void ADS1292_PowerOnInit(void)
{	
		u8 i;
		u8 REG_Cache[12];	
	
//		ADS_CLKSEL=1;//�����ڲ�ʱ��
		ADS_START=0; //ֹͣ�������	
		ADS_RESET=0; //��λ
		delay_ms(1000);
		ADS_RESET=1;//оƬ�ϵ磬����ʹ��	
		delay_ms(100);	//�ȴ��ȶ�
	  
		ADS1292_Send_CMD(SDATAC);//����ֹͣ������ȡ��������
		delay_ms(100);	
		ADS1292_Send_CMD(RESET);//��λ
		delay_ms(1000);		
		ADS1292_Send_CMD(SDATAC);//����ֹͣ������ȡ��������
		delay_ms(100);		
	
#ifdef DEBUG_ADS1292	
		ADS1292_WR_REGS(RREG|ID,12,REG_Cache);
		printf("read default REG:\r\n");
		for(i=0;i<12;i++	)	//��Ĭ�ϼĴ���
				printf("%d %x\r\n",i,REG_Cache[i]);		
#endif
		ADS1292_Send_CMD(STANDBY);//�������ģʽ	
}

//����ͨ��1�ڲ�1mV�����ź�
u8 ADS1292_Single_Test(void) //ע��1292R���˺�����������ͨ��һ���ڲ������źŲ������Ӱ�죬����ֻ�ο�ͨ��2���ɣ�1292����Ӱ��
{
		
		u8 res=0;
		Ads1292_Config2.Int_Test = INT_TEST_ON;//���ڲ������ź�
		Ads1292_Ch2set.MUX=MUX_Test_signal;//�����ź�����	
		//Ads1292_Ch2set.MUX=MUX_VDD_signal;//�����ź�����	
		if(ADS1292_WRITE_REGBUFF())//д��Ĵ���
				res=1;	
		delay_ms(1);			
		return res;			
}
//�����źŲɼ�ģʽ
u8 ADS1292_Single_Read(void)
{
	
	  /*define	PDB_RLD						PDB_RLD_ON			//RLD�����Դ		
		#define	RLD_LOFF_SENSE		RLD_LOFF_SENSE_OFF	//RLD�������书�ܣ����Ե�ʱ�������ȵ���������������������������ͬʱ������
		#define	RLD2N							RLD_CANNLE_ON			//ͨ���������������
		#define	RLD2P							RLD_CANNLE_ON
		#define	RLD1N							RLD_CANNLE_OFF
		#define	RLD1P							RLD_CANNLE_OFF*/
		u8 res=0;
		Ads1292_Config2.Int_Test = INT_TEST_OFF;//���ڲ������ź�
		Ads1292_Ch1set.MUX = MUX_Normal_input;//��ͨ�缫����
		Ads1292_Ch2set.MUX = MUX_Normal_input;//��ͨ�缫����
	
		if(ADS1292_WRITE_REGBUFF())//д��Ĵ���
				res=1;
		delay_ms(1);		
		return res;		
}	

//����ads1292�ɼ���ʽ
u8 Set_ADS1292_Collect(u8 mode)
{
		u8 res;
		ADS1292_Send_CMD(WAKEUP);//����
		delay_ms(1);		
		switch(mode)//���òɼ���ʽ
		{
				case 0:
					res =ADS1292_Single_Read();												
				break;
				case 1:
					res =ADS1292_Single_Test();											
				break;
		}		
		if(res)return 1;//�Ĵ�������ʧ��		
		ADS1292_Send_CMD(RDATAC); //��������ģʽ
		delay_ms(1);		
		//ADS1292_Send_CMD(START);	//���Ϳ�ʼ����ת������Ч������START���ţ�	
		//delay_ms(1);		
		return 0;
}


void stop_ads1292(){

	 ADS_CS=0;

	ADS1292_Send_CMD(STOP);	//���Ϳ�ʼ����ת������Ч������START���ţ�	
	delay_ms(1);	
  ADS1292_Send_CMD(STANDBY);
  delay_ms(1);
	ADS_CS=1;
}


void start_ads1292(){

	ADS_CS=0;
	ADS1292_Send_CMD(WAKEUP);//����
	delay_ms(1);	

	//ADS1292_Single_Read();	
	ADS1292_Send_CMD(START);	//���Ϳ�ʼ����ת������Ч������START���ţ�	
	delay_ms(1);	
  ADS_CS=1;	
}

s32 get_volt(u32 num)
{		
			s32 temp;			
			temp = num;
			temp <<= 8;
			temp >>= 8;
			return temp;
}


