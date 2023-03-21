#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <DNSServer.h>  //密码直连将其三个库注释
#include <ESP8266WebServer.h>
#include <CustomWiFiManager.h>

#include <time.h>
#include <sys/time.h>
#include <coredecls.h>


//#include "SH1106Wire.h"   //1.3寸用这个
#include "SSD1306Wire.h"  //0.96寸用这个
#include "OLEDDisplayUi.h"
#include "HeFeng.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"

/***************************
   Begin Settings
 **************************/

//const char *WIFI_SSID = "Alex";  //填写你的WIFI名称及密码
//const char *WIFI_PWD = "66666666";

const char *BILIBILIID = "634795344";
;  //填写你的B站账号

//由于太多人使用我的秘钥，导致获取次数超额，所以不提供秘钥了，大家可以到https://dev.heweather.com/获取免费的
const char *HEFENG_KEY = "d5ea2c715bf7484f93ac6de5f8496914";  //填写你的和风天气秘钥
const char *HEFENG_LOCATION = "101040100";                    //填写你的城市ID,可到https://where.heweather.com/index.html查询
//const char* HEFENG_LOCATION = "auto_ip";//自动IP定位

#define TZ 8      // 中国时区为8
#define DST_MN 0  // 默认为0

const int UPDATE_INTERVAL_SECS = 30 * 60;       // 30分钟更新一次天气
const int UPDATE_CURR_INTERVAL_SECS = 60 * 59;  // 60分钟更新一次粉丝数

const int I2C_DISPLAY_ADDRESS = 0x3c;  //I2c地址默认
#if defined(ESP8266)
const int SDA_PIN = 0;  //引脚连接
const int SDC_PIN = 2;  //
#endif

const String WDAY_NAMES[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };                                      //星期
const String MONTH_NAMES[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };  //月份

// web配网页面自定义我的图标请随便使用一个图片转base64工具转换https://tool.css-js.com/base64.html, 64*64
const char Icon[] PROGMEM = "data:image/png;base64,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";

/***************************
   End Settings
 **************************/

//SH1106Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);   // 1.3寸用这个
SSD1306Wire display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);  // 0.96寸用这个
OLEDDisplayUi ui(&display);

HeFengCurrentData currentWeather;  //实例化对象
HeFengForeData foreWeather[3];
HeFeng HeFengClient;

#define TZ_MN ((TZ)*60)  //时间换算
#define TZ_SEC ((TZ)*3600)
#define DST_SEC ((DST_MN)*60)

time_t now;  //实例化时间

bool readyForWeatherUpdate = false;  // 天气更新标志
bool first = true;                   //首次更新标志
long timeSinceLastWUpdate = 0;       //上次更新后的时间
long timeSinceLastCurrUpdate = 0;    //上次天气更新后的时间

String fans = "-1";  //粉丝数

void drawProgress(OLEDDisplay *display, int percentage, String label);  //提前声明函数
void updateData(OLEDDisplay *display);
void updateDatas(OLEDDisplay *display);
void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawForecast(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex);
void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState *state);
void setReadyForWeatherUpdate();

//添加框架
//此数组保留指向所有帧的函数指针
//框架是从右向左滑动的单个视图
FrameCallback frames[] = { drawDateTime, drawCurrentWeather, drawForecast };
//页面数量
int numberOfFrames = 3;

OverlayCallback overlays[] = { drawHeaderOverlay };  //覆盖回调函数
int numberOfOverlays = 1;                            //覆盖数

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // 屏幕初始化
  display.init();
  display.clear();
  display.display();

  display.flipScreenVertically();  //屏幕翻转
  display.setContrast(120);        //屏幕亮度

  //Web配网，密码直连请注释
  webconnect();

  // 用固定密码连接，Web配网请注释
  //wificonnect();

  ui.setTargetFPS(30);  //刷新频率

  ui.setActiveSymbol(activeSymbole);      //设置活动符号
  ui.setInactiveSymbol(inactiveSymbole);  //设置非活动符号

  // 符号位置
  // 你可以把这个改成TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // 定义第一帧在栏中的位置
  ui.setIndicatorDirection(LEFT_RIGHT);

  // 屏幕切换方向
  // 您可以更改使用的屏幕切换方向 SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  ui.setFrames(frames, numberOfFrames);  // 设置框架
  ui.setTimePerFrame(8000);              //设置切换时间

  ui.setOverlays(overlays, numberOfOverlays);  //设置覆盖

  // UI负责初始化显示
  ui.init();
  display.flipScreenVertically();  //屏幕反转

  configTime(TZ_SEC, DST_SEC, "ntp.ntsc.ac.cn", "ntp1.aliyun.com");  //ntp获取时间，你也可用其他"pool.ntp.org","0.cn.pool.ntp.org","1.cn.pool.ntp.org","ntp1.aliyun.com"
  delay(200);
}

void loop() {
  if (first) {  //首次加载
    updateDatas(&display);
    first = false;
  }
  if (millis() - timeSinceLastWUpdate > (1000L * UPDATE_INTERVAL_SECS)) {  //屏幕刷新
    setReadyForWeatherUpdate();
    timeSinceLastWUpdate = millis();
  }
  if (millis() - timeSinceLastCurrUpdate > (1000L * UPDATE_CURR_INTERVAL_SECS)) {  //粉丝数更新
    HeFengClient.fans(&currentWeather, BILIBILIID);
    fans = String(currentWeather.follower);
    timeSinceLastCurrUpdate = millis();
  }

  if (readyForWeatherUpdate && ui.getUiState()->frameState == FIXED) {  //天气更新
    updateData(&display);
  }

  int remainingTimeBudget = ui.update();  //剩余时间预算

  if (remainingTimeBudget > 0) {
    //你可以在这里工作如果你低于你的时间预算。
    delay(remainingTimeBudget);
  }
}

