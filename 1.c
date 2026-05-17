#include <xc.h>
#include <stdio.h>

#define _XTAL_FREQ 20000000

// CONFIG
#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

// LCD
#define RS RB0
#define EN RB1

char buffer[16];

// ---------------- LCD ----------------

void pulse_enable()
{
    EN = 1;
    __delay_ms(2);
    EN = 0;
}

void lcd_cmd(unsigned char cmd)
{
    RS = 0;

    PORTB &= 0x0F;
    PORTB |= (cmd & 0xF0);

    pulse_enable();

    PORTB &= 0x0F;
    PORTB |= ((cmd << 4) & 0xF0);

    pulse_enable();

    __delay_ms(2);
}

void lcd_data(unsigned char data)
{
    RS = 1;

    PORTB &= 0x0F;
    PORTB |= (data & 0xF0);

    pulse_enable();

    PORTB &= 0x0F;
    PORTB |= ((data << 4) & 0xF0);

    pulse_enable();

    __delay_ms(2);
}

void lcd_init()
{
    __delay_ms(20);

    lcd_cmd(0x02);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
}

void lcd_string(const char *str)
{
    while(*str)
    {
        lcd_data(*str++);
    }
}

// ---------------- ADC ----------------

void ADC_Init()
{
    ADCON0 = 0x41;
    ADCON1 = 0x80;
}

unsigned int ADC_Read(unsigned char channel)
{
    ADCON0 &= 0xC5;

    if(channel == 1)
    {
        ADCON0 |= 0x08;
    }

    __delay_ms(2);

    GO_nDONE = 1;

    while(GO_nDONE);

    return ((ADRESH << 8) + ADRESL);
}

// ---------------- PWM ----------------

void PWM_Init()
{
    TRISC2 = 0;

    PR2 = 249;

    CCP1CON = 0x0C;

    T2CON = 0x04;
}

void PWM_Set_Duty(unsigned int duty)
{
    CCPR1L = duty >> 2;

    CCP1CON &= 0xCF;

    CCP1CON |= ((duty & 0x03) << 4);
}

// ---------------- MAIN ----------------

void main()
{
    unsigned int temp_adc;
    unsigned int light_adc;

    float temperature;

    // INPUT / OUTPUT
    TRISA = 0xFF;

    // LCD PORTB
    TRISB = 0x00;

    // LED -> RD0
    TRISD0 = 0;

    // BUZZER -> RC0
    TRISC0 = 0;

    PORTB = 0x00;

    RD0 = 0;
    RC0 = 0;

    lcd_init();
    ADC_Init();
    PWM_Init();

    while(1)
    {
        // SICAKLIK OKU
        temp_adc = ADC_Read(0);

        temperature = temp_adc * 0.488;

        // ISIK OKU
        light_adc = ADC_Read(1);

        // PWM FAN
        if(temperature < 25)
        {
            PWM_Set_Duty(0);
        }
        else if(temperature < 30)
        {
            PWM_Set_Duty(250);
        }
        else if(temperature < 35)
        {
            PWM_Set_Duty(500);
        }
        else
        {
            PWM_Set_Duty(800);
        }

        // LED
        if(light_adc < 500)
        {
            RD0 = 1;
        }
        else
        {
            RD0 = 0;
        }

        // BUZZER
        if(temperature > 30)
        {
            RC0 = 1;
        }
        else
        {
            RC0 = 0;
        }

        // LCD SATIR 1
        lcd_cmd(0x80);

        sprintf(buffer, "TEMP: %.1f C ", temperature);

        lcd_string(buffer);

        // LCD SATIR 2
        lcd_cmd(0xC0);

        if(temperature > 30)
        {
            lcd_string("HOT WARNING ");
        }
        else if(temperature < 25)
        {
            lcd_string("FAN:OFF     ");
        }
        else if(temperature < 30)
        {
            lcd_string("FAN:LOW     ");
        }
        else if(temperature < 35)
        {
            lcd_string("FAN:MID     ");
        }
        else
        {
            lcd_string("FAN:HIGH    ");
        }

        __delay_ms(300);
    }
}