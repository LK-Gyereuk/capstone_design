/**************************************************************************************
*
* 화제센싱 및 (최적탈출로 계산) 아두이노 코드 제작
*
* 이름      날짜              함수              설명
* 사공준    23.05.06                            tmp36소자 활용 화재감지코드 최초작성
* 강태욱    23.05.06                            tmp36소자 활용 화재감지코드 수정      
* 강태욱    23.05.06          send(),read()     send() -> 송신 함수,   read() -> 수신 함수 
* SGJ       23.05.21                           English translation for Github uploading
* Kang      23.05.21         test_mapping()    Function for testing variable fire situation
* Kang      23.05.21         reset_pakt()      Function to reset PACKET 
*
**************************************************************************************/
/**************************************************************************************
*
* 제작이 필요한 부분:
*
* 최적탈출로 계산
* 스타트/리셋/킬 스위치 필요. (화재상황 종료 및 점검시 사용)
* 화재 위치에 따라 모터의 방향 설정하는 방법이 필요함.
*
**************************************************************************************/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

/* Master and Slave address */
#define MASTER "00001"  //read
#define SLAVE "10001"   //write

/*RF module pin set*/
#define CE_PIN 7
#define CSN_PIN 8

/* Transmission power level */
#define PA_LEVEL RF24_PA_MIN

/* Serial monitor Baud rate */
#define SERIAL_COMM 115200

/* PACKET Structure  */
typedef struct {  // 최소한의 정보로 제작함.
  int fire;       // 화재여부 확인
  int dir;        // 대피방향 설정
  int floor;      // 층수
  int sector;     // 위치
  int person;     // person true / false
} PACKET;

/* Radio pin set */
RF24 radio(CE_PIN, CSN_PIN);

/* Transmission Address */
const byte master_add[6] = MASTER; // tx
const byte slave_add[6] = SLAVE; // rx

/* Variable reset */
PACKET rx, tx, saved;
bool newdata = FALSE;

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 1000;

int num = 0;

PACKET test_mapping(int i);
void read();
void send();
void reset_pakt(PACKET a);

void setup() {
  // put your setup code here, to run once;//
  Serial.begin(SERIAL_COMM);  //serial mointor speed//
  Serial.println("SimpleTxAckPayload Starting");

  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(master_add);
  radio.openReadingPipe(1, slave_add);
  radio.setPALevel(PA_LEVEL);
  // radio.enableAckPayload();
  radio.setRetries(3,5); // delay, count

  reset_pakt(saved);
  reset_pakt(rx);
  reset_pakt(tx);

}

void loop() {

  if(num==3){
    num-=3;
  }
  send();

  num += 1;

  read();

  Serial.println(rx.floor);
  Serial.println(rx.sector);
  Serial.println("");

  delay(3000);
}

void send() {
  tx = test_mapping(num);
  while(1){
    radio.stopListening();
    bool rslt;
    rslt = radio.write(&tx, sizeof(PACKET));
    radio.startListening();

    Serial.print("Data Sent ");
    Serial.println(tx.dir);
    Serial.println(tx.fire);

    if (rslt) {
      Serial.println("Acknowledge received");
      break;
    }

    else {
      Serial.println("Tx failed");
      delay(1000);
      continue;
    }
  }
  Serial.println();
  newdata = FALSE;

  reset_pakt(tx);
  reset_pakt(rx);
}

void reset_pakt(PACKET a) {
  a.fire = 0;
  a.dir = 0;
  a.floor = 0;
  a.sector = 0;
}

void read() {
/***********************
* Receivie Function
* 화재감지기가 붙어있는 아두이노로부터
* 데이터 수신 (화재여부, 위치 등등)
* 수신된 데이터는 rx에 저장.
* 데이터 변환이 있으므로, newdata에 참 저장.
***********************/
  if(radio.available()){
    radio.read(&rx, sizeof(PACKET));
    newdata = TRUE;
  }
  return;
}


PACKET test_mapping(int i){
/* JUST FOR TEST */
  PACKET pakt;
  if(i == 0){
    pakt.fire = 0;
    pakt.dir = 0;
    pakt.floor = 0;
    pakt.sector = 0;
    pakt.person = 0;
  }
  else if(i == 1){
    pakt.fire = 1;
    pakt.dir = -1;
    pakt.floor = 0;
    pakt.sector = 0;
    pakt.person = 0;
  }
  else if(i == 2){
    pakt.fire = 1;
    pakt.dir = 1;
    pakt.floor = 0;
    pakt.sector = 0;
    pakt.person = 0;
  }
  return pakt;
}
