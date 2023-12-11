#include <Arduino.h>
#include <string.h>

const int MAX_DATA_POINTS = 10;  // 定义最大数据点数量，根据需求设置

struct DataPoint {
    int id;
    char name[40];
    int values[4];
};

DataPoint dataPoints[MAX_DATA_POINTS]; // 用于存储多组数据的数组
int dataPointIndex = 0; // 当前数据点索引

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        ; // 等待串口连接
    }
}

void resetDataPoint(DataPoint &data) {
    data.id = 0;
    memset(data.name, 0, sizeof(data.name)); // 清除name字段
    for (int i = 0; i < 4; i++) {
        data.values[i] = 0;
    }
}

void loop() {
    if (Serial.available()) {
        if (dataPointIndex < MAX_DATA_POINTS) {
            DataPoint data;

            data.id = Serial.parseInt(); // 读取整数
            Serial.read(); // 读取逗号并丢弃

            Serial.readBytesUntil(',', data.name, sizeof(data.name)); // 读取名称，直到遇到逗号
            Serial.read(); // 读取逗号并丢弃

            for (int i = 0; i < 4; i++) {
                data.values[i] = Serial.parseInt(); // 读取四个整数
                if (i < 3) {
                    Serial.read(); // 读取逗号并丢弃
                }
            }

            // 将数据点存储到数组中
            dataPoints[dataPointIndex] = data;
            dataPointIndex++;

            // 打印解析后的数据
            Serial.print("ID: ");
            Serial.print(data.id);
            /*Serial.print(", Name: ");
            Serial.print(data.name);*/
            Serial.print(", Values: ");
            for (int i = 0; i < 4; i++) {
                Serial.print(data.values[i]);
                if (i < 3) {
                    Serial.print(" ");
                }
            }
            Serial.print(" ");
            //Serial.flush();
            //Serial.println();

            // 清除已接收的数据，准备接收下一组数据
            resetDataPoint(data);
        } else {
            Serial.println("Data storage is full. Skipping data.");
            // 在数据存储已满时可以采取适当的处理，如跳过数据或其他操作
        }
    }
}
