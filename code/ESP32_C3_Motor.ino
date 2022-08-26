#include <OneButton.h>
#include <AccelStepper.h>
#include "Oled.h"
#include "AdcCalibration.h"
#include "NvsTool.h"

//  全步进- 1.8° | 半步进- 0.9° | 1/4步进- 0.45° | 1/8步进- 0.225°
//  一圈：  200步 | 400步 | 800步 | 1600步
#define driveMode 1

//电机引脚定义
#define xStepPin 10
#define xDirPin 8
#define yStepPin 7
#define yDirPin 6
#define motorEnPin 19

//初始化电机
AccelStepper stepperX(driveMode, xStepPin, xDirPin); //倍率调整
AccelStepper stepperY(driveMode, yStepPin, yDirPin); //上下调整

//摇杆引脚定义
#define analogStickXPin 0 // A0
#define analogStickYPin 1 // A1
#define keyPin 2

//手柄的初始化数据
float xMin = 10.0;      // x轴最小
float yMin = 10.0;      // y轴最小
float xCenter = 1400.0; // x轴中心
float yCenter = 1400.0; // y轴中心
float xMax = 3800.0;    // x轴Max值
float yMax = 3800.0;    // y轴Max值
float deadZone = 300.0; //中心死区

#define ledPin 18 // Led照明引脚定义（高电平开启）

int xVal, yVal;                                               //摇杆采样值
float xSpeed, ySpeed, xMaxSpeed = 1000.0, yMaxSpeed = 1000.0; //电机当前速度和最高速度
float xRate = 0, yRate = 0;                                   //电机当前速度倍率

uint32_t timeFlag = 0;                       //计数锁定的时间戳
uint32_t clickTime = 0;                      //按键长按时间
uint32_t sleepMs = 180000;                   //电机锁定时间3分钟
uint8_t ledEn = 0;                           //是否开启照明
bool caliOK = false;                         //是否完成校准
uint32_t caliResult[6] = {0, 0, 0, 0, 0, 0}; //校准结果，顺序是 xmin center max ymin center max
uint8_t modeFlag = 0;
uint8_t menuLength[1] = {2};
uint8_t analogStickStatus = 0; //在菜单中记录摇杆上下左右的值

enum FunctionEnum
{
    FUNC_LENS,         //镜头操作模式
    FUNC_SETTING_MENU, //设置模式菜单
    FUNC_STICK_SENSOR, //显示调试信息
    FUNC_STICK_CALI    //校准摇杆
};

enum MotionEnum
{
    LENS_READY,    //就绪
    LENS_UP,       //镜头上移
    LENS_DOWN,     //镜头下移
    LENS_ZOOM_IN,  //倍率放大
    LENS_ZOOM_OUT, //倍率缩小
    LENS_LOCK      //镜头锁定
};

FunctionEnum currFunc = FUNC_LENS;  //当前功能
MotionEnum lastMotion = LENS_READY; //上次镜头状态
MotionEnum currMotion = LENS_READY; //当前镜头状态

OneButton btnKey = OneButton(keyPin, true, true); //摇杆按键初始化，按下为低电平
Oled oled = Oled();                               // Oled初始化

esp_err_t err = nvs_flash_init(); //存储空间初始化

/**
   @brief 摇杆单击
*/
void singleKeyClickHandler()
{
    if (currFunc == FUNC_LENS)
    {
        //在镜头模式中开关照明
        if (ledEn)
        {
            ledEn = 0;
            digitalWrite(ledPin, LOW);
            oled.drawLighting(false);
            delay(100);
        }
        else
        {
            ledEn = 1;
            digitalWrite(ledPin, HIGH);
            oled.drawLighting(true);
            delay(100);
        }
        timeFlag = millis();
    }
    else if (currFunc == FUNC_SETTING_MENU)
    {
        //在菜单模式中，进入对应菜单
        if (oled.currSelectMenuIndex == 0)
        {
            currFunc = FUNC_STICK_CALI;
            caliOK = false;
            digitalWrite(motorEnPin, LOW);
            oled.cls();
            oled.drawTitle(1);
            oled.refresh();
        }
        else if (oled.currSelectMenuIndex == 1)
        {
            currFunc = FUNC_STICK_SENSOR;
            digitalWrite(motorEnPin, LOW);
            oled.cls();
            oled.drawTitle(2);
            oled.refresh();
        }
    }
    else if (currFunc == FUNC_STICK_CALI)
    {
        //在校准模式中，保存校准值
        if (caliOK)
        {
            //已完成校准，保存
            setNvsValueU32("xMin", caliResult[0]);
            setNvsValueU32("xCenter", caliResult[1]);
            setNvsValueU32("xMax", caliResult[2]);
            setNvsValueU32("yMin", caliResult[3]);
            setNvsValueU32("yCenter", caliResult[4]);
            setNvsValueU32("yMax", caliResult[5]);
            xMin = caliResult[0];
            xCenter = caliResult[1];
            xMax = caliResult[2];
            yMin = caliResult[3];
            yCenter = caliResult[4];
            yMax = caliResult[5];
            oled.cls();
            oled.drawCNText(24, 24, 11, 1);
            oled.drawCNText(44, 24, 12, 1);
            oled.drawCNText(64, 24, 17, 1);
            oled.drawCNText(84, 24, 18, 1);
            oled.refresh();
            delay(2000);
        }
        currFunc = FUNC_LENS;
        currMotion = LENS_READY;
        lastMotion = LENS_READY;
        digitalWrite(motorEnPin, LOW);
        delay(100);
        oled.drawReady();
        timeFlag = millis();
    }
}

