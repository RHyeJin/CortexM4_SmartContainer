
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx.h"
#include "uart2.h"
#include "uart4.h"
#include "adc1.h"
#include "tim10_motor.h"
#include "tim4_counter.h"
#include "key.h"
#include "lcd.h"
#include "led.h"

/******************************************************************************
* General-purpose timers TIM10

  포트 연결:

  PA0,PA1 : UART4 RX,TX
  PA2,PA3 : UART2 RX,TX
//  PA8 :  M_SEN(Motor Sensor)

  PA5 ~ PA12 : CLCD D0~D7
  PB0 ~ PB2  : CLCD RS, RW, E

   PB8  : DC Motor M_EN
   PB9  : DC Motor M_D2
   PB10 : DC Motor M_D1

   PC12~PC15 ==> Button0 ~ 3 

******************************************************************************/
#define CMD_SIZE 50
#define ARR_CNT 9  
#define PORTC_FND 

volatile int finFlag = 0;
volatile int clkFlag = 0;
volatile int pwm = 50;
volatile int counter = 0;
volatile int sendFlag=0;
volatile int limFlag = 0;
volatile int humFlag = 0;
volatile int temFlag =0;
char Line1 [20] = {0};
char Line2 [20] = {0};
char erase[20];
char setTime [20] = {0};
char aryt[20] = {0};
int totalSec = 0;
double progress = 0;
int basedist = 14;
int curSec = 0;
int limIllu = 0;
int illu = 0, dis = 0;
int curillu = 0,curdis = 0;
volatile int progFlag = 0;
double curtemp = 0, curhumi = 0;
double temp = 0, humi = 0;
double limTemp = 0, limHumi = 0;
extern uint16_t adc_data;
extern volatile int fndNumber;
extern volatile int adc1Flag;
extern volatile unsigned long systick_sec;            //1sec
extern volatile int systick_secFlag;
extern volatile unsigned char rx2Flag;
extern char rx2Data[50];
extern volatile unsigned char rx4Flag;
extern char rx4Data[50];
extern int key;
extern unsigned int tim1_counter;
long map(long x, long in_min, long in_max, long out_min, long out_max);
void Motor_Right();
void Motor_Left();
void Motor_Stop();
void Motor_Pwm(int val);
typedef struct{
  int msec;
  int sec;
  int min;
  int hour;
}TIME;

TIME time;

char ctime[20];
char pretime[20];
int t_cnt = 0;

void clock_display(){
  sprintf(ctime,"%02d:%02d:%02d",time.hour,time.min,time.sec);
  strcpy(pretime,ctime);
  if(!strcmp(ctime,pretime)){
    lcd(0,1,ctime);
    sprintf(Line1, "Progress: %.1lf%%",progress*100);
    lcd(0,0,Line1);
  }
}

void clock_calc(){
  if(clkFlag == 1){
    if(t_cnt == 10){
      t_cnt = 0;
      progFlag=1;
      printf("pr: %.1lf, tem:%.2lf,cur = %d, tot = %d, tc = %d \r\n",progress*100, temp,curSec,totalSec,t_cnt);    
      curSec ++;
      progress = (double)curSec/totalSec;
      time.sec--;
    }
    else if(time.sec == 0){
      time.sec = 59;
      if(time.min > 0)
      time.min--;
      else{
        time.min =59;
        if(time.hour > 0)
          time.hour --;      
      }
    }  
  }
}

void TIM7_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
  {    
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    time.msec++;
    if(time.msec == 100){
        t_cnt++ ; //100ms
        time.msec = 0;
      }
    clock_calc();
    if(time.hour == 0 && time.min == 0 && time.sec == 0 && clkFlag == 1){    
      lcd(0,0,erase);
      lcd(0,1,erase);
      lcd(0,0,"Progress: 100.0%");
      lcd(0,1,"Timer Finished!!");
      Serial4_Send_String("[AND]PROGRESS@100\r\n");
      finFlag=1;
      clkFlag=0;
      progFlag =0;
    } 
  }
}

