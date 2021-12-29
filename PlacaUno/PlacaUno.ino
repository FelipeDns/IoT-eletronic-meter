// AC Voltage Sensor without LCD By Solarduino (Revision 2)

// Note Summary
// Note :  Safety is very important when dealing with electricity. We take no responsibilities while you do it at your own risk.
// Note :  This AC Votlage Sensor Code is for Single Phase AC Voltage transformer ZMPT101B module use.
// Note :  The value shown in Serial Monitor is refreshed every second and is the average value of 4000 sample readings.
// Note :  The voltage measured is the Root Mean Square (RMS) value.
// Note :  The analog value per sample is squared and accumulated for every 4000 samples before being averaged. The averaged value is then getting square-rooted.
// Note :  The auto calibration (voltageOffset1) is using averaged analogRead value of 4000 samples.
// Note :  The auto calibration (currentOffset2) is using calculated RMS current value including currentOffset1 value for calibration.  
// Note :  The unit provides reasonable accuracy and may not be comparable with other expensive branded and commercial product.
// Note :  All credit shall be given to Solarduino.

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/////////////*/


/* 0- General */
        #include "ACS712.h"
        #include <SoftwareSerial.h>
        #include <Wire.h>
        #include <RTClib.h>

        RTC_DS3231 rtc;
        ACS712 sensor(ACS712_30A, A0);
        SoftwareSerial ArduinoUno(7,8); //(RX,TX)
        
        int decimalPrecision = 2;                   // decimal places for all values shown in LED Display & Serial Monitor
        String data;

        int k7 = 0;
        int FIC =0;
        DateTime energiaCaiu = 0;
        uint32_t energiaCaiuUnix =0;
        DateTime energiaVoltou = 0;
        uint32_t energiaVoltouUnix =0;

        double parametroDIC_segundos = 0;
        
        /* 1- AC Voltage Measurement */
        int VoltageAnalogInputPin = A2;             // Which pin to measure voltage Value
        float voltageSampleRead  = 0;               /* to read the value of a sample in analog including voltageOffset1 */
        float voltageLastSample  = 0;               /* to count time for each sample. Technically 1 milli second 1 sample is taken */
        float voltageSampleSum   = 0;               /* accumulation of sample readings */
        float voltageSampleCount = 0;               /* to count number of sample. */
        float voltageMean ;                         /* to calculate the average value from all samples, in analog values*/ 
        float RMSVoltageMean ;                      /* square roof of voltageMean without offset value, in analog value*/
        float adjustRMSVoltageMean;
        float FinalRMSVoltage;                      /* final voltage value with offset value*/
   
        
        /* 1.1- AC Voltage Offset */
        
              float voltageOffset1 =0.00 ;          // to Offset deviation and accuracy. Offset any fake current when no current operates. 
                                                    // Offset will automatically callibrate when SELECT Button on the LCD Display Shield is pressed.
                                                    // If you do not have LCD Display Shield, look into serial monitor to add or minus the value manually and key in here.
                                                    // 26 means add 26 to all analog value measured.
              float voltageOffset2 = 0.00;          // too offset value due to calculation error from squared and square root 
              


void setup() {
 
/* 0- General */

  if (! rtc.begin()) {                         //Se o RTC nao for inicializado, faz
    Serial.println("RTC NAO INICIALIZADO");    //Imprime o texto
    while (1);                                 //Trava o programa
  }
  
    Serial.begin(9600);
    ArduinoUno.begin(115200);                             /* In order to see value in serial monitor */
    sensor.calibrate();
}
     
void loop() 

