// TX

 // подключаем библу
#include "GyverButton.h"
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <GyverPower.h>

void(* resetFunc) (void) = 0;
  void isr() {
     resetFunc();
}
GButton btn1(4);
GButton btn2(5);
  
RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53); // для Меги

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

uint16_t transmit_data[1]; // массив, хранящий передаваемые данные
uint16_t latest_data[1]; // массив, хранящий последние переданные данные
boolean flag; // флажок отправки данных
uint32_t timer0;
float voltage_t;
  int power_mesure = A0;
int state;

void setup() {
  pinMode(2, INPUT);

  btn1.setDebounce(80);        // настройка антидребезга (по умолчанию 80 мс)
  btn1.setTimeout(600);        // настройка таймаута на удержание (по умолчанию 500 мс)
  btn1.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)
  
  btn2.setDebounce(80);        // настройка антидребезга (по умолчанию 80 мс)
  btn2.setTimeout(600);        // настройка таймаута на удержание (по умолчанию 500 мс)
  btn2.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)

  radio.begin(); //активировать модуль
  radio.setAutoAck(false);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных
  radio.setChannel(0x66);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_1MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp(); //начать работу
  radio.stopListening();  //не слушаем радиоэфир, мы передатчик
  power.autoCalibrate(); // автоматическая калибровка ~ 2 секунды , средняя но достаточная точность
  power.hardwareDisable(PWR_ADC | PWR_TIMER1); // см раздел константы в GyverPower.h, разделяющий знак " | "
  power.setSleepMode(POWERDOWN_SLEEP); // если нужен другой режим сна, см константы в GyverPower.h (по умолчанию POWERDOWN_SLEEP)
  power.bodInSleep(false); // рекомендуется выключить bod во сне для сохранения энергии (по умолчанию false - выключен!!)
  attachInterrupt(0, isr, HIGH);


}

void loop() {
voltage_t = (float)(analogRead(power_mesure) * 3.3) / 1024/0.666; //получаем напряжение аккумулятора
if (millis() -timer0 >=10000){
  timer0 = millis();
  state =1;
}
if (voltage_r < 3.3) state = 2;
if (voltage_t < 3.3) state = 3;
switch (state){
  case 1:
  state =0;
  power.sleep(SLEEP_FOREVER);
  break;
  
  case 2:
  state =0;
  pwr_led =red;
  break;
}
btn1.tick();
btn2.tick();
  if (btn1.isClick()) transmit_data[0]=0x1;         // проверка на один клик
  if (btn1.isHold())  transmit_data[0]=0x2;         // проверк удержания
  if (btn2.isClick())  transmit_data[0]=0x3;         // проверка на один клик
  if (btn2.isHold())  transmit_data[0]=0x4;         // проверк удержания

  for (int i = 0; i < 1; i++) { // в цикле от 0 до количества переменных
    if (transmit_data[i] != latest_data[i]) { // если есть изменения в transmit_data
      flag = 1; // поднять флаг отправки по радио
      latest_data[i] = transmit_data[i]; // запомнить последнее изменение
    }
  }

  if (flag == 1) {
    radio.powerUp(); // включить передатчик
    radio.write(&transmit_data, sizeof(transmit_data)); // отправить по радио
    flag = 0; //опустить флаг
    if(radio.available() ) {                    // если в ответе что-то есть
        radio.read(&voltage_r, sizeof(voltage_r));    // читаем   
    radio.powerDown(); // выключить передатчик
  }

}
}