/**
   @brief 摇杆双击
*/
void doubleKeyClickHandler()
{
    //镜头模式下，锁定/解锁电机
    if (currFunc == FUNC_LENS)
    {
        if (currMotion == LENS_LOCK)
        {
            currMotion = LENS_READY;
            lastMotion = LENS_READY;
            digitalWrite(motorEnPin, LOW);
            oled.drawReady();
            timeFlag = millis();
            delay(100);
        }
        else
        {
            currMotion = LENS_LOCK;
            digitalWrite(motorEnPin, HIGH);
            oled.cls();
            oled.drawCNText(24, 24, 27, 1);
            oled.drawCNText(44, 24, 28, 1);
            oled.drawCNText(64, 24, 31, 1);
            oled.drawCNText(84, 24, 32, 1);
            oled.refresh();
        }
    }
}

/**
   @brief 长按开始

*/
void longKeyClickStartHandler()
{
    // Serial.println("长按开始");
    clickTime = millis();
}

/**
   @brief 长按结束

*/
void longKeyClickHandler()
{
    // Serial.println("长按结束");
    clickTime = millis() - clickTime + 800; // 800是默认的长按阈值
    if (clickTime > 5000)
    {
        //长按5秒以上

        timeFlag = millis();
    }
    else
    {
        //短长按，进入或者退出设置模式
        if (currFunc == FUNC_LENS)
        {
            //进入设置
            currFunc = FUNC_SETTING_MENU;
            modeFlag = 0;
            digitalWrite(motorEnPin, HIGH);
        }
        else
        {
            //退出设置
            currFunc = FUNC_LENS;
            currMotion = LENS_READY;
            lastMotion = LENS_READY;
            digitalWrite(motorEnPin, LOW);
            delay(100);
            oled.drawReady();
            timeFlag = millis();
        }
    }
    clickTime = 0;
}

/**
   @brief 长按持续中事件

*/
void longKeyDuringPressHandler()
{
}

/**
   @brief 初始化按键信息

*/
void initBtns()
{
    btnKey.reset();
    btnKey.setPressTicks(800); //按住800毫秒进入长按状态
    btnKey.attachClick(singleKeyClickHandler);
    btnKey.attachDoubleClick(doubleKeyClickHandler); // 添加双击事件函数
    btnKey.attachLongPressStart(longKeyClickStartHandler);
    // btnKey.attachDuringLongPress(longKeyDuringPressHandler);
    btnKey.attachLongPressStop(longKeyClickHandler);
}

/**
   @brief 初始化步进电机

*/
void initMotors()
{
    //初始化引脚状态
    pinMode(motorEnPin, OUTPUT);
    //初始化结束之前先禁用步进电机
    digitalWrite(motorEnPin, HIGH);
    pinMode(ledPin, OUTPUT);
    pinMode(yStepPin, OUTPUT);
    pinMode(yDirPin, OUTPUT);
    pinMode(xStepPin, OUTPUT);
    pinMode(xDirPin, OUTPUT);
    //初始化电机速度
    xSpeed = 0.0;
    stepperX.setMaxSpeed(xMaxSpeed);
    stepperX.setAcceleration(20.0); // runSpeed用不到这个
    stepperX.setSpeed(xSpeed);
    ySpeed = 0.0;
    stepperY.setMaxSpeed(yMaxSpeed);
    stepperY.setAcceleration(20.0); // runSpeed用不到这个
    stepperY.setSpeed(ySpeed);
}

