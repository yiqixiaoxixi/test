
#include "common.h"
#include "include.h"

//�����궨��

#define BD_REC_NULL     0    //���У�
#define BD_REC_CODE     1    //ָ��/���ݣ�
#define BD_REC_LENGTH   2    //֡���ȣ�
#define BD_REC_USRADDR  3    //�û���ַ��   
#define BD_REC_INFOR    4    //��Ϣ���ݣ�
#define BD_REC_SUM      5    //У��ͣ�

#define BD_CODE_FULL    5    //ָ����ֽ���
#define BD_LENGTH_FULL  2    //֡�����ֽ���
#define BD_USRADDR_FULL 3    //�û���ַ�ֽ���
#define BD_SUM_FULL     1    //У����ֽ���

#define USERADDR    292210   //�û���ַ 

uint16 BD_INFOR_FULL;        //��Ϣ�����ֽ���
uint16 BD_REC_STAT = BD_REC_NULL;    //����״̬
uint16 BD_REC_COUNT;    //�Խ����ֽڼ���
char BD_REC_XOR; //�����ֽ��ֻ���
char BD_REC[100];  //������յ�������
char BD_CODE[6];  //������յ���ָ������
char BD_LENGTH[2];  //������յ���֡����
char BD_USRADDR[3];  //������յ����û���ַ
char BD_INFOR[100];  //������յ�����Ϣ����
char BD_SUM;  //������յ���У���
char GLJC[12]={'$','G','L','J','C',0x00,0x0C,0x00,0x2A};
char DWSQ[22]={'$','D','W','S','Q',0x00,0x16,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x23};
char TXSQ[22]={'$','D','W','S','Q',0x00,0x16,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x23};

void readuart(char ch,char *str); 
void BDProtocol(void);      //����Э��ת������
uint32 stringtodec(uint16 i,uint16 j); 
char *string123(uint16 i,uint16 j);

void uart1_handler(void)
{
    char ch;
    char *temp;
    temp=BD_REC;
    if(uart_query    (UART1) == 1)   //�������ݼĴ�����
    {
        //�û���Ҫ�����������
        uart_getchar(UART1,&ch);
        readuart(ch,temp);
    }
}
void uart3_handler(void)
{
    char ch;
    char *str;
    str=GLJC;
    if(uart_query    (UART1) == 1)   //�������ݼĴ�����
    {
        //�û���Ҫ�����������
        uart_getchar(UART3,&ch);
        switch(ch)
        {
          
            case '1':
                while(*str)
                {
                    uart_putchar(UART1, *str++);
                }
                break;
            case '2':
                str=DWSQ;
                while(*str)
                {
                    uart_putchar(UART1, *str++);
                }
                break;
            default:
                break;     
        }
    }
}

