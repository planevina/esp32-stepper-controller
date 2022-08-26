#ifndef OLED_H
#define OLED_H

#define SSD1306_NO_SPLASH //去掉开机logo
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "cnfont.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define SDA_PIN 4
#define SCL_PIN 5


const uint16_t arrowIcon[16] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFE00, 0xFF00, 0xFF80, 0xF7C0, 0xF3E0, 0xF1F0, 0xF0F8, 0xF07C, 0xF03E, 0xF01F, 0xF00F, 0xF007};

class Oled
{
private:
public:
    /**
     * 构造函数
     */
    Oled();

    //用作动画计数的，暂未使用
    uint8_t currCt = 0;

    //当前菜单索引
    uint8_t currSelectMenuIndex = 0;

    /**
     * 初始化
     */
    void init();

    /**
     * 清空显示
     */
    void clearShow();

    /**
     * 清空显示
     */
    void cls();

    /**
     * 刷新显示
     */
    void refresh();

    /**
     * 显示文字
     */
    void drawText(uint8_t xPos, uint8_t yPos, uint8_t textSize, String txt);

    void drawText(uint8_t xPos, uint8_t yPos, uint8_t textSize, int num);

    /**
     * 显示中文字
     */
    void drawCNText(uint8_t xPos, uint8_t yPos, uint8_t index, uint8_t color);
    /**
     * 显示上箭头
     */
    void drawUpArrow(uint8_t x, uint8_t y);

    /**
     * 显示下箭头
     */
    void drawDownArrow(uint8_t x, uint8_t y);

    /**
     * 显示照明
     */
    void drawLighting(bool isOn);

    /**
     * 显示放大
     */
    void drawZoomIn();

    /**
     * 显示缩小
     */
    void drawZoomOut();

    /**
     * 清除指定区域
     */
    void clearArea(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

    /**
     * 显示16 x 16模型
     * 参数16长度的数组，起点xy坐标， 和绘制方法 0正向  1水平翻转 2垂直翻转 3水平和垂直翻转
     */
    void drawIco16(const uint16_t *icoData, uint8_t xPos, uint8_t yPos, uint8_t drawFilp);

    /**
     * 显示32 x 32模型
     * 参数32长度的数组，起点xy坐标， 和绘制方法 0正向  1水平翻转 2垂直翻转 3水平和垂直翻转
     */
    void drawIco32(const uint32_t *icoData, uint8_t xPos, uint8_t yPos, uint8_t drawFilp);

    void drawReady();

    /**
     * 显示菜单
     */
    void drawMenu(uint8_t menuId);

    void drawMenuItem(uint8_t index, uint8_t itemId, bool isHighlight);

    /**
     * 显示标题
     */
    void drawTitle(uint8_t titleId);

    void processUp(int speed);

    void processDown(int speed);

    void processZoomIn();

    void processZoomOut();
};

#endif