/**
   @brief 初始化摇杆信息

*/
void initStick()
{
    //从nvs中读取数据（如有）
    xMin = getNvsValueU32("xMin", 10);
    yMin = getNvsValueU32("yMin", 10);
    xCenter = getNvsValueU32("xCenter", 1400);
    yCenter = getNvsValueU32("yCenter", 1400);
    xMax = getNvsValueU32("xMax", 3800);
    yMax = getNvsValueU32("yMax", 3800);
}

void setup()
{

    initMotors(); //初始化电机
    Serial.begin(115200);
    oled.init(); //初始化屏幕
    initBtns();  //初始化按键
    initStick(); //初始化摇杆

    //校准采样
    cali_enable = adc_calibration_init();
    // analogSetAttenuation(ADC_11db); 全局采样衰减
    // ADC_11db       max2500mv
    // ADC_6db        max1350mv
    // ADC_2_5db      max1100mv
    // ADC_0db        max800mv
    // 5v * 1/(1+5.49) = 774mv,所以用0db挡位
    //单个pin采样衰减
    analogSetPinAttenuation(analogStickXPin, ADC_0db);
    analogSetPinAttenuation(analogStickYPin, ADC_0db);

    delay(500);
    oled.drawReady();
    //启用步进电机
    digitalWrite(motorEnPin, LOW);
    timeFlag = millis();
}

/**
   @brief 按键中断处理

*/
void btnLoop()
{
    btnKey.tick();
}

/**
   @brief 功能处理

*/
void functionLoop()
{
    switch (currFunc)
    {
    case FUNC_LENS:
        motorLoop();
        break;
    case FUNC_STICK_CALI:
        caliLoop();
        break;
    case FUNC_STICK_SENSOR:
        sensorLoop();
        break;
    case FUNC_SETTING_MENU:
        menuLoop();
        break;
    default:
        break;
    }
}

/**
   @brief motorLoop

*/
void motorLoop()
{
    if (currMotion == LENS_LOCK)
        return;
    if (millis() - timeFlag >= sleepMs)
    {
        //电机锁定
        currMotion = LENS_LOCK;
        digitalWrite(motorEnPin, HIGH);
        //显示电机锁定
        oled.cls();
        oled.drawCNText(24, 24, 27, 1);
        oled.drawCNText(44, 24, 28, 1);
        oled.drawCNText(64, 24, 31, 1);
        oled.drawCNText(84, 24, 32, 1);
        oled.refresh();
        return;
    }
    //读取摇杆值
    xVal = analogRead(analogStickXPin);
    yVal = analogRead(analogStickYPin);

    //计算速度比率
    if (xVal < xCenter - deadZone)
    {
        xRate = (xCenter - xVal) / (xCenter - xMin);
        if (xRate > 1)
            xRate = 1;
        xSpeed = xRate * xMaxSpeed;
    }
    else if (xVal > xCenter + deadZone)
    {
        xRate = (xVal - xCenter) / (xMax - xCenter);
        if (xRate > 1)
            xRate = 1;
        xSpeed = -xRate * xMaxSpeed; //负数
    }
    else
    {
        xRate = 0;
        xSpeed = 0;
    }

    if (yVal < yCenter - deadZone)
    {
        yRate = (yCenter - yVal) / (yCenter - yMin);
        if (yRate > 1)
            yRate = 1;
        ySpeed = yRate * yMaxSpeed; //正数，上升
    }
    else if (yVal > yCenter + deadZone)
    {
        yRate = (yVal - yCenter) / (yMax - yCenter);
        if (yRate > 1)
            yRate = 1;
        ySpeed = -yRate * yMaxSpeed; //负数
    }
    else
    {
        yRate = 0;
        ySpeed = 0;
    }
    //如果都有值，Rate小的方向被舍弃（本项目不需要在调整物距的同时还调整倍率）
    if (xRate > 0 && yRate > 0)
    {
        if (yRate >= xRate)
        {
            xRate = 0;
            xSpeed = 0;
        }
        else
        {
            yRate = 0;
            ySpeed = 0;
        }
    }

    if (xSpeed > 0)
    {
        currMotion = LENS_ZOOM_OUT;
    }
    else if (xSpeed < 0)
    {
        currMotion = LENS_ZOOM_IN;
    }
    else if (ySpeed > 0)
    {
        currMotion = LENS_DOWN;
    }
    else if (ySpeed < 0)
    {
        currMotion = LENS_UP;
    }
    else
    {
        currMotion = LENS_READY;
    }
    stepperX.setSpeed(xSpeed);
    stepperY.setSpeed(ySpeed);
    if (currMotion == LENS_UP)
    {
        if (lastMotion != currMotion)
        {
            //i2c的oled显示刷新会占用20-30ms，严重影响电机运转，故只在第一次进入的时候刷新屏幕
            lastMotion = currMotion;
            oled.cls();
            oled.drawUpArrow(36, 8);
            oled.refresh();
        }
        stepperY.runSpeed();
        timeFlag = millis();
    }
    else if (currMotion == LENS_DOWN)
    {
        if (lastMotion != currMotion)
        {
            lastMotion = currMotion;
            oled.cls();
            oled.drawDownArrow(36, 8);
            oled.refresh();
        }
        stepperY.runSpeed();
        timeFlag = millis();
    }
    else if (currMotion == LENS_ZOOM_IN)
    {
        if (lastMotion != currMotion)
        {
            lastMotion = currMotion;
            oled.drawZoomIn();
        }
        stepperX.runSpeed();
        timeFlag = millis();
    }
    else if (currMotion == LENS_ZOOM_OUT)
    {
        if (lastMotion != currMotion)
        {
            lastMotion = currMotion;
            oled.drawZoomOut();
        }
        stepperX.runSpeed();
        timeFlag = millis();
    }
    else
    {
        if (lastMotion != currMotion)
        {
            lastMotion = currMotion;
            oled.drawReady();
        }
    }
}

