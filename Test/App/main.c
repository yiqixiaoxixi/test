
#include "common.h"
#include "include.h"

//北斗宏定义

#define BD_REC_NULL     0    //空闲；
#define BD_REC_CODE     1    //指令/内容；
#define BD_REC_LENGTH   2    //帧长度；
#define BD_REC_USRADDR  3    //用户地址；   
#define BD_REC_INFOR    4    //信息内容；
#define BD_REC_SUM      5    //校验和；

#define BD_CODE_FULL    5    //指令长度字节数
#define BD_LENGTH_FULL  2    //帧长度字节数
#define BD_USRADDR_FULL 3    //用户地址字节数
#define BD_SUM_FULL     1    //校验和字节数

#define USERADDR    292210   //用户地址 

uint16 BD_INFOR_FULL;        //信息内容字节数
uint16 BD_REC_STAT = BD_REC_NULL;    //接收状态
uint16 BD_REC_COUNT;    //对接收字节计数
char BD_REC_XOR; //保存字节抑或结果
char BD_REC[100];  //保存接收到的数据
char BD_CODE[6];  //保存接收到的指令内容
char BD_LENGTH[2];  //保存接收到的帧长度
char BD_USRADDR[3];  //保存接收到的用户地址
char BD_INFOR[100];  //保存接收到的信息内容
char BD_SUM;  //保存接收到的校验和
char GLJC[12]={'$','G','L','J','C',0x00,0x0C,0x00,0x2A};
char DWSQ[22]={'$','D','W','S','Q',0x00,0x16,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x23};
char TXSQ[22]={'$','D','W','S','Q',0x00,0x16,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x23};

void readuart(char ch,char *str); 
void BDProtocol(void);      //北斗协议转换函数
uint32 stringtodec(uint16 i,uint16 j); 
char *string123(uint16 i,uint16 j);