{
  
   
   /* 1- AC Voltage Measurement */
        
        if(micros() >= voltageLastSample)                                                                             /* every 0.2 milli second taking 1 reading */
          {
            voltageSampleRead = (analogRead(VoltageAnalogInputPin)- 512)+ voltageOffset1;                             /* read the sample value including offset value*/
            voltageSampleSum = voltageSampleSum + sq(voltageSampleRead) ;                                             /* accumulate total analog values for each sample readings*/
            
            voltageSampleCount = voltageSampleCount + 1;                                                              /* to move on to the next following count */
            voltageLastSample = micros() ;                                                                            /* to reset the time again so that next cycle can start again*/ 
          }
        
        if(voltageSampleCount == 1000)                                                                                /* after 4000 count or 800 milli seconds (0.8 second), do the calculation and display value*/
          {
            voltageMean = voltageSampleSum/voltageSampleCount;                                                        /* calculate average value of all sample readings taken*/
            RMSVoltageMean = (sqrt(voltageMean))*1.5;                                                                 // The value X 1.5 means the ratio towards the module amplification.      
            adjustRMSVoltageMean = RMSVoltageMean + voltageOffset2;                                                   /* square root of the average value including offset value */                                                                                                                                                       /* square root of the average value*/                                                                                                             
            
            FinalRMSVoltage = RMSVoltageMean + voltageOffset2;                                                        /* this is the final RMS voltage*/
            float I = sensor.getCurrentAC();
            if(FinalRMSVoltage <= 2.5)                                                                                /* to eliminate any possible ghost value*/
                {FinalRMSVoltage = 0;}
            if(I <= 0.08){                                                                                            /* to eliminate any possible ghost value*/
                I = 0;}
                
            float P = FinalRMSVoltage*I;


            if(FinalRMSVoltage == 0 && k7 == 0){
                FIC = FIC+1;
                k7 = 1;
                energiaCaiu = rtc.now();
                energiaCaiuUnix = energiaCaiu.unixtime();
                Serial.print("Hora em que houve interrupção: ");     
                Serial.print(energiaCaiu.hour(), DEC);                        //Imprime hora
                Serial.print(':');                                      //Imprime dois pontos
                Serial.print(energiaCaiu.minute(), DEC);                      //Imprime os minutos
                Serial.print(':');                                      //Imprime dois pontos
                Serial.print(energiaCaiu.second(), DEC);                      //Imprime os segundos
                Serial.println();
                
            }

            if(FinalRMSVoltage != 0 && k7 == 1){
                k7 = 0;
                DateTime energiaVoltou = rtc.now();
                energiaVoltouUnix = energiaVoltou.unixtime();
                Serial.print("Hora em que voltou a energia: ");     
                Serial.print(energiaVoltou.hour(), DEC);                        //Imprime hora
                Serial.print(':');                                      //Imprime dois pontos
                Serial.print(energiaVoltou.minute(), DEC);                      //Imprime os minutos
                Serial.print(':');                                      //Imprime dois pontos
                Serial.print(energiaVoltou.second(), DEC);                      //Imprime os segundos
                Serial.println();

                parametroDIC_segundos = parametroDIC_segundos + (energiaVoltouUnix - energiaCaiuUnix);
                Serial.println(parametroDIC_segundos);
                /*Serial.print("Tempo de Interrupção: ");     
                Serial.print(parametroDEC.hours(), DEC);                        //Imprime hora
                Serial.print(':');                                      //Imprime dois pontos
                Serial.print(parametroDEC.minutes(), DEC);                      //Imprime os minutos
                Serial.print(':');                                      //Imprime dois pontos
                Serial.print(parametroDEC.seconds(), DEC);                      //Imprime os segundos
                Serial.println();*/

                
            }

                
            Serial.print(" Valor da tensão: ");
            Serial.print(FinalRMSVoltage,decimalPrecision);
            Serial.println(" V ");
            Serial.print(" Valor da corrente: ");
            Serial.print(I);
            Serial.println(" A ");
            Serial.print(" Valor da potência: ");
            Serial.print(P);
            Serial.println(" W ");
            Serial.print(" Valor de DIC: ");
            Serial.print(parametroDIC_segundos);
            Serial.println("s ");
            Serial.print(" Valor de FIC: ");
            Serial.println(FIC);

            
            //data = String(' ') + String(P) + String(' ') + String(FinalRMSVoltage) + String(' ') + String(I);
            data = String(P) + String(',') + String(FinalRMSVoltage) + String(',') + String(I) + String(',') + String(parametroDIC_segundos) + String(',') + String(FIC);
            ArduinoUno.println(data);
            Serial.println(data);
            Serial.println("  ");
            Serial.println("  ");
            voltageSampleSum =0;                                                                                      /* to reset accumulate sample values for the next cycle */
            voltageSampleCount=0;                                                                                     /* to reset number of sample for the next cycle */
            delay(1000);
          }


}