void loop()
{
    btnLoop();
    functionLoop();
}

void sensorLoop()
{
    xVal = analogRead(analogStickXPin);
    yVal = analogRead(analogStickYPin);
    // Serial.print("X= ");
    // Serial.print(xVal);
    // Serial.print("，Y= ");
    // Serial.println(yVal);
    oled.clearArea(0, 20, 127, 63);
    oled.drawText(0, 23, 2, "X:");
    oled.drawText(24, 23, 2, xVal);
    oled.drawText(0, 45, 2, "Y:");
    oled.drawText(24, 45, 2, yVal);
    if (cali_enable)
    {
        uint32_t xVoltage = esp_adc_cal_raw_to_voltage(xVal, &adc1_chars);
        uint32_t yVoltage = esp_adc_cal_raw_to_voltage(yVal, &adc2_chars);
        // Serial.print("X Votage= ");
        // Serial.print(xVoltage);
        // Serial.print(",Y Votage= ");
        // Serial.println(yVoltage);
        oled.drawText(72, 23, 2, ",");
        oled.drawText(84, 23, 2, xVoltage);
        oled.drawText(72, 45, 2, ",");
        oled.drawText(84, 45, 2, yVoltage);
    }
    oled.refresh();
    delay(10);
}

void menuLoop()
{
    if (modeFlag != 99)
    {
        modeFlag = 99;
        oled.cls();
        oled.drawTitle(0);
        oled.drawMenu(0);
        oled.refresh();
    }
    //获取摇杆信息，x轴暂不使用
    yVal = analogRead(analogStickYPin);
    if (yVal < yMin + 200)
    {
        //按住了下
        analogStickStatus = 2;
    }
    else if (yVal > yMax - 200)
    {
        //按住了上
        analogStickStatus = 1;
    }
    else if (yVal > yCenter - deadZone && yVal < yCenter + deadZone)
    {
        //判断是否松开
        if (analogStickStatus == 1)
        {
            if (oled.currSelectMenuIndex == 0)
                oled.currSelectMenuIndex = menuLength[0] - 1;
            else
                oled.currSelectMenuIndex -= 1;
            modeFlag = 0;
            analogStickStatus = 0;
        }
        else if (analogStickStatus == 2)
        {
            oled.currSelectMenuIndex++;
            if (oled.currSelectMenuIndex >= menuLength[0])
                oled.currSelectMenuIndex = 0;
            modeFlag = 0; //重绘菜单
            analogStickStatus = 0;
        }
    }
}