void main()
{
    uart_init(UART1,19200); 
    set_vector_handler(UART1_RX_TX_VECTORn,uart1_handler);   // �����жϷ��������ж��������� 
    set_vector_handler(UART3_RX_TX_VECTORn,uart3_handler);   // �����жϷ��������ж���������
    uart_rx_irq_en (UART3);                                 //�����ڽ����ж�
    //uart_tx_irq_en (UART3);                                 //�����ڷ����ж�
    uart_rx_irq_en (UART1);                                 //�����ڽ����ж�
    //uart_tx_irq_en (UART1);                                 //�����ڷ����ж�   
    while(1)
    {
        printf("�����빦�ܴ��룺\n");
        printf("1�����ʼ��  2����λ����  3��ͨ������");     
    }
}
void readuart(char ch,char *str)
{
    switch (BD_REC_STAT)
    {
        case BD_REC_NULL: //����״̬��־��
            if ('$' == ch) //�ж��Ƿ�Ϊ��ʼ��
            {
                *str = ch; //����ַ�
                str++;
                BD_REC_STAT = BD_REC_CODE; //��ʼ��־ 
                BD_REC_XOR = ch;
                BD_REC_COUNT++;
            }
            break;
        case BD_REC_CODE: //��ʼ��־λ��
            *str = ch; //����ַ�
            str++;
            BD_REC_XOR ^= ch; //�����ֻ����㣻
            BD_REC_COUNT++;
            if (BD_CODE_FULL == BD_REC_COUNT) //�Ƿ��������ַ����ǣ�
            {
                BD_REC_STAT = BD_REC_LENGTH; //��Ϊ֡���ȱ�־��
                BD_REC_COUNT = 0;
            }
            break;
         case BD_REC_LENGTH: //֡���ȱ�־
            *str = ch; //����ַ�;
            str++;
            BD_REC_XOR ^= ch; //�����ֻ����㣻
            BD_REC_COUNT++;
            if (BD_LENGTH_FULL == BD_REC_COUNT) //�Ƿ���������ַ����ǣ�
            {
                BD_REC_STAT = BD_REC_USRADDR; //��Ϊ�û���ַ��־��
                BD_INFOR_FULL = ch-11;
                BD_REC_COUNT = 0;
                
            }
            break;
        case BD_REC_USRADDR: //�Ƿ�Ϊ�û���ַ��־
            *str = ch; //����ַ�;
            str++;
            BD_REC_XOR ^= ch; //�����ֻ����㣻
            BD_REC_COUNT++;
            if (BD_USRADDR_FULL == BD_REC_COUNT) //�Ƿ���������ַ����ǣ�
            {
                BD_REC_STAT = BD_REC_INFOR; // ��Ϊ������Ϣ��־��
                BD_REC_COUNT = 0;
            }
            break;
        case BD_REC_INFOR: //�Ƿ�Ϊ������Ϣ��־
            *str = ch; //����ַ�;
            str++;
            BD_REC_XOR ^= ch; //�����ֻ����㣻
            BD_REC_COUNT++;
            if (BD_INFOR_FULL == BD_REC_COUNT) //�Ƿ����������Ϣ���ݣ��ǣ�
            {
                BD_REC_STAT = BD_REC_SUM; //��ΪУ���־��
                BD_REC_COUNT = 0;
            }
            break;
        case BD_REC_SUM: //У��λ��־
            *str = ch; //����ַ�;
            if (ch == BD_REC_XOR) //У�����ȷ��
            {
              printf("%s",BD_REC);                    //�����ַ���
              str = BD_REC;
              strncpy(BD_CODE,str,5);
              str+=5;
              strncpy(BD_LENGTH,str,2);
              str+=2;
              strncpy(BD_USRADDR,str,3);
              str+=3;
              strncpy(BD_INFOR,str,BD_INFOR_FULL);
              str+=BD_INFOR_FULL;
              BD_SUM=*str;
              BDProtocol();
            }
            BD_REC_STAT = BD_REC_NULL; //���ؿ���̬;
            break;
        default:
            break;
    }  
}
void BDProtocol(void)
{
    if (strcmp("$GLZK",BD_CODE) == 0)
    {
       printf("----------------����״��----------------\n");
       printf("����1��%d \n",BD_INFOR[0]);
       printf("����2��%d \n",BD_INFOR[1]);
       printf("����3��%d \n",BD_INFOR[2]);
       printf("����4��%d \n",BD_INFOR[3]);
       printf("����5��%d \n",BD_INFOR[4]);
       printf("����6��%d \n",BD_INFOR[5]);
    }
    if (strcmp("$DWXX",BD_CODE) == 0)
    {
        printf("----------------��λ��Ϣ----------------\n");
        printf("��λʱ��");
        if (0x08 == BD_INFOR[0])
        {
            printf("(��8��ʱ��)��");
        }
        if (0x00 == BD_INFOR[0])
        {
            printf("(UTCʱ��)��");
        }
        printf("%dh %dm %ds %d(.1s) \n",BD_INFOR[4],BD_INFOR[5],BD_INFOR[6],BD_INFOR[7]);
        printf("��ؾ��ȣ�");
        printf("%d�� %d�� %d�� %d(.1��) \n",BD_INFOR[8],BD_INFOR[9],BD_INFOR[10],BD_INFOR[11]);
        printf("���γ�ȣ�");
        printf("%d�� %d�� %d�� %d(.1��) \n",BD_INFOR[12],BD_INFOR[13],BD_INFOR[14],BD_INFOR[15]);
    }
    if (strcmp("$TXXX",BD_CODE) == 0)
    {
        printf("----------------ͨ����Ϣ----------------\n");
        printf("��Ϣ��𣺵�����ʽ����ϣ�ͨ�ŷ�ʽ��ͨ�ţ�\n");
        printf("�û���ַ��%d\n",BD_USRADDR);
        printf("���ŵ�ַ��%ld\n",stringtodec(1,3));
        printf("���ĳ��ȣ�%ld\n",stringtodec(6,2));
        printf("CRC��־��0\n");
        printf("�������ݣ�����ϣ�%s\n",string123(9,BD_INFOR_FULL-2));
    }
    if (strcmp("$GNPX",BD_CODE) == 0)
    {
        printf("----------------λ����Ϣ----------------\n");   
        printf("��ؾ��ȣ�");
        if (0x45 == BD_INFOR[0])
        {
            printf("����");
        }
        if (0x57 == BD_INFOR[0])
        {
            printf("����");
        }
        printf("%d�� %d�� %d�� %d(.1��) \n",BD_INFOR[1],BD_INFOR[2],BD_INFOR[3],BD_INFOR[4]);
        printf("���γ�ȣ�");
         if (0x53 == BD_INFOR[5])
        {
            printf("��γ");
        }
        if (0x4E == BD_INFOR[5])  
        {
            printf("��γ");
        }
        printf("%d�� %d�� %d�� %d(.1��) \n",BD_INFOR[6],BD_INFOR[7],BD_INFOR[8],BD_INFOR[9]);
        printf("�߶ȣ�%ldm \n",stringtodec(10,2));
        printf("�ٶȣ�%ld(.1m/s) \n",stringtodec(12,2));
        printf("����%ld�� \n",stringtodec(14,2));
        printf("��������%dm \n",BD_INFOR[16]);
        printf("״̬��");
        if(0x01 == BD_INFOR[17])
        {
            printf("��λ�ɹ� \n");
        }
        if(0x00 == BD_INFOR[17])
        {
            printf("��λ���ɹ� \n");
        }
        printf("����ϵ����%d \n",BD_INFOR[18]);
        printf("������%ld(.1m) \n",stringtodec(19,2));
    }
    if (strcmp("$GNTX",BD_CODE) == 0)
    {
        printf("----------------ʱ��������Ϣ----------------\n");
        printf("��ǰʱ��"); 
        if (0x08 == BD_INFOR[0])
        {
            printf("(��8��ʱ��)��");
        }
        if (0x00 == BD_INFOR[0])
        {
            printf("(UTCʱ��)��");
        }
        printf("20%d�� %d�� %d�� %dʱ %d�� %d�� \n",BD_INFOR[1],BD_INFOR[2],BD_INFOR[3],BD_INFOR[4],BD_INFOR[5],BD_INFOR[6]);
    }
}
uint32 stringtodec(uint16 i,uint16 j)
{
    uint16 t;
    uint32 l=0;
    char *str;
    str=(char*)malloc(sizeof(char)*j);
    for(t=i;t<i+j;t++)
    {
        *str=BD_INFOR[t];
        str++;        
    }
    str-=j;
    switch(j)
    {
    	case 2:
    		l=str[1]+str[0]*16*16;
    		break;
    	case 3:
    		l=str[2]+str[1]*16*16+str[0]*16*16*16*16;
    		break;
    	default:
    		break;
	}
    return l;
}
char *string123(uint16 i,uint16 j)
{
    uint16 t;
    char *str;
    str=(char*)malloc(sizeof(char)*j);
    for(t=i;t<i+j;t++)
    {
        *str=BD_INFOR[t];
        str++;        
    }
    str-=j;
    return str;
}