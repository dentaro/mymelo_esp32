// Arduino  私だけの電子オルゴール 減衰矩形波/減衰正弦波
// 2020.4.21 koyama@hirosaki-u.ac.jp
// 2021.2.24 esp32移植byでんたろう
#include "mymelo.h"
#define  Nc  256
#define BUFFER_LEN    (256)        // バッファサイズ
const char W[] PROGMEM={0,0,0,0,1,1,1,2,2,3,4,5,5,6,7,9,10,11,12,14,15,17,18,20,21,23,25,27,29,31,33,35,37,40,42,44,47,49,52,54,57,59,62,65,67,70,73,76,79,82,85,88,90,93,97,100,103,106,109,112,115,118,121,124,127,131,134,137,140,143,146,149,152,155,158,162,165,167,170,173,176,179,182,185,188,190,193,196,198,201,203,206,208,211,213,215,218,220,222,224,226,228,230,232,234,235,237,238,240,241,243,244,245,246,248,249,250,250,251,252,253,253,254,254,254,255,255,255,255,255,255,255,254,254,254,253,253,252,251,250,250,249,248,246,245,244,243,241,240,238,237,235,234,232,230,228,226,224,222,220,218,215,213,211,208,206,203,201,198,196,193,190,188,185,182,179,176,173,170,167,165,162,158,155,152,149,146,143,140,137,134,131,128,124,121,118,115,112,109,106,103,100,97,93,90,88,85,82,79,76,73,70,67,65,62,59,57,54,52,49,47,44,42,40,37,35,33,31,29,27,25,23,21,20,18,17,15,14,12,11,10,9,7,6,5,5,4,3,2,2,1,1,1,0,0,0};

//original
//const char W[] PROGMEM={0,0,0,0,1,1,1,2,2,3,4,5,5,6,7,9,10,11,12,14,15,17,18,20,21,23,25,27,29,31,33,35,37,40,42,44,47,49,52,54,57,59,62,65,67,70,73,76,79,82,85,88,90,93,97,100,103,106,109,112,115,118,121,124,127,131,134,137,140,143,146,149,152,155,158,162,165,167,170,173,176,179,182,185,188,190,193,196,198,201,203,206,208,211,213,215,218,220,222,224,226,228,230,232,234,235,237,238,240,241,243,244,245,246,248,249,250,250,251,252,253,253,254,254,254,255,255,255,255,255,255,255,254,254,254,253,253,252,251,250,250,249,248,246,245,244,243,241,240,238,237,235,234,232,230,228,226,224,222,220,218,215,213,211,208,206,203,201,198,196,193,190,188,185,182,179,176,173,170,167,165,162,158,155,152,149,146,143,140,137,134,131,128,124,121,118,115,112,109,106,103,100,97,93,90,88,85,82,79,76,73,70,67,65,62,59,57,54,52,49,47,44,42,40,37,35,33,31,29,27,25,23,21,20,18,17,15,14,12,11,10,9,7,6,5,5,4,3,2,2,1,1,1,0,0,0};
// 三角波(値:0～255, データ数:256)
//prog_uint8_t W[]={0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,129,131,133,135,137,139,141,143,145,147,149,151,153,155,157,159,161,163,165,167,169,171,173,175,177,179,181,183,185,187,189,191,193,195,197,199,201,203,205,207,209,211,213,215,217,219,221,223,225,227,229,231,233,235,237,239,241,243,245,247,249,251,253,255,253,251,249,247,245,243,241,239,237,235,233,231,229,227,225,223,221,219,217,215,213,211,209,207,205,203,201,199,197,195,193,191,189,187,185,183,181,179,177,175,173,171,169,167,165,163,161,159,157,155,153,151,149,147,145,143,141,139,137,135,133,131,129,127,126,124,122,120,118,116,114,112,110,108,106,104,102,100,98,96,94,92,90,88,86,84,82,80,78,76,74,72,70,68,66,64,62,60,58,56,54,52,50,48,46,44,42,40,38,36,34,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2};
uint8_t x,y;

#define extSW 34
#define Bzz1  25
#define Bzz2  26

#define CNT 8000000/256*0.5/8*120/TEMPO // テンポ120の四分音符(長さ8)を0.5秒とする
uint16_t cntr=CNT;                    // 32us*(1953*120/Tempo)=62.5ms
uint16_t p1=0, p2=0, p3=0;            // 楽譜part1～part3のポインタ(初期値は先頭)
uint8_t len1=8, len2=8, len3=8;       // 初期ディレイ(62.5ms*8=.5s)
uint8_t decayTimer=0;                 // 32μs*16=480μsごとに減衰させる
uint8_t icntr;                        // チャタリング防止カウンタ(32*255=8.1ms)
uint8_t nPlay=2;                      // 演奏回数(初回は2回演奏)