void caliLoop()
{
    if (caliOK)
    {
        return;
    }
    //校准摇杆
    //步骤1、采样记录50次xy摇杆不动时的值，求平均数，此为中心值
    oled.drawCNText(0, 23, 2, 1);
    oled.drawCNText(20, 23, 3, 1);
    oled.drawCNText(40, 23, 19, 1);
    oled.drawCNText(60, 23, 33, 1);
    oled.drawCNText(80, 23, 34, 1);
    oled.drawCNText(0, 45, 23, 1);
    oled.drawCNText(20, 45, 24, 1);
    oled.drawCNText(40, 45, 25, 1);
    oled.drawCNText(60, 45, 26, 1);
    oled.drawCNText(80, 45, 4, 1);
    oled.drawCNText(100, 45, 5, 1);
    oled.refresh();
    delay(3000);
    uint32_t xCaliCenter = 0, yCaliCenter = 0;
    for (int i = 0; i < 50; i++)
    {
        xVal = analogRead(analogStickXPin);
        yVal = analogRead(analogStickYPin);
        xCaliCenter += xVal;
        yCaliCenter += yVal;
        oled.clearArea(0, 23, 127, 63);
        oled.drawText(0, 24, 1, "X:");
        oled.drawText(12, 24, 1, xVal);
        oled.drawText(0, 38, 1, "Y:");
        oled.drawText(12, 38, 1, yVal);
        oled.refresh();
        delay(20);
    }

    xCaliCenter /= 50;
    yCaliCenter /= 50;

    //步骤2、旋转摇杆几圈，记录xy轴最大值和最小值
    oled.clearArea(0, 23, 127, 63);
    oled.drawCNText(0, 23, 35, 1);
    oled.drawCNText(20, 23, 36, 1);
    oled.drawCNText(40, 23, 20, 1);
    oled.drawCNText(60, 23, 21, 1);
    oled.drawCNText(80, 23, 4, 1);
    oled.drawCNText(100, 23, 5, 1);
    oled.drawText(0, 45, 2, "3-5");
    oled.drawCNText(40, 45, 22, 1);
    oled.refresh();
    delay(3000);
    uint32_t xCaliMax = xCaliCenter, xCaliMin = xCaliCenter, yCaliMax = yCaliCenter, yCaliMin = yCaliCenter;

    timeFlag = millis(); // 10秒超时
    while (millis() - timeFlag < 10000)
    {
        xVal = analogRead(analogStickXPin);
        yVal = analogRead(analogStickYPin);
        if (xCaliMax < xVal)
            xCaliMax = xVal;
        if (xCaliMin > xVal)
            xCaliMin = xVal;
        if (yCaliMax < yVal)
            yCaliMax = yVal;
        if (yCaliMin > yVal)
            yCaliMin = yVal;
        oled.clearArea(0, 23, 127, 63);
        oled.drawText(0, 24, 1, "X Max:");
        oled.drawText(36, 24, 1, xCaliMax);
        oled.drawText(64, 24, 1, "Min:");
        oled.drawText(88, 24, 1, xCaliMin);
        oled.drawText(0, 38, 1, "Y Max:");
        oled.drawText(36, 38, 1, yCaliMax);
        oled.drawText(64, 38, 1, "Min:");
        oled.drawText(88, 38, 1, yCaliMin);
        oled.refresh();
    }

    //步骤3、显示结果并记录
    oled.cls();
    oled.drawTitle(3);
    oled.drawText(0, 23, 1, "X");
    oled.drawText(11, 23, 1, xCaliMin);
    oled.drawText(47, 23, 1, xCaliCenter);
    oled.drawText(83, 23, 1, xCaliMax);
    oled.drawText(0, 33, 1, "Y");
    oled.drawText(11, 33, 1, yCaliMin);
    oled.drawText(47, 33, 1, yCaliCenter);
    oled.drawText(83, 33, 1, yCaliMax);
    oled.drawCNText(0, 45, 37, 1);
    oled.drawCNText(16, 45, 38, 1);
    oled.drawCNText(32, 45, 11, 1);
    oled.drawCNText(48, 45, 12, 1);
    oled.drawCNText(64, 45, 41, 1);
    oled.drawCNText(80, 45, 42, 1);
    oled.drawCNText(96, 45, 39, 1);
    oled.drawCNText(112, 45, 40, 1);
    oled.refresh();
    caliResult[0] = xCaliMin;
    caliResult[1] = xCaliCenter;
    caliResult[2] = xCaliMax;
    caliResult[3] = yCaliMin;
    caliResult[4] = yCaliCenter;
    caliResult[5] = yCaliMax;
    caliOK = true;
}
