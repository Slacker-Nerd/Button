#include "Button.h"

/* ---------------------------------------------------
 * 状态机只用一个变量记录"逻辑是否按下"（_pressed），
 * 电平到逻辑的换算集中在 levelToPressed()，避免旧版
 * 把"电平"和"是否按下"混存导致空闲时狂发事件。
 * --------------------------------------------------- */

KeyDetector::KeyDetector(uint8_t pin, bool usePullUp)
{
    _keyPin = pin;
    _isPullUp = usePullUp;

    _debounceTime = 10;
    _longPressThreshold = 1000;
    _doubleClickInterval = 300;
    _longPressRepeatEnable = false;
    _longPressRepeatInterval = 300;

    // 空闲电平：上拉接法为高，下拉接法为低
    _rawLevel = _isPullUp ? HIGH : LOW;
    _pressed = false;
    _lastPressed = false;
    _rawChangeTime = 0;

    _pressTime = 0;
    _releaseTime = 0;
    _lastRepeatTime = 0;
    _clickCount = 0;
    _longPressTriggered = false;

    _pressedCb = nullptr;
    _releasedCb = nullptr;
    _clickCb = nullptr;
    _doubleClickCb = nullptr;
    _longPressCb = nullptr; // 旧版漏了这一项，长按回调是野指针
}

void KeyDetector::begin()
{
    if (_isPullUp)
    {
        pinMode(_keyPin, INPUT_PULLUP);
    }
    else
    {
        pinMode(_keyPin, INPUT_PULLDOWN);
    }

    _rawLevel = readRaw();
    _pressed = levelToPressed(_rawLevel);
    _lastPressed = _pressed;
    _rawChangeTime = millis();
}

bool KeyDetector::readRaw()
{
    return digitalRead(_keyPin);
}

bool KeyDetector::levelToPressed(bool level) const
{
    // 上拉接法：低电平 = 按下；下拉接法：高电平 = 按下
    return _isPullUp ? (level == LOW) : (level == HIGH);
}

bool KeyDetector::isPressed()
{
    return _pressed;
}

unsigned long KeyDetector::heldTime()
{
    return _pressed ? (millis() - _pressTime) : 0;
}

void KeyDetector::setDebounceTime(unsigned long ms)
{
    _debounceTime = ms;
}

void KeyDetector::setLongPressThreshold(unsigned long ms)
{
    _longPressThreshold = ms;
}

void KeyDetector::setDoubleClickInterval(unsigned long ms)
{
    _doubleClickInterval = ms;
}

void KeyDetector::setLongPressRepeat(bool en, unsigned long interval)
{
    _longPressRepeatEnable = en;
    _longPressRepeatInterval = interval;
}

KeyEvent KeyDetector::detect()
{
    KeyEvent event = KEY_NONE;
    unsigned long now = millis();

    /* ---------- 1. 软件消抖 ---------- */
    bool raw = readRaw();
    if (raw != _rawLevel)
    {
        _rawLevel = raw;
        _rawChangeTime = now;
    }
    if (now - _rawChangeTime >= _debounceTime)
    {
        _pressed = levelToPressed(_rawLevel);
    }

    /* ---------- 2. 按下边沿 ---------- */
    if (_pressed && !_lastPressed)
    {
        _pressTime = now;
        _longPressTriggered = false;
        _lastRepeatTime = 0;
        event = KEY_PRESSED;
        if (_pressedCb)
        {
            _pressedCb();
        }
    }
    /* ---------- 3. 持续按住：长按 ---------- */
    else if (_pressed && _lastPressed)
    {
        if (!_longPressTriggered)
        {
            if (now - _pressTime >= _longPressThreshold)
            {
                _longPressTriggered = true;
                _clickCount = 0; // 长按不参与单击判定
                _lastRepeatTime = now;
                event = KEY_LONG_PRESS;
                if (_longPressCb)
                {
                    _longPressCb();
                }
            }
        }
        else if (_longPressRepeatEnable && (now - _lastRepeatTime >= _longPressRepeatInterval))
        {
            _lastRepeatTime = now;
            event = KEY_LONG_PRESS;
            if (_longPressCb)
            {
                _longPressCb();
            }
        }
    }
    /* ---------- 4. 松开边沿 ---------- */
    else if (!_pressed && _lastPressed)
    {
        event = KEY_RELEASED;
        if (_releasedCb)
        {
            _releasedCb();
        }

        _releaseTime = now;
        if (now - _pressTime < _longPressThreshold)
        {
            if (_clickCount < 2)
            {
                _clickCount++;
            }
        }
        else
        {
            _clickCount = 0; // 长按松开不算点击
        }
    }

    /* ---------- 5. 双击窗口结算 ---------- */
    if (!_pressed && _clickCount > 0 && (now - _releaseTime >= _doubleClickInterval))
    {
        if (_clickCount == 1)
        {
            event = KEY_CLICK;
            if (_clickCb)
            {
                _clickCb();
            }
        }
        else
        {
            event = KEY_DOUBLE_CLICK;
            if (_doubleClickCb)
            {
                _doubleClickCb();
            }
        }
        _clickCount = 0;
    }

    _lastPressed = _pressed;
    return event;
}

void KeyDetector::update(KeyEventCallback callback)
{
    KeyEvent evt = detect();
    if (evt != KEY_NONE && callback != nullptr)
    {
        callback(evt);
    }
}

void KeyDetector::onPressed(KeySimpleCallback cb)
{
    _pressedCb = cb;
}

void KeyDetector::onReleased(KeySimpleCallback cb)
{
    _releasedCb = cb;
}

void KeyDetector::onClick(KeySimpleCallback cb)
{
    _clickCb = cb;
}

void KeyDetector::onDoubleClick(KeySimpleCallback cb)
{
    _doubleClickCb = cb;
}

void KeyDetector::onLongPress(KeySimpleCallback cb)
{
    _longPressCb = cb;
}
