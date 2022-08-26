#include "Oled.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Oled::Oled()
{
}

void Oled::init()
{
    Wire.begin(SDA_PIN, SCL_PIN); //自定义
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS, false, false))
    {
        Serial.println(F("SSD1306 allocation failed"));
    }
    else
    {
        display.display();
        delay(500);
        display.clearDisplay();
    }
}

void Oled::refresh()
{
    display.display();
}

void Oled::cls()
{
    display.clearDisplay();
    //此方法只清除单片机缓存，不会显示在屏幕上，需配合display清屏
}

void Oled::drawText(uint8_t xPos, uint8_t yPos, uint8_t textSize, String txt)
{
    display.setTextSize(textSize); //字体大小倍数，s为1时为6x8，s为2时为12x16
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(xPos, yPos);
    display.print(txt);
}

void Oled::drawText(uint8_t xPos, uint8_t yPos, uint8_t textSize, int num)
{
    display.setTextSize(textSize); //字体大小倍数，s为1时为6x8，s为2时为12x16
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(xPos, yPos);
    display.print(num);
}

void Oled::drawUpArrow(uint8_t x, uint8_t y)
{
    display.fillTriangle(
        1 + x, 45 + y,
        27 + x, 1 + y,
        27 + x, 27 + y, SSD1306_WHITE);
    display.fillTriangle(
        54 + x, 45 + y,
        27 + x, 1 + y,
        27 + x, 27 + y, SSD1306_WHITE);
}

void Oled::drawDownArrow(uint8_t x, uint8_t y)
{
    display.fillTriangle(
        1 + x, 0 + y,
        27 + x, 45 + y,
        27 + x, 18 + y, SSD1306_WHITE);
    display.fillTriangle(
        54 + x, 0 + y,
        27 + x, 45 + y,
        27 + x, 18 + y, SSD1306_WHITE);
}

void Oled::clearArea(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    for (int y = y1; y <= y2; y++)
    {
        for (int x = x1; x <= x2; x++)
        {
            display.drawPixel(x, y, BLACK);
        }
    }
}

void Oled::processUp(int speed)
{
    //原本用于运动时候的屏幕动画的，由于i2c延迟太高导致电机几乎不运转，故停用
    clearArea(0, 12, 40, 56);
    drawUpArrow(8, 24 - currCt);
    display.setCursor(48, 24);
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    clearArea(48, 24, 102, 48);
    display.print(speed);
    display.display();
    currCt++;
    if (currCt > 11)
        currCt = 0;
}

void Oled::processDown(int speed)
{
    //原本用于运动时候的屏幕动画的，由于i2c延迟太高导致电机几乎不运转，故停用
    clearArea(0, 12, 40, 56);
    drawDownArrow(8, 12 + currCt);
    display.setCursor(48, 24);
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    clearArea(48, 24, 102, 48);
    display.print(speed);
    display.display();
    currCt++;
    if (currCt > 11)
        currCt = 0;
}

void Oled::drawZoomIn()
{
    cls();
    drawIco16(arrowIcon, 33, 2, 0);    //左上
    drawIco16(arrowIcon, 77, 2, 1);  //右上
    drawIco16(arrowIcon, 33, 46, 2);   //左下
    drawIco16(arrowIcon, 77, 46, 3); //右下
    display.display();
}

void Oled::drawZoomOut()
{
    cls();
    drawIco16(arrowIcon, 33, 2, 3);    //左上
    drawIco16(arrowIcon, 77, 2, 2);  //右上
    drawIco16(arrowIcon, 33, 46, 1);   //左下
    drawIco16(arrowIcon, 77, 46, 0); //右下
    display.display();
}

void Oled::drawLighting(bool isOn)
{
    cls();
    if(isOn)
    {
        display.fillCircle(63, 31, 24, SSD1306_WHITE);
        display.fillCircle(63, 31, 12, SSD1306_BLACK);
    }
    else
    {
        display.drawCircle(63, 31, 24, SSD1306_WHITE);
        display.drawCircle(63, 31, 12, SSD1306_WHITE);
    }
    display.display();
}


void Oled::drawIco16(const uint16_t *icoData, uint8_t xPos, uint8_t yPos, uint8_t drawFilp)
{
    byte point;
    for (int i = 0; i < 16; i++)
    { 
        for (int j = 0; j < 16; j++)
        {
            // 0正向  1水平翻转 2垂直翻转 3水平和垂直翻转
            switch (drawFilp)
            {
            case 0:
                point = ((icoData[i] >> (15 - j)) & 1);
                display.drawPixel(xPos + j, yPos + i, point ? SSD1306_WHITE : SSD1306_BLACK);
                break;
            case 1:
                point = ((icoData[i] >> (j)) & 1);
                display.drawPixel(xPos + j, yPos + i, point ? SSD1306_WHITE : SSD1306_BLACK);
                break;
            case 2:
                point = ((icoData[15 - i] >> (15 - j)) & 1);
                display.drawPixel(xPos + j, yPos + i, point ? SSD1306_WHITE : SSD1306_BLACK);
                break;
            case 3:
                point = ((icoData[15 - i] >> (j)) & 1);
                display.drawPixel(xPos + j, yPos + i, point ? SSD1306_WHITE : SSD1306_BLACK);
                break;
            default:
                break;
            }
        }
    }
}


