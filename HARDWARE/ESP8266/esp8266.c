#include "esp8266.h"
#include "app.h"

const char *ssid = "ESP_Exclusive";
const char *password = "17041126";

u16 wifi_status;
u16 wifi_err;




void ESP_At_Response(u8 mode)
{
	if(USART2_RX_STA & 0X8000) {	//���յ�һ��������
		USART2_RX_BUF[USART2_RX_STA & 0X3FFF] = 0; //��ӽ�����
		printf("%s", USART2_RX_BUF);	//���͵�����
		if(mode)
			USART2_RX_STA = 0;
	}
}

void ESP_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	DBGMCU->CR  &= ~((uint32_t)1<<5);    
	AFIO->MAPR = (AFIO->MAPR & ~((uint32_t)0x7 << 24)) | (2 << 24);     /*  PA15 PB3 PB4 */

	/*EN*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOB, &GPIO_InitStructure);		
	
	/*STATE*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB,GPIO_Pin_3);	
}

void ESP_Init(void)
{
	u8 temp;
	ESP8266_QuitTrans();
	while(ESP8266_SendCmd((u8*)AT, (u8*)answer1))        //ͨ������3����ATָ��
	{
		temp++;
		if(temp == 5)
		{
			wifi_err = 1;
			temp = 0;
			break;
		} 
	}	
	temp = 0;
	delay_ms(500);
	while(ESP8266_SendCmd((u8*)CWMODE_TCP,(u8*)answer1))     //ͨ������3����ATָ��
	delay_ms(500);
	while(ESP8266_SendCmd((u8*)RST,(u8*)answer1))        //ͨ������3����ATָ��
	delay_ms(500);
	while(ESP8266_SendCmd((u8*)CWSAP,(u8*)answer1))      //ͨ������3����ATָ��
	delay_ms(1000);
	wifi_status = 1;
}

void ESP_TCP(void)
{
	u8 temp;
	wifi_status = 2;
	wifi_err = 0;
	while(ESP8266_SendCmd((u8*)AT, (u8*)answer1))//ͨ������3����ATָ��	       
	delay_ms(500);
	while(ESP8266_SendCmd((u8*)CWSAP,(u8*)answer1))      //ͨ������3����ATָ��
	delay_ms(1000);
	while(ESP8266_SendCmd((u8*)CIFSR,""))      //ͨ������3����ATָ��
	delay_ms(500);
	while(ESP8266_SendCmd((u8*)CIPSTARTPC,"CONNECT")){
		temp++;
		if(temp == 30)
		{
			wifi_err = 2;
			temp = 0;
			break;
		}	
	}   //ͨ������3����ATָ��
	temp = 0;
	delay_ms(500);
	if(wifi_err == 0){
		ESP8266_SendCmd("AT+CIPMODE=1\r\n", "OK");//ʹ��͸��
		delay_ms(200); 
		ESP8266_SendCmd("AT+CIPSEND\r\n", ">");  //ͨ������2����ATָ��
		wifi_status = 3;
	}
}


void ESP_TCP_quit(void)
{
  delay_ms(500); 
	while((USART2->SR & 0X40) == 0);	//�ȴ����Ϳ�
	USART2->DR = '+';
	delay_ms(15); 					//���ڴ�����֡ʱ��(10ms)
	
	while((USART2->SR & 0X40) == 0);	//�ȴ����Ϳ�
	USART2->DR = '+';        
	delay_ms(15); 					//���ڴ�����֡ʱ��(10ms)
	
	while((USART2->SR & 0X40) == 0);	//�ȴ����Ϳ�
	USART2->DR = '+';        
	delay_ms(500); 					//���ڴ�����֡ʱ��(10ms)

	usartx_puchar(USART1,"+++\r\n",5);  //��������ã�ͨ������1�鿴
	
	wifi_status = 4;
}


void ESP_Enter_Trans(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_3);
	while(!ESP_Send_Cmd("AT\r\n", "OK", 100));
	{
		wifi_status = 0;
	}
	wifi_status = 1;
}

uint8_t ESP_Send_Cmd(u8 *cmd, u8 *ack, u16 waittime)
{
	USART2_RX_STA = 0;
	printf("%s", cmd);	
	USART2_SendString(cmd);
	if(ack && waittime) {					//��Ҫ�ȴ�Ӧ��
		while(--waittime) {				//�ȴ�����ʱ
			delay_ms(10);
			if(USART2_RX_STA & 0X8000) { 	//���յ��ڴ���Ӧ����
				if(ESP_Check_Cmd(ack)) {
					ESP_At_Response(1);   	//��ӡATָ�����Ϣ
					return 1;		//�õ���Ч����
				}
				else
					USART2_RX_STA = 0;
			}
		}
		if(waittime == 0)
			return 0;
	} else;
	return 1;
}



u8* ESP_Check_Cmd(u8 *str)
{
	char *strx = NULL;
	if(USART2_RX_STA & 0X8000) {	//���յ�һ��������
		USART2_RX_BUF[USART2_RX_STA & 0X3FFF] = 0; //��ӽ�����
		strx = strstr((const char*)USART2_RX_BUF, (const char*)str);
	}
	return (u8*)strx;
}