void uart1_handler(void)
{
    char ch;
    char *temp;
    temp=BD_REC;
    if(uart_query    (UART1) == 1)   //接收数据寄存器满
    {
        //用户需要处理接收数据
        uart_getchar(UART1,&ch);
        readuart(ch,temp);
    }
}
void uart3_handler(void)
{
    char ch;
    char *str;
    str=GLJC;
    if(uart_query    (UART1) == 1)   //接收数据寄存器满
    {
        //用户需要处理接收数据
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
    set_vector_handler(UART1_RX_TX_VECTORn,uart1_handler);   // 设置中断服务函数到中断向量表里 
    set_vector_handler(UART3_RX_TX_VECTORn,uart3_handler);   // 设置中断服务函数到中断向量表里
    uart_rx_irq_en (UART3);                                 //开串口接收中断
    //uart_tx_irq_en (UART3);                                 //开串口发送中断
    uart_rx_irq_en (UART1);                                 //开串口接收中断
    //uart_tx_irq_en (UART1);                                 //开串口发送中断   
    while(1)
    {
        printf("请输入功能代码：\n");
        printf("1：功率检测  2：定位申请  3：通信申请");     
    }
}
void readuart(char ch,char *str)
{
    switch (BD_REC_STAT)
    {
        case BD_REC_NULL: //空闲状态标志；
            if ('$' == ch) //判断是否为起始符
            {
                *str = ch; //添加字符
                str++;
                BD_REC_STAT = BD_REC_CODE; //起始标志 
                BD_REC_XOR = ch;
                BD_REC_COUNT++;
            }
            break;
        case BD_REC_CODE: //起始标志位；
            *str = ch; //添加字符
            str++;
            BD_REC_XOR ^= ch; //进行抑或运算；
            BD_REC_COUNT++;
            if (BD_CODE_FULL == BD_REC_COUNT) //是否接满五个字符，是；
            {
                BD_REC_STAT = BD_REC_LENGTH; //置为帧长度标志；
                BD_REC_COUNT = 0;
            }
            break;
         case BD_REC_LENGTH: //帧长度标志
            *str = ch; //添加字符;
            str++;
            BD_REC_XOR ^= ch; //进行抑或运算；
            BD_REC_COUNT++;
            if (BD_LENGTH_FULL == BD_REC_COUNT) //是否接满两个字符，是；
            {
                BD_REC_STAT = BD_REC_USRADDR; //置为用户地址标志；
                BD_INFOR_FULL = ch-11;
                BD_REC_COUNT = 0;
                
            }
            break;
        case BD_REC_USRADDR: //是否为用户地址标志
            *str = ch; //添加字符;
            str++;
            BD_REC_XOR ^= ch; //进行抑或运算；
            BD_REC_COUNT++;
            if (BD_USRADDR_FULL == BD_REC_COUNT) //是否接满三个字符，是；
            {
                BD_REC_STAT = BD_REC_INFOR; // 置为内容信息标志；
                BD_REC_COUNT = 0;
            }
            break;
        case BD_REC_INFOR: //是否为内容信息标志
            *str = ch; //添加字符;
            str++;
            BD_REC_XOR ^= ch; //进行抑或运算；
            BD_REC_COUNT++;
            if (BD_INFOR_FULL == BD_REC_COUNT) //是否接满所有信息内容，是；
            {
                BD_REC_STAT = BD_REC_SUM; //置为校验标志；
                BD_REC_COUNT = 0;
            }
            break;
        case BD_REC_SUM: //校验位标志
            *str = ch; //添加字符;
            if (ch == BD_REC_XOR) //校验和正确；
            {
              printf("%s",BD_REC);                    //发送字符串
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
            BD_REC_STAT = BD_REC_NULL; //返回空闲态;
            break;
        default:
            break;
    }  
}
void BDProtocol(void)
{
    if (strcmp("$GLZK",BD_CODE) == 0)
    {
       printf("----------------功率状况----------------\n");
       printf("功率1：%d \n",BD_INFOR[0]);
       printf("功率2：%d \n",BD_INFOR[1]);
       printf("功率3：%d \n",BD_INFOR[2]);
       printf("功率4：%d \n",BD_INFOR[3]);
       printf("功率5：%d \n",BD_INFOR[4]);
       printf("功率6：%d \n",BD_INFOR[5]);
    }
    if (strcmp("$DWXX",BD_CODE) == 0)
    {
        printf("----------------定位信息----------------\n");
        printf("定位时刻");
        if (0x08 == BD_INFOR[0])
        {
            printf("(东8区时间)：");
        }
        if (0x00 == BD_INFOR[0])
        {
            printf("(UTC时间)：");
        }
        printf("%dh %dm %ds %d(.1s) \n",BD_INFOR[4],BD_INFOR[5],BD_INFOR[6],BD_INFOR[7]);
        printf("大地经度：");
        printf("%d度 %d分 %d秒 %d(.1秒) \n",BD_INFOR[8],BD_INFOR[9],BD_INFOR[10],BD_INFOR[11]);
        printf("大地纬度：");
        printf("%d度 %d分 %d秒 %d(.1秒) \n",BD_INFOR[12],BD_INFOR[13],BD_INFOR[14],BD_INFOR[15]);
    }
    if (strcmp("$TXXX",BD_CODE) == 0)
    {
        printf("----------------通信信息----------------\n");
        printf("信息类别：电文形式（混合）通信方式（通信）\n");
        printf("用户地址：%d\n",BD_USRADDR);
        printf("发信地址：%ld\n",stringtodec(1,3));
        printf("电文长度：%ld\n",stringtodec(6,2));
        printf("CRC标志：0\n");
        printf("电文内容：（混合）%s\n",string123(9,BD_INFOR_FULL-2));
    }
    if (strcmp("$GNPX",BD_CODE) == 0)
    {
        printf("----------------位置信息----------------\n");   
        printf("大地经度：");
        if (0x45 == BD_INFOR[0])
        {
            printf("东经");
        }
        if (0x57 == BD_INFOR[0])
        {
            printf("西经");
        }
        printf("%d度 %d分 %d秒 %d(.1秒) \n",BD_INFOR[1],BD_INFOR[2],BD_INFOR[3],BD_INFOR[4]);
        printf("大地纬度：");
         if (0x53 == BD_INFOR[5])
        {
            printf("南纬");
        }
        if (0x4E == BD_INFOR[5])  
        {
            printf("北纬");
        }
        printf("%d度 %d分 %d秒 %d(.1秒) \n",BD_INFOR[6],BD_INFOR[7],BD_INFOR[8],BD_INFOR[9]);
        printf("高度：%ldm \n",stringtodec(10,2));
        printf("速度：%ld(.1m/s) \n",stringtodec(12,2));
        printf("方向：%ld度 \n",stringtodec(14,2));
        printf("卫星数：%dm \n",BD_INFOR[16]);
        printf("状态：");
        if(0x01 == BD_INFOR[17])
        {
            printf("定位成功 \n");
        }
        if(0x00 == BD_INFOR[17])
        {
            printf("定位不成功 \n");
        }
        printf("精度系数：%d \n",BD_INFOR[18]);
        printf("估计误差：%ld(.1m) \n",stringtodec(19,2));
    }
    if (strcmp("$GNTX",BD_CODE) == 0)
    {
        printf("----------------时间日期信息----------------\n");
        printf("当前时间"); 
        if (0x08 == BD_INFOR[0])
        {
            printf("(东8区时间)：");
        }
        if (0x00 == BD_INFOR[0])
        {
            printf("(UTC时间)：");
        }
        printf("20%d年 %d月 %d日 %d时 %d分 %d秒 \n",BD_INFOR[1],BD_INFOR[2],BD_INFOR[3],BD_INFOR[4],BD_INFOR[5],BD_INFOR[6]);
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