#include <Arduino.h>
int x = 0;    // 变量
 
void setup() {
  Serial.begin(9600);      // 打开串口通讯
}
 
void loop() {  
  // print labels
  Serial.print("NO FORMAT");       // 打印文字标志
  Serial.print("\t");             
 
  Serial.print("DEC");  
  Serial.print("\t");      
 
  Serial.print("HEX");
  Serial.print("\t");  
 
  Serial.print("OCT");
  Serial.print("\t");
 
  Serial.print("BIN");
  Serial.print("\t");
 
  for(x=0; x< 64; x++){    
 
    //通过不同格式显示
    Serial.print(x);       // 输出ASCII编码的十进制数字。与"DEC"相同
    Serial.print("\t");   
 
    Serial.print(x, DEC);  // 输出ASCII编码的十进制数字。
    Serial.print("\t");    
 
    Serial.print(x, HEX);  // 输出ASCII编码的十六进制数字。
    Serial.print("\t");    
 
    Serial.print(x, OCT);  // 输出ASCII编码的八进制数字
    Serial.print("\t");   
 
    Serial.println(x, BIN);  // 输出ASCII编码的二进制数字，然后换行
    //                             
    delay(200);            //延迟200毫秒
  }
  Serial.println("");      // 再次换行
}