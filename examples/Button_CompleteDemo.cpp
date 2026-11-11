#include <Arduino.h>
#include "Button.h"

KeyDetector key(9, true); // GPIO9，上拉输入

void keyClick()
{
    Serial.println("单击");
}
void keyDouble()
{
    Serial.println("双击");
}
void keyLong()
{
    Serial.println("长按");
}

void keyEventCallback(KeyEvent evt)
{
    switch (evt)
    {
    case KEY_PRESSED:
        Serial.println("按下");
        break;
    case KEY_RELEASED:
        Serial.println("松开");
        break;
    case KEY_CLICK:
        Serial.println("单击");
        break;
    case KEY_DOUBLE_CLICK:
        Serial.println("双击");
        break;
    case KEY_LONG_PRESS:
        Serial.println("长按");
        break;
    default:
        break;
    }
}
// 在loop里：key.update(keyEventCallback);

void setup()
{
    Serial.begin(115200);
    key.begin();
    key.onClick(keyClick);
    key.onDoubleClick(keyDouble);
    key.onLongPress(keyLong);
    // 开启长按连续触发，每300ms触发一次
    // key.setLongPressRepeat(true,300);
}

void loop()
{
    key.detect(); // 建议 10ms 轮询一次，别用 1ms 空转
    // 这里可以放 strip.service()、WiFi、TCP 逻辑，完全不冲突
    delay(10);
}