// void wificonnect() {  //WIFI密码连接，Web配网请注释
//   WiFi.begin(WIFI_SSID, WIFI_PWD);
//   while (WiFi.status() != WL_CONNECTED) {
//     Serial.print('.');
//     delay(80);
//     display.clear();
//     display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_5);
//     display.display();
//     delay(80);
//     display.clear();
//     display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_6);
//     display.display();
//     delay(80);
//     display.clear();
//     display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_7);
//     display.display();
//     delay(80);
//     display.clear();
//     display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_8);
//     display.display();
//     delay(80);
//     display.clear();
//     display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_1);
//     display.display();
//     delay(80);
//     display.clear();
//     display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_2);
//     display.display();
//     delay(80);
//     display.clear();
//     display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_3);
//     display.display();
//     delay(80);
//     display.clear();
//     display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_4);
//     display.display();
//   }
//   Serial.println("");
//   delay(500);
// }

void webconnect() {  ////Web配网，密码直连将其注释
 display.clear();
 display.drawXbm(0, 0, 128, 64, bilibili); //显示哔哩哔哩
 display.display();

 WiFiManager wifiManager;  //实例化WiFiManager
 wifiManager.setDebugOutput(false); //关闭Debug
 //wifiManager.setConnectTimeout(10); //设置超时
 wifiManager.setHeadImgBase64(FPSTR(Icon)); //设置图标
 wifiManager.setPageTitle("欢迎来到您的多功能显示设备的WiFi配置页");  //设置页标题

 if (!wifiManager.autoConnect("esp8266_WifiClock")) {  //AP模式/设置wifi名称
   Serial.println("连接失败并超时");
   //重新设置并再试一次，或者让它进入深度睡眠状态
   ESP.restart();
   delay(1000);
 }
 Serial.println("connected...yo");
 yield();
}

void drawProgress(OLEDDisplay *display, int percentage, String label) {  //绘制进度
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);  //alignment 对齐
  display->setFont(ArialMT_Plain_10);            //设置字体
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

void updateData(OLEDDisplay *display) {  //天气更新
  HeFengClient.doUpdateCurr(&currentWeather, HEFENG_KEY, HEFENG_LOCATION);
  HeFengClient.doUpdateFore(foreWeather, HEFENG_KEY, HEFENG_LOCATION);
  readyForWeatherUpdate = false;
}

void updateDatas(OLEDDisplay *display) {  //首次天气更新
  drawProgress(display, 0, "Updating fansnumb...");
  HeFengClient.fans(&currentWeather, BILIBILIID);
  fans = String(currentWeather.follower);

  drawProgress(display, 33, "Updating weather...");
  HeFengClient.doUpdateCurr(&currentWeather, HEFENG_KEY, HEFENG_LOCATION);

  drawProgress(display, 66, "Updating forecasts...");
  HeFengClient.doUpdateFore(foreWeather, HEFENG_KEY, HEFENG_LOCATION);

  readyForWeatherUpdate = false;
  drawProgress(display, 100, "Done...");
  delay(200);
}

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {  //显示时间
  now = time(nullptr);
  struct tm *timeInfo;
  timeInfo = localtime(&now);
  char buff[16];

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  String date = WDAY_NAMES[timeInfo->tm_wday];

  sprintf_P(buff, PSTR("%04d-%02d-%02d  %s"), timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday, WDAY_NAMES[timeInfo->tm_wday].c_str());
  display->drawString(64 + x, 5 + y, String(buff));
  display->setFont(ArialMT_Plain_24);

  sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  display->drawString(64 + x, 22 + y, String(buff));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {  //显示天气
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 38 + y, currentWeather.cond_txt + "    |   Wind: " + currentWeather.wind_sc + "  ");

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  String temp = currentWeather.tmp + "°C";
  display->drawString(60 + x, 3 + y, temp);
  display->setFont(ArialMT_Plain_10);
  display->drawString(62 + x, 26 + y, currentWeather.fl + "°C | " + currentWeather.hum + "%");
  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(32 + x, 0 + y, currentWeather.iconMeteoCon);
}

void drawForecast(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y) {  //天气预报
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 1);
  drawForecastDetails(display, x + 88, y, 2);
}

void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex) {  //天气预报

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y, foreWeather[dayIndex].datestr);
  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, foreWeather[dayIndex].iconMeteoCon);

  String temp = foreWeather[dayIndex].tmp_min + " | " + foreWeather[dayIndex].tmp_max;
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState *state) {  //绘图页眉覆盖
  now = time(nullptr);
  struct tm *timeInfo;
  timeInfo = localtime(&now);
  char buff[14];
  sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);

  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(6, 54, String(buff));
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  String temp = fans;
  display->drawString(122, 54, temp);
  display->drawHorizontalLine(0, 52, 128);
}

void setReadyForWeatherUpdate() {  //为天气更新做好准备
  Serial.println("Setting readyForUpdate to true");
  readyForWeatherUpdate = true;
}
