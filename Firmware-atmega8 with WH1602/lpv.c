//----------------------------------------------------------------------------------------------------
//������������ ����������
//----------------------------------------------------------------------------------------------------
#include "based.h"
#include "wh1602.h"

//----------------------------------------------------------------------------------------------------
//���������
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
//���������� ����������
//----------------------------------------------------------------------------------------------------
extern char String[STRING_BUFFER_SIZE];//������

//****************************************************************************************************
//����������������
//****************************************************************************************************

//****************************************************************************************************
//����� ������ ������� � ������ ��������
//****************************************************************************************************

#define MISO_DDR      DDRB
#define MISO_PORT     PORTB
#define MISO_PIN      PINB
#define MISO          4

#define MOSI_DDR      DDRB
#define MOSI_PORT     PORTB
#define MOSI_PIN      PINB
#define MOSI          3

#define SCK_DDR      DDRB
#define SCK_PORT     PORTB
#define SCK_PIN      PINB
#define SCK          5

#define RW_DDR      DDRD
#define RW_PORT     PORTD
#define RW_PIN      PIND
#define RW          6

//----------------------------------------------------------------------------------------------------
//��������� �������
//----------------------------------------------------------------------------------------------------
void InitAVR(void);//������������� �����������
uint16_t GetData(void);//������� ������

void MOSI_Hi(void);//��������� ������� ������� �� ������ MOSI
void MOSI_Lo(void);//��������� ������ ������� �� ������ MOSI
void SCK_Hi(void);//��������� ������� ������� �� ������ SCK
void SCK_Lo(void);//��������� ������ ������� �� ������ SCK
bool GetMISOState(void);//�������� �������� ������ �� ����� MISO
float FilterIIR(float new_sample);//������

//----------------------------------------------------------------------------------------------------
//�������� ������� ���������
//----------------------------------------------------------------------------------------------------
int main(void)
{ 
 InitAVR();
 _delay_ms(500);
 WH1602_Init();
 while(1)
 {
  uint16_t value=GetData();
  float v=2.56*value/1023;//���������� �� ������������� ���������
  const float r_measure=13000;//������������� ��������
  const float r_ballast=10000000;//���������� ��������
  float i=v/r_measure;//��� � ����
  float u=i*(r_measure+r_ballast);
  u=FilterIIR(u);//��������� ���������
  sprintf(String,"U,�:%i",(int)(u));
  WH1602_SetTextUpLine(String);
  const float c=2000E-6;//������� ������� �������������
  uint32_t e=(uint32_t)(c*u*u/2);
  sprintf(String,"E,��:%i",(int)e);
  WH1602_SetTextDownLine(String);
  _delay_ms(50);
 }
 return(0);
}
//****************************************************************************************************
//****************************************************************************************************
//����� �������
//****************************************************************************************************
//****************************************************************************************************


//----------------------------------------------------------------------------------------------------
//������������� �����������
//----------------------------------------------------------------------------------------------------
void InitAVR(void)
{
 cli(); 
 //����������� �����
 DDRB=0;
 DDRD=0; 
 DDRC=0;
 
 RW_DDR|=(1<<RW); 
 MOSI_DDR|=(1<<MOSI);
 SCK_DDR|=(1<<SCK);
 MISO_DDR&=0xff^(1<<MISO);

 //����� ��������� ������
 PORTB=0x00;
 PORTD=0x00;
 PORTC=0x00;
 
 RW_PORT&=0xff^(1<<RW);
 sei(); 
}
//----------------------------------------------------------------------------------------------------
//������� ������
//----------------------------------------------------------------------------------------------------
uint16_t GetData(void)
{ 
 //���������� ������, ���� �� ������� 32 ������� ����� ����
 uint8_t counter=0;
 bool zero=false;
 while(1)
 {
  SCK_Hi();
  _delay_ms(1);
  bool state=GetMISOState();
  SCK_Lo();
  _delay_ms(1); 
  if (zero==false)
  {
   counter=0;  
   if (state==false) zero=true;
   continue;
  } 
  if (state==true) counter++;//������� �������
  else//������
  {
   counter=0;
   zero=false;
   continue;
  }
  if (counter==32) break;
 }
 //��������� ������
 uint16_t mask=(1<<0);
 uint16_t value=0;
 for(int16_t n=0;n<sizeof(uint16_t)*8;n++,mask<<=1)
 {
  SCK_Hi();
  _delay_ms(1);
  bool state=GetMISOState();
  SCK_Lo();
  _delay_ms(1);
  if (state==true) value|=mask; 
 }
 return(value);
}

//----------------------------------------------------------------------------------------------------
//��������� ������� ������� �� ������ MOSI
//----------------------------------------------------------------------------------------------------
void MOSI_Hi(void)
{
 MOSI_PORT|=(1<<MOSI);
}
//----------------------------------------------------------------------------------------------------
//��������� ������ ������� �� ������ MOSI
//----------------------------------------------------------------------------------------------------
void MOSI_Lo(void)
{
 MOSI_PORT&=0xff^(1<<MOSI);
}

//----------------------------------------------------------------------------------------------------
//��������� ������� ������� �� ������ SCK
//----------------------------------------------------------------------------------------------------
void SCK_Hi(void)
{
 SCK_PORT|=(1<<SCK);
}
//----------------------------------------------------------------------------------------------------
//��������� ������ ������� �� ������ SCK
//----------------------------------------------------------------------------------------------------
void SCK_Lo(void)
{
 SCK_PORT&=0xff^(1<<SCK);
}

//----------------------------------------------------------------------------------------------------
//�������� �������� ������ �� ����� MISO
//----------------------------------------------------------------------------------------------------
bool GetMISOState(void)
{
 if (MISO_PIN&(1<<MISO)) return(true);
 return(false);
}
//----------------------------------------------------------------------------------------------------
//������
//----------------------------------------------------------------------------------------------------
float FilterIIR(float new_sample)
{
 //������� �������
 #define NCoef 2
 
 const float ACoef[NCoef+1]=
 {
  0.02010938982754319900,
  0.04021877965508639800,
  0.02010938982754319900
 };
 const float BCoef[NCoef+1]= 
 {
  1.00000000000000000000,
  -1.56101807580071790000,
  0.64135153805756284000
 };
 static float y[NCoef+1];//������� ��������
 static float x[NCoef+1];//�������� ��������
 int n;
 //�������� ��������
 for(n=NCoef;n>0;n--) 
 {
  x[n]=x[n-1];
  y[n]=y[n-1];
 }
 //��������� ����� �������� ��������
 x[0]=new_sample;
 y[0]=ACoef[0]*x[0];
 for(n=1;n<=NCoef;n++) y[0]+=ACoef[n]*x[n]-BCoef[n]*y[n];
 return(y[0]);
}