/**************************************************************************************
*
* 손잡이측 제어 및 센싱 아두이노 코드 제작
*
* 이름      날짜        함수/설명                   함수설명
* 강태욱    23.04.29    통신용 패킷 작성.
* 강태욱    23.04.29    모터 기본 동작 함수 작성
* 사공준    23.04.29    모터 기본 동작 함수 작성
*                       void motor_set_default()   모터의 위치를 default값으로 설정한다.
*                       void motor_set()           모터의 위치를 설정값으로 맞춰준다.
* 강태욱    23.05.21    통신용 함수 작성
*                       void send()                송신용 함수.
*                       void read()                수신용 함수.
* 강태욱    23.05.21    패킷 관련 함수 작성
*                       PACKET reset_pakt(PACKET)  패킷 정보 0으로 초기화한다.
* 강태욱    23.05.21    모터 동작 조건 함수 작성
*                       void process()             조건에 따라 모터를 동작시킨다.
* 강태욱    23.05.21    사람 존재여부 확인 함수 작성
*                       void check_person()        해당 위치에 사람이 있는지 여부를 판단한다.
*
* Kang      23.05.21 ~  LANGUAGE TRANSFER START (for Github Commit)
* 
**************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

#define TRUE 1
#define FALSE 0

/* 아두이노 위치 설정 */
#define FLOOR 1
#define SECTOR 3

/* 모터각 설정 */
#define ANGLE 78
#define RESET 90

/* 송수신 주소 설정 */
#define MASTER "00001"  //write
#define SLAVE "10001"   //read

/* RF모듈 핀 설정*/
#define CE_PIN 7
#define CSN_PIN 8

/* 송수신 강도 설정 */
#define PA_LEVEL RF24_PA_MIN

/* 시리얼모니터 Baud rate 설정 */
#define SERIAL_COMM 115200

/* 통신 구조체(패킷) 설정 */
typedef struct {
  int fire;    // 클라이언트 기준 수신, 화재여부
  int dir;     // 클라이언트 기준 수신, 수신받은 모터작동방향
  int floor;   // 클라이언트 기준 송신, 사람위치 전송, 매크로 전송
  int sector;  // 클라이언트 기준 송신, 사람위치 전송, 매크로 전송
  int person;  // 클라이언트 기준 송신, 사람유무 전송
} PACKET;

/* 모터 핀, 이름 설정*/
Servo forward;
Servo backward;

/* 라디오 핀 설정*/
RF24 radio(CE_PIN, CSN_PIN);

/* 송수신 주소 설정*/
const byte master_add[6] = MASTER;  //rx
const byte slave_add[6] = SLAVE; //tx

/* 변수 정의/초기화 */
PACKET rx, tx, saved;   // 통신용 패킷 선언
bool newdata = FALSE;   // 데이터수신여부
int touch1 = 2;         // 터치센서 핀 설정(현재 1개)

/* 함수 정의 */
void motor_set_default();
void motor_set(int dir);
void read();
void send();
void process();
PACKET reset_pakt(PACKET a);
void check_person();

/* setup 초기화 */
void setup() {
  Serial.begin(SERIAL_COMM);
  Serial.println("Slave Start");

  //모터 핀 설정
  forward.attach(5);
  forward.write(RESET);
  backward.attach(6);
  backward.write(RESET);

  //통신 설정
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(slave_add);
  radio.openReadingPipe(1, master_add);
  radio.setRetries(3,5); // delay, count
  radio.setPALevel(PA_LEVEL);
  radio.startListening();

  //통신 패킷 초기화
  tx = reset_pakt(tx);
  rx = reset_pakt(rx);
  saved = reset_pakt(saved);

  // 터치센서 핀 설정
  pinMode(touch1, INPUT);
}

void loop() {
  while(1){
    read();
    if(newdata == FALSE){
      Serial.println("No Received");
      delay(1000);
      continue;
    }
    process();
    Serial.println(rx.fire);
    Serial.println(rx.dir);
    Serial.println("");
    check_person();
    send();
  }
}

void motor_set_default() {
/************************************
* 초기값 설정 함수
* 90도 상태로 설정
*************************************/
  forward.write(RESET);
  backward.write(RESET);
}

void motor_set(int dir) {
/************************************
* 각도 설정 함수.
* 세팅된 값에 맞춰 댄스
*************************************/
  if (dir == 1) {
    forward.write(RESET + ANGLE);
  } else if (dir == -1) {
    backward.write(RESET - ANGLE);
  }
}

void read() {
/************************************
* 통신. 수신함수
* 화재감지기가 붙어있는 아두이노로부터
* 데이터 수신 (화재여부, 위치 등등)
* 수신된 데이터는 rx에 저장.
* 데이터 변환이 있으므로, newdata에 참 저장.
*************************************/
  if(radio.available()){
    radio.read(&rx, sizeof(PACKET));
    newdata = TRUE;
  }
  return;
}

void process() {
/************************************
* 조건제어. 수신된 데이터 바탕으로
* 모터 제어
*************************************/

  if (rx.fire == 0 && saved.fire == 0) {
  }

  else if (rx.fire == 1 && saved.fire == 0) {
    motor_set(rx.dir);
  }

  else if (rx.dir != saved.dir) {
    motor_set_default();
    motor_set(rx.dir);
  }

  else if (rx.fire == 0) {
    motor_set_default();
  }
  saved = rx;
}

PACKET reset_pakt(PACKET a) {
/************************************
* Reset PACKETs
*************************************/
  a.fire = 0;
  a.dir = 0;
  a.floor = 0;
  a.sector = 0;
  a.person = 0;
  return a;
}

void send() {
/************************************
* Transmit Function
*
* Transmit data to MASTER
* If transmit success, print 'Acknowledge Received'
* Else, 'TX failed'
*************************************/
  while(1){
    if (newdata == TRUE) {
      radio.stopListening();
      bool rslt;
      rslt = radio.write(&tx, sizeof(tx));
      radio.startListening();

      if (rslt) {
        Serial.println("Acknowledge Received");
        break;
      }
      else {
        Serial.println("TX failed");
        delay(1000);
      }
    }
  }
  Serial.println();
  newdata = false;
  reset_pakt(tx);
  reset_pakt(rx);
}

void check_person(){
/************************************
* Check whether a person exists or not.
*************************************/
  int val = digitalRead(touch);
  if(val == HIGH){
    tx.person = 1;
    tx.floor = FLOOR;
    tx.sector = SECTOR;
  }
}