uint16_t v,v1,v2,v3, u,u1,u2;          // 振幅とその合算値
uint16_t E,E1,E2;                     // 包絡
uint16_t pW1,pW2,pW3, sw1,sw2,sw3;    // 波形ポインタとサンプリングステップ幅
uint8_t note1,note2;                  // 楽譜データDO|L4

hw_timer_t * timerA = NULL;//スピーカー用
boolean timer_flag = false;
float vol = 0.0;
int i = 0.0;
void IRAM_ATTR onTimerA() {
  timer_flag = true;
}
void setup() {
  Serial.begin(115200);
//  float wmax = 255;
//  for (int i = 0; i < 1024; i++) {
//    W[i] = (int)(wmax - (cos(2 * PI * i/1023) + 1)/2*wmax);
//    Serial.print (W[i]);
//    Serial.print (",");
//  }
  timerA = timerBegin(0, 80, true);//カウント時間は1マイクロ秒
  timerAttachInterrupt(timerA, &onTimerA, true);
  timerAlarmWrite(timerA, 32, true);//8usまで
  timerAlarmEnable(timerA);
  delay(1000);
}
void loop() {
  if(timer_flag){
//#if defined(SINE_WAVE) && (TRACK==2)//---------------------------------------------------------
    if(nPlay>0){
    if(--cntr==0){
      cntr=CNT;                       // 32us*(1953*120/Tempo)=62.5ms
      if(--len1==0){//大きい値から処理していく
        note1=pgm_read_byte(&part1[p1++]);  //mymelo.hにある楽譜データpart1のnoteデータを取得　p1で順次noteデータを参照していく
        
        if(note1==0){                 // part1終了!!
          nPlay--;                    // 演奏回数
          p1=p2=0;                    // 楽譜先頭
          note1=note2=0;
          len1=31; len2=32;           // 初期ディレイ(62.5ms*32=2s)
        }else{//波形生成用のデータを抽出
          len1=len[note1>>5];         // 音符は上位3ビット 1110 0000 -> 0000 0111
          sw1=SW[note1&=0x1f];  pW1=0;// 音は下位5ビット 0x1f= 0001 1111 
          E1=Nc<<7;                   // 初めは最大振幅(128)
          sw3=sw1+10; pW3=0;          // わずか周波数をずらす
        }
      }
      
      //-----------------------------------------
      if(--len2==0){
        note2=pgm_read_byte(&part2[p2++]);  // part2の次のデータ
        if(note2==0)  p2--;           // 0ならPart2の演奏終了
        else{
          len2=len[note2>>5];         // 音符は上位3ビット
          sw2=SW[note2&=0x1f];  pW2=0;// 音は下位5ビット
          E2=Nc<<7;                   // 初めは最大振幅
        }
      }
      //-----------------------------------------
    }
    //u1=0; v1=v3=0;
    u1=u2=0;  v1=v2=v3=0;             //初期化
    if(note1>1){                      // part1の振幅を加算
      E=(Nc<<7)-E1+128;
      u1=(uint16_t)pgm_read_byte(&W[(uint8_t)(E>>9)]);  u1-=(u1>>5);
      x=((pW1+=sw1)+128)>>8;  y=(pW1+E)>>8;
      v1=(uint16_t)pgm_read_byte(&W[x])+(uint16_t)pgm_read_byte(&W[y]);
      x=((pW3+=sw3)+128)>>8;  y=(pW3+E)>>8;
      v3=(uint16_t)pgm_read_byte(&W[x])+(uint16_t)pgm_read_byte(&W[y]);
    }else{  v1=v3=255;  u1=127; }     // 休符
////-----------------------------------------
    if(note2>1){                      // part2の振幅を加算
      E=(Nc<<7)-E2+128;
      u2=(uint16_t)pgm_read_byte(&W[(uint8_t)(E>>9)]);  u2-=(u2>>5);
      x=(pW2+=sw2)>>8;  y=(pW2+E)>>8;
      v2=(uint16_t)pgm_read_byte(&W[x])+(uint16_t)pgm_read_byte(&W[y]);
    }else{  v2=255; u2=127; }         // 休符
    v=v1+((v2+v3)>>1)+2;  u=(u1<<1)+u2+u1;
//-----------------------------------------
    dacWrite(25,(v-u)>>2);
  }
  if(icntr>0)  icntr--;
  if(--decayTimer==0) E1-=(E1>>5);    // 32us*256=8.192ms毎に包絡減衰
//  if(icntr>0) icntr--;
//  if(--decayTimer==0){ E1-=(E1>>5); E2-=(E2>>5);} // 32us*256=8.192ms毎に包絡減衰
//  dacWrite(25,32*F_CPU/8/1000000);
//  dacWrite(26,32*F_CPU/8/1000000);
  timer_flag = false;
  }//timer
}
