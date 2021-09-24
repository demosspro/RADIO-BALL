// RX

#define STRIP_PIN 5     // пин ленты
#define NUMLEDS 20      // кол-во светодиодов
#define COLOR_DEBTH 3


 // подключаем библу
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include <microLED.h>  
microLED<NUMLEDS, STRIP_PIN, MLED_NO_CLOCK, LED_WS2812, ORDER_GRB, CLI_AVER> strip;
RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53); // для Меги
uint32_t timer0,timer1,timer2,timer3;
  int PUpin = 2;
  int power_mesure = A6;
uint16_t recieve_data[1]; // массив принятых данных
uint16_t last_data[1];
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

void setup() {
pinMode(PUpin, INPUT); //пинать питальник, чтоб не заснул
pinMode(power_mesure, INPUT); // вход с делителя
  Serial.begin(9600); //открываем порт для связи с ПК
  recieve_data[0] = 0x0;
  radio.begin(); //активировать модуль
  radio.setAutoAck(true);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.openReadingPipe(1, address[0]);     //хотим слушать трубу 0
  radio.setChannel(0x66);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_1MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp(); //начать работу
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
  strip.setBrightness(255); //яркость ленты 100%
  delay(500);
}

void loop() {
  byte pipeNo;
  float voltage = (float)(analogRead(power_mesure) * 3.3) / 1024/0.666; //получаем напряжение аккумулятора
  static int bright = 255; //яркость сотка
 while (millis() - timer0 <= 60000) { //рабочий цикл 60 сек
  if (millis() - timer1 >= 20000) {   // таймер на 20 сек пинать питальник - прижать
    pinMode(PUpin, OUTPUT);
    Serial.println ("push");
    timer1 = millis();
  }
     if (millis() - timer2 >= 20150) { // таймер на 150 мсек пинать питальник - отжать
     pinMode(PUpin, INPUT_PULLUP);
    timer2 = millis();
    Serial.println ("unpush");
  }
    if (millis() - timer3 >= 10000) {   // таймер на 10 сек - замер напруги
      Serial.println (voltage);
    timer3 = millis();
    }
 }

  if ( radio.available(&pipeNo)) {  // слушаем эфир со всех труб
    radio.read( &recieve_data, sizeof(recieve_data) );   
    // чиатем входящий сигнал
 if (recieve_data[0] == 0x1) { // если есть изменения в recieve_data
      strip.setBrightness(255); //задаем яркость
      strip.fill(mGreen);       //заливаем зеленым
      strip.show();             //послали на ленту
      Serial.println("green");
      last_data[0] = recieve_data[0]; // запомнить последнее изменение

    }
     if ((recieve_data[0] == 0x2) && (last_data[0] != 0x3) && (last_data[0] != 0x4))  { // затухание зеленого
        bright = 255;
              strip.fill(mGreen);
        last_data[0] = recieve_data[0];
while (bright > 0) {      //пока яркость выше 0
    bright --;    // 5 - множитель скорости изменения
  strip.setBrightness(bright); //присваиваем  значение яркости
  strip.show();               //выводим на ленту
delay(5);
}
}
     if (recieve_data[0] == 0x3) { // если есть изменения в recieve_data
      strip.setBrightness(255);   //задаем яркость
      strip.fill(mRed);           //заливаем ленту красным
      strip.show();              //послали на ленту
      Serial.println("red");
      last_data[0] = recieve_data[0]; // запомнить последнее изменение

    }
     if ((recieve_data[0] == 0x4) && (last_data[0] != 0x1) && (last_data[0] != 0x2)) { // если есть изменения в recieve_data
        bright = 255;
              strip.fill(mRed);
        last_data[0] = recieve_data[0];
while (bright > 0) {      //пока яркость выше 0
    bright --;    // 5 - множитель скорости изменения
  strip.setBrightness(bright); //присваиваем  значение яркости
  strip.show(); //
  delay(5);
  }
  }
  radio.writeAckPayload(pipeNo, &voltage, sizeof(voltage));
}
}
