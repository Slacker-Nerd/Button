#ifndef __BUTTON_H__
#define __BUTTON_H__
#include <Arduino.h>

/**
 * 按键检测（非阻塞软件消抖 + 单击 / 双击 / 长按）
 *
 * 用法（推荐放到一个 10ms 周期任务里轮询）：
 *
 *     KeyDetector key(10);           // 上拉接法：按键另一端接 GND
 *     key.begin();
 *     key.onPressed(cbPress);        // 下行两种写法二选一，别混用
 *     key.onClick(cbClick);
 *     ...
 *     loop() { key.detect(); }
 *
 *     // 或者只用统一回调：
 *     loop() { key.update(onKeyEvent); }
 *
 * 注意：detect() 内部已经会调用通过 onXxx() 绑定的回调，
 *       update() 只是在 detect() 之上再转发一份事件，二者不要同时用。
 */
enum KeyEvent
{
    KEY_NONE = 0,
    KEY_PRESSED,      // 按下瞬间（消抖后只触发一次）
    KEY_RELEASED,     // 松开瞬间（消抖后只触发一次）
    KEY_CLICK,        // 单击（松开后等过一个双击窗口才判定）
    KEY_DOUBLE_CLICK, // 双击
    KEY_LONG_PRESS,   // 长按（默认只触发一次，可开启重复触发）
};

// 回调类型定义
typedef void (*KeySimpleCallback)(void);
typedef void (*KeyEventCallback)(KeyEvent evt);

class KeyDetector
{
public:
    // usePullUp = true：内部上拉，按下为低电平（最常见）
    // usePullUp = false：按下为高电平（外部下拉或按键接 VCC）
    KeyDetector(uint8_t pin, bool usePullUp = true);

    // 初始化引脚，必须在 setup() 或任务启动前调用
    void begin();

    // 核心轮询，【必须周期性调用，非阻塞】。返回本轮产生的事件
    KeyEvent detect();

    // 统一事件回调版本（内部会调用 detect()）
    void update(KeyEventCallback callback = nullptr);

    // ========== 绑定独立回调 ==========
    void onPressed(KeySimpleCallback cb);
    void onReleased(KeySimpleCallback cb);
    void onClick(KeySimpleCallback cb);
    void onDoubleClick(KeySimpleCallback cb);
    void onLongPress(KeySimpleCallback cb);

    // ========== 参数配置 ==========
    void setDebounceTime(unsigned long ms);                         // 消抖时间，默认 10ms
    void setLongPressThreshold(unsigned long ms);                   // 长按判定时间，默认 1000ms
    void setDoubleClickInterval(unsigned long ms);                  // 双击最大间隔，默认 300ms
    void setLongPressRepeat(bool en, unsigned long interval = 300); // 长按连续触发

    // 当前是否处于按下状态（消抖后）
    bool isPressed();

    // 当前按住了多久（毫秒，未按下时为 0）
    unsigned long heldTime();

private:
    uint8_t _keyPin;
    bool _isPullUp;

    // 配置参数
    unsigned long _debounceTime;
    unsigned long _longPressThreshold;
    unsigned long _doubleClickInterval;
    bool _longPressRepeatEnable;
    unsigned long _longPressRepeatInterval;

    // 运行时状态
    bool _rawLevel;      // 上一次读到的原始电平
    bool _pressed;       // 消抖后的逻辑状态：true = 按着
    bool _lastPressed;   // 上一轮轮询的逻辑状态，用于边沿检测
    unsigned long _rawChangeTime;

    unsigned long _pressTime;
    unsigned long _releaseTime;
    unsigned long _lastRepeatTime;

    uint8_t _clickCount;
    bool _longPressTriggered;

    // 独立回调
    KeySimpleCallback _pressedCb;
    KeySimpleCallback _releasedCb;
    KeySimpleCallback _clickCb;
    KeySimpleCallback _doubleClickCb;
    KeySimpleCallback _longPressCb;

    // 内部工具
    bool readRaw();
    bool levelToPressed(bool level) const;
};

#endif
