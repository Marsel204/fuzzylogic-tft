#pragma once
#include <Arduino.h>
#include <vector>
#define TFT_YELLOW 1
#define TFT_SKYBLUE 2
#define TFT_GREEN 3
struct TftDraw {
    std::string kind, text;
    int x, y, w, h, radius, font;
    uint16_t color, background;
};
struct TFT_eSPI {
    std::vector<TftDraw> commands;
    bool pressed = false;
    uint16_t touchX = 0, touchY = 0;
    int calibrations = 0;
    uint16_t foreground = 0, background = 0;
    void init() {}
    void setRotation(int) {}
    void record(const char* kind, int x, int y, int w, int h, int radius, uint16_t color) {
        commands.push_back({kind, "", x, y, w, h, radius, 0, color, 0});
    }
    void fillScreen(uint16_t c) { commands.clear(); record("rect", 0, 0, 320, 240, 0, c); }
    void fillRoundRect(int x,int y,int w,int h,int r,uint16_t c) { record("round",x,y,w,h,r,c); }
    void drawRoundRect(int x,int y,int w,int h,int r,uint16_t c) { record("outline",x,y,w,h,r,c); }
    void fillRect(int x,int y,int w,int h,uint16_t c) { record("rect",x,y,w,h,0,c); }
    void drawFastHLine(int x,int y,int w,uint16_t c) { record("rect",x,y,w,1,0,c); }
    void drawCircle(int x,int y,int r,uint16_t c) { record("circle",x,y,0,0,r,c); }
    void drawLine(int x,int y,int x2,int y2,uint16_t c) { record("line",x,y,x2,y2,0,c); }
    void setTextColor(uint16_t f,uint16_t b) { foreground=f; background=b; }
    int fontHeight(int font) const { return font==4?26:font==2?16:8; }
    int textWidth(const char* text,int font) const {
        static const uint8_t font2[96] = {6,3,4,9,8,9,9,3,7,7,8,6,3,6,5,7,8,8,8,8,8,8,8,8,8,8,3,3,6,6,6,8,9,8,8,8,8,8,8,8,8,4,8,8,7,10,8,8,8,8,8,8,8,8,8,10,8,8,8,4,7,4,7,9,5,7,7,7,7,7,6,7,7,4,5,6,4,8,7,8,7,8,6,6,5,7,8,8,6,7,7,5,3,5,8,6};
        static const uint8_t font4[96] = {5,8,8,19,14,21,17,6,8,8,12,10,7,8,7,8,14,14,14,14,14,14,14,14,14,14,7,7,14,9,14,13,25,16,17,18,18,16,15,19,18,6,13,17,13,21,18,19,16,19,17,16,14,18,15,23,15,16,16,9,13,9,12,13,9,14,15,13,15,14,8,15,15,6,6,12,6,22,15,15,15,15,8,12,7,14,12,18,13,13,12,13,13,13,15,1};
        int width=0;
        for (const unsigned char* p=(const unsigned char*)text;*p;++p) width+=font==1?6:*p>=32&&*p<128?(font==2?font2[*p-32]:font4[*p-32]):0;
        return width;
    }
    void drawString(const char* text,int x,int y,int font) {
        commands.push_back({"text",text,x,y,textWidth(text,font),fontHeight(font),0,font,foreground,background});
    }
    void drawCentreString(const char* text,int x,int y,int font) { drawString(text,x-textWidth(text,font)/2,y,font); }
    void drawRightString(const char* text,int x,int y,int font) { drawString(text,x-textWidth(text,font),y,font); }
    bool getTouch(uint16_t* x,uint16_t* y) { *x=touchX;*y=touchY;return pressed; }
    void setTouch(uint16_t*) {}
    void calibrateTouch(uint16_t* data,uint32_t,uint32_t,uint8_t) {
        ++calibrations;
        uint16_t values[5]={300,3500,300,3500,7};
        memcpy(data,values,sizeof(values));
    }
};