void Oled::drawIco32(const uint32_t *icoData, uint8_t xPos, uint8_t yPos, uint8_t drawFilp)
{
    byte point;
    for (int i = 0; i < 32; i++)
    { //行
        for (int j = 0; j < 32; j++)
        {
            // 0正向  1水平翻转 2垂直翻转 3水平和垂直翻转
            switch (drawFilp)
            {
            case 0:
                point = ((icoData[i] >> (31 - j)) & 1);
                display.drawPixel(xPos + j, yPos + i, point ? SSD1306_WHITE : SSD1306_BLACK);
                break;
            case 1:
                point = ((icoData[i] >> (j)) & 1);
                display.drawPixel(xPos + j, yPos + i, point ? SSD1306_WHITE : SSD1306_BLACK);
                break;
            case 2:
                point = ((icoData[31 - i] >> (31 - j)) & 1);
                display.drawPixel(xPos + j, yPos + i, point ? SSD1306_WHITE : SSD1306_BLACK);
                break;
            case 3:
                point = ((icoData[31 - i] >> (j)) & 1);
                display.drawPixel(xPos + j, yPos + i, point ? SSD1306_WHITE : SSD1306_BLACK);
                break;
            default:
                break;
            }
        }
    }
}

void Oled::drawMenuItem(uint8_t index, uint8_t itemId, bool isHighlight)
{
    clearArea(0, index * 22 + 20, 127, index * 22 + 41);
    uint8_t y = index * 22 + 23;
    if (isHighlight)
    {
        display.fillRect(0, index * 22 + 21, 127, 21, SSD1306_WHITE);
    }
    if (itemId == 1)
    {
        drawCNText(0, y, 2, !isHighlight);
        drawCNText(20, y, 3, !isHighlight);
        drawCNText(40, y, 4, !isHighlight);
        drawCNText(60, y, 5, !isHighlight);
    }
    else if (itemId == 2)
    {
        drawCNText(0, y, 6, !isHighlight);
        drawCNText(20, y, 7, !isHighlight);
        drawCNText(40, y, 8, !isHighlight);
        drawCNText(60, y, 9, !isHighlight);
        drawCNText(80, y, 10, !isHighlight);
    }
}

void Oled::drawMenu(uint8_t menuId)
{
    if (menuId == 0)
    {
        drawMenuItem(0,1, currSelectMenuIndex == 0);
        drawMenuItem(1,2, currSelectMenuIndex == 1);
    }
}


void Oled::drawTitle(uint8_t titleId)
{
    clearArea(0, 0, 127, 19);
    display.drawLine(0, 19, 127, 19, SSD1306_WHITE);
    if (titleId == 0)
    {
        drawCNText(0, 0, 0, 1);
        drawCNText(20, 0, 1, 1);
    }
    else if (titleId == 1)
    {
        drawCNText(0, 0, 2, 1);
        drawCNText(20, 0, 3, 1);
        drawCNText(40, 0, 4, 1);
        drawCNText(60, 0, 5, 1);
    }
    else if (titleId == 2)
    {
        drawCNText(0, 0, 6, 1);
        drawCNText(20, 0, 7, 1);
        drawCNText(40, 0, 8, 1);
        drawCNText(60, 0, 9, 1);
        drawCNText(80, 0, 10, 1);
    }
    else if (titleId == 3)
    {
        drawCNText(0, 0, 2, 1);
        drawCNText(20, 0, 3, 1);
        drawCNText(40, 0, 17, 1);
        drawCNText(60, 0, 18, 1);
    }
}

void Oled::drawCNText(uint8_t xPos, uint8_t yPos, uint8_t index, uint8_t color)
{
    byte point;
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            if (j < 8)
            {
                point = ((fonts[index][i * 2] >> (7 - j)) & 1);
            }
            else
            {
                point = ((fonts[index][i * 2 + 1] >> (15 - j)) & 1);
            }
            if (point)
                display.drawPixel(xPos + j, yPos + i, color);
        }
    }
}


void Oled::drawReady()
{
    cls();
    drawIco32(bigFonts[0], 28, 16, 0);
    drawIco32(bigFonts[1], 68, 16, 0);
    refresh();
}