int main()
{
  int old_pwm=50;
  int adc_pwm=50;
  int ccr1;

  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef   NVIC_InitStructure; 
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  TIM_TimeBaseStructure.TIM_Prescaler = 84-1;         //(168Mhz/2)/840 = 0.1MHz(10us)
  TIM_TimeBaseStructure.TIM_Period = 1000-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
  
  TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
  TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM7, ENABLE);
  
  Key_Init();

#ifdef PORTC_FND
#else
   PORTC_Led_Init();  
#endif  
  UART2_init();
  UART4_init();
  TIM10_Motor_Init();
  TIM1_Counter_Init();        //PORTA  사용
  GPIOAB_LCD_Init();
  lcd(0,0,"  C    M    P");    // 문자열 출력
  lcd(0,1," P R O J E C T");    // 문자열 출력

  Serial2_Send_String("start main()\r\n");

  while(1)
  { 
    if(finFlag){
        Serial4_Send_String("[FCM]Timer_Finished\r\n");
        finFlag=0;
    }
    if(limFlag==1){
      printf("LIMFLAG ON, curhumi %.1lf, curtemp %.1lf, curillu %d, limhumi %.1lf",curhumi,curtemp,curillu,limHumi);
      if((curhumi > limHumi) && (limHumi != 0)){
        Motor_Left();
        Serial4_Send_String("[FCM]High_Humidity\r\n");
        humFlag =1;
        limFlag =0;
      }
      if(curtemp > limTemp && limTemp != 0){
        Motor_Left();
        Serial4_Send_String("[FCM]High_Temperature\r\n");
        limFlag =0;
        temFlag=1;
      }
      if(curillu > limIllu && limIllu != 0){
        Serial4_Send_String("[FCM]High_Illumination\r\n");
        limFlag =0;
      }
    }
    if(curhumi < limHumi && limFlag == 0 && humFlag ==1){
      Motor_Stop();
      limFlag=1;
    }
    if(curtemp < limTemp && limFlag == 0 && temFlag == 1){
      Motor_Stop();
      limFlag=1;
    }
    if(clkFlag){
      sprintf(aryt,"%.1lf",progress*100);
      if(progFlag){
        Serial4_Send_String("[AND]PROGRESS@");
        Serial4_Send_String(aryt);
        Serial4_Send_String("\r\n");
        progFlag = 0;
      }
    }
    if(sendFlag){
      Serial4_Send_String("[SQL]GETSENSOR\r\n");
      sendFlag=0;
    }
    if(rx2Flag)
    {
      printf("rx2Data : %s\r\n",rx2Data);
      rx2Flag = 0;
    }  
    if(rx4Flag)
    {
      Serial4_Event();
      rx4Flag = 0;
    }
    if(key != 0)
    {
      printf("Key : %d  \r\n",key);
      if(key == 1) //check time //progress percentage
      { 
        if(clkFlag == 1){
          clock_display();
        }
       }
      else if(key == 2) //senser output display
      {
        sprintf(Line1,"Volume : %.1lf%%",((double)(basedist-curdis)/basedist)*100);
         if(clkFlag == 1){
            lcd(0,0,erase);
            lcd(0,1,erase);
            lcd(0,0,Line1);
            lcd(0,1,setTime);
            key=0;
         }
         else{
            clrscr();
            lcd(0,0,Line1);//check volume
            key = 0;
         }
      }

      else if(key == 3) //senser output display
      {
          sendFlag = 1;
          key=0;
      }
      else if(key == 4) //senser output display
      {
          lcd(0,0,erase);
          lcd(0,1,erase);
          key=0;
      }
    } 
    
    if(adc1Flag)
    {  
      adc_pwm = map(adc_data,0,4095,0,100);
      if(abs(adc_pwm - old_pwm)>=5)
          pwm = adc_pwm;
      adc1Flag = 0;
    }

    if(pwm != old_pwm  )
    {
        if(pwm == 0)          
             ccr1 = 1;
        else if(pwm == 100)
             ccr1 = 177 * 100 - 1;
        else
             ccr1 = 177 * pwm;
        
        TIM10->CCR1 = ccr1;
        old_pwm = pwm;
        printf("PWM : %d\r\n",pwm);
    }
  }
}

void Serial4_Event()
{
  int i=0;
  int num = 0;
  
  char * pToken;
  char * pArray[ARR_CNT]={0};
  char recvBuf[CMD_SIZE]={0};
  char sendBuf[CMD_SIZE]={0}; 

  strcpy(recvBuf,rx4Data);
  i=0; 
  printf("rx4Data : %s\r\n",recvBuf);
  pToken = strtok(recvBuf,"[@]");

  while(pToken != NULL)
  {
    pArray[i] =  pToken;
    if(++i >= ARR_CNT)
      break;
    pToken = strtok(NULL,"[@]");
  }

  if(!strcmp(pArray[1],"SENSOR")){
    curillu = atoi(pArray[2]);
    curtemp = atof(pArray[3]);
    curhumi = atof(pArray[4]);
    curdis = atoi(pArray[5]);
  }
  else if(!strcmp(pArray[1],"TIMESET")){
          totalSec = 0;
          t_cnt = 0;
          clkFlag = 1;
          strcpy(erase,"                ");
          time.hour = atoi(pArray[2]);
          time.min = atoi(pArray[3]);
          time.sec = atoi(pArray[4]);
          totalSec = 3600 * time.hour + 60 * time.min + time.sec;
          sprintf(Line1,"Timer set");          
          sprintf(Line2,"%02d:%02d:%02d",time.hour, time.min, time.sec );
          strcpy(setTime, Line2);
          curSec = 0;
          progress=0;
          clrscr();
          lcd(0,0,Line1);
          lcd(0,1,Line2);
  }      
  else if(!strcmp(pArray[1],"VAL")){
          illu = atoi(pArray[2]);
          temp = atof(pArray[3]);
          humi = atof(pArray[4]);
          dis = atoi(pArray[5]);
          sprintf(Line1,"T: %.1lf, H: %.1lf",temp, humi);
          sprintf(Line2,"D: %d, I: %d",dis, illu);
          clrscr();
          lcd(0,0,Line1);
          lcd(0,1,Line2);
  }
  if(!strcmp(pArray[1],"LIMIT")){
          limFlag = 1;
          printf("limflag = %d",limFlag);
          if(pArray[2] != NULL)
            limIllu = atoi(pArray[2]);
          else
            limIllu = 0;
          if(pArray[3] != NULL)
            limTemp = atof(pArray[3]);
          else
            limTemp = 0;
          if(pArray[4]!=NULL)
            limHumi = atof(pArray[4]);
          else
            limHumi = 0;
  }
  else if(!strcmp(pArray[1],"MOT")){
    if(pArray[3]!=NULL)
      num = atoi(pArray[3]);
    else
      num = 0;
    if(!strcmp(pArray[2],"RIGHT")){
      Motor_Right();
    }
    else if(!strcmp(pArray[2],"LEFT")){
      Motor_Left();
    }
    else if(!strcmp(pArray[2],"STOP")){
      Motor_Stop();
    }
    else if(!strcmp(pArray[2],"PWM")){
      Motor_Pwm(num);
    }
  }
  else if(!strncmp(pArray[1]," New conn",sizeof(" New conn")))
  {
      return;
  }
  else if(!strncmp(pArray[1]," Already log",sizeof(" Already log")))
  {
      return;
  }    
  else
    return;
  
  sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
  Serial4_Send_String(sendBuf);
}

void Motor_Stop()
{
  GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_RESET); 
  GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_RESET);
}
void Motor_Right()
{
  GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_SET); 
  GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_RESET);
}
void Motor_Left()
{
  GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_RESET); 
  GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_SET);
}
void Motor_Pwm(int val)
{
    pwm = val;
}
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
