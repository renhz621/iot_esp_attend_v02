#include "./esppl_functions.h"

#include <ESP8266WiFi.h>   //WIFI_Init：使用一些连接变量连接wifi（关联变量：内置DEFINE：如STATION.. ;WIFI_SSID；WIFI_PASSWORD）
#include <PubSubClient.h>  //如果测试一直卡在MQTT连接上，请使用PubSubClient库的2.7版本（资源文件夹下有压缩包文件），目前测试最新版2.8无法连接
#include <ArduinoJson.h>

////***#define 定义开始
//灯的位置
#define LEDPIN 0

//WIFI_Init：使用一些连接变量连接wifi（关联变量：内置DEFINE：如STATION.. ;WIFI_SSID；WIFI_PASSWORD）
#define WIFI_SSID "TP-LINK_E138"    //wifi名称
#define WIFI_PASSWORD "1234567890"  //wifi密码 \
                                    //WIFI_Init：END

//MQTT_Init：使用一些变量组（关联DEFINE：device_id;secret；关联变量：myclient 关联callback回调函数）连接MQTT
#define device_id "664d954cecb67c02e4270249_myNodeId"  //注册设备的ID和密钥
#define secret "1234567890"
//MQTT_Init：end

//MQTT三元组
#define ClientId "attend-wifi_888888888888_0_0_2024072906"
#define Username "attend-wifi_888888888888"
#define Password "817b0451ff2555c0d672b57eef6f088405af3cb21faec0d1fb6a55686777f1c1"
#define MQTT_Address "4d6d781166.st1.iotda-device.cn-north-4.myhuaweicloud.com"
#define MQTT_Port 1883
#define PID "attend-wifi_888888888888"
////MQTT_Init：使用一些变量组（关联DEFINE：device_id;secret；关联变量：myclient 关联callback回调函数）连接MQTT 设备属性上报
#define Iot_link_MQTT_Topic_Report "$oc/devices/attend-wifi_888888888888/sys/properties/report"              //测主动获取属性组
#define Iot_link_MQTT_Topic_shadow "$oc/devices/attend-wifi_888888888888/sys/properties/set/#"               //平台的下发属性设置，即影子下发 需测试，好像不需订阅，也可使用？还是
#define Iot_link_MQTT_Topic_request_shadow "$oc/devices/attend-wifi_888888888888/sys/shadow/get/response/#"  //设备侧主动请求影子数据，也需先订阅影子请求后的回复订阅
#define Iot_link_MQTT_request_shadow "$oc/devices/attend-wifi_888888888888/sys/shadow/get/request_id=998"    //设备请求影子，初次上电

////MQTT_Init：接收平台下发的命令
#define Iot_link_MQTT_Topic_Commands "$oc/devices/attend-wifi_888888888888/sys/commands/#"
////MQTT_Init：设备响应平台的命令 没用到
#define Iot_link_MQTT_Topic_CommandsRes "$oc/devices/attend-wifi_888888888888/sys/commands/response/request_id=987"

//MQTT_POST：手机MAC数据上报 body体
// #define Iot_link_Body_Format "{\"services\":[{\"service_id\":\"Provice_001\",\"properties\":{\"phone_mac_status\": {\"123456789012\":\"true\",\"003456789012\":\"false\"}}}]}"  //注意修改自己的服务ID
#define Iot_link_Body_Format "{\"services\":[{\"service_id\":\"provice01\",\"properties\":{\"phone_mac_status\": [";
//一些MQTT订阅主题或内容；临时测试数据

// #define Iot_link_Body_Format "{\"services\":[{\"service_id\":\"Provice_001\",\"Temp\":{%s" //注意修改自己的服务ID
// {"services":[{"service_id":"Dev_data","properties":{"temp": 39}}]}
// {"services":[{"service_id":"attend-wifi_888888888888","properties":{"phone_mac_status":[ {"phone_mac":"3447D400C6E1","status":true,"phone_mac":"745909FBA8FC","status":false,"phone_mac":"C46AB72F8FDB","status":true}]}}]}
//设置MAC测试：
// {
//   "services" : [
//     {
//       "service_id": "Provice_001",
//       "properties": {
//         "phone_mac_status": {"123456789012": "true","123456789012": "true"}
//       }
//     }
//   ]
// }
//
//{"services":[{"service_id":"Provice_001","properties":{"phone_mac_status": {"123456789012":"true","123456789012":"true"}}}]}
//设置MAC测试："{\"services\":[{\"service_id\":\"Provice_001\",\"properties\":{\"phone_mac_status\": {\"123456789012\":\"true\",\"123456789012\":\"true\"}}}]}"
////一些MQTT订阅主题或内容；临时测试数据END

//parseAndConvertMacs：MQTT初使化用回调/下行期望值数据解析给变量friendmac（关联），数据使用：LIST_SIZE；ESPPL_MAC_LEN；friendmac，static_mac_size
#define LIST_SIZE 100    // 假设最大100个MAC地址，动态得到实妹大小
#define ESPPL_MAC_LEN 6  // MAC地址长度
//***#define 定义结束

////变量定义开始
//测试取WIFI刚连接上时的状态通道值
int Wifi_current_channel = 0;
//MQTT_Init：使用一些变量组（关联变量：myclient/myesp8266Client）连接MQTT
WiFiClient myesp8266Client;              //定义一个网络连接客户端（用于MQTT连接）
PubSubClient myclient(myesp8266Client);  //在此连接上做MQTT对象，使用MQTT功能
//MQTT_Init：END


//临时定义
int data_temp = 1;
long lastMsg = 0;
//临时定义 end

/*
 * Define you friend's list size here
 */
// #define LIST_SIZE 4
/*
 * This is your friend's MAC address list
 */
//0xae,0x74,0x2b,0x99,0x56,0x98
//0x88,0x25,0x93,0x00,0xE1,0x38 TP
//0x10,0x60,0x4B,0x78,0x86,0x82 PC
//0x3C,0xA5,0x81,0x1E,0xB0,0xF2 VIVO
//0x74,0x59,0x09,0xFB,0xA8,0xFC navi mac
//0x52,0x0F,0x93,0xE3,0x27,0x10  iQOO-Neo5
/*  本机地址?
        macaddr[1] = 0xba;
        macaddr[2] = 0x7a;
        macaddr[3] = 0xb1;
        macaddr[4] = 0xe0;
        macaddr[5] = 0x42;
        */
// V1 函数解析json影子数据



//parseAndConvertMacs：MQTT初使化用回调/下行期望值数据解析给变量friendmac（关联），数据使用：LIST_SIZE；ESPPL_MAC_LEN；friendmac，static_mac_size
uint8_t friendmac[LIST_SIZE][ESPPL_MAC_LEN];  //也可以改造成在使用时创建，并返回给其它用，内存空间malloc
int static_mac_size = 0;                      //动态得到实妹大小
//parseAndConvertMacs：END

//cb： 定义一个需要上报report的body
int mac_status[100];
// String report_mac_status_body = "";
//定义一个需要上报report的bodyEND

uint8_t interval[2] = { 50, 20 };  // 创建一个的数组来存储间隔数据


//V1 上一版本的mac名称:对应index 知道是谁
String friendname[LIST_SIZE] = {
  "vivo-X20A", "360_CAMERA", "我的 ", "Friend 2"
};

//MAC比较函数据 uint8_t指针参数
bool maccmp(uint8_t* mac1, uint8_t* mac2) {
  for (int i = 0; i < ESPPL_MAC_LEN; i++) {
    if (mac1[i] != mac2[i]) {
      return false;
    }
  }
  return true;
}

void cb(esppl_frame_info* info) {
  Serial.printf(" cb is channel :%d", info->channel);  //新增 打印 channel
  for (int i = 0; i < static_mac_size; i++) {
    if (maccmp(info->sourceaddr, friendmac[i]) || maccmp(info->receiveraddr, friendmac[i])) {
      //拼接要上报的BODY前，对找到的对应index置
      mac_status[i] = 1;
      Serial.printf("\n%s is 在这里! :)", friendname[i].c_str());
    }
  }
}

////
unsigned long previousMillis = 0;
// const long interval = 25000;  // 20秒

void setup() {
  //WIFI连接成功后，

  ///////////加入连接
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);  //LED低电平触发,高电平熄灭
  delay(5000);

  Serial.print("开始WIFI_Init()");
  WIFI_Init();  //连接有错，则死循环
                //没有指定channel 试扫描



  ///////
  //原有
  delay(1000);
  //Serial.begin(115200);
  //原有
  Serial.print("打印 printState();");
  printState();
}


//新的loop
// 声明变量来保存上一次状态改变的时间
unsigned long lastStateChangeMillis = 0;
// 声明变量来保存参数A的当前状态
bool parameterAState = false;  // 假设初始状态为false,loop 等 任意扫描列表；

// 设置时间间隔
// const unsigned long activeInterval = 30000;    // 激活状态保持时间：50秒网络正常
// const unsigned long inactiveInterval = 60000;  // 非激活状态保持时间：6秒
void loop() {
  unsigned long currentMillis = millis();

  // 检查是否需要改变状态 当状态为true:6秒；为false:50秒  ；默认起步50秒
  if (currentMillis - lastStateChangeMillis >= (parameterAState ? interval[0]*1000 : interval[1]*1000)) {
    // 改变状态
    parameterAState = !parameterAState;

    lastStateChangeMillis = currentMillis;  // 更新上次改变状态的时间

    // 打印新状态
    printState();  ///在状态合适，且符合扫描准备好前题，
    if (parameterAState) {
      Serial.print("开始扫描时间");
      Serial.println(currentMillis);
      Serial.print("查看状态：");
      Serial.println(static_mac_size);
      ////true状态就打开扫描 再判断是否符合前置条件
      if (static_mac_size != 0) {
        Serial.print("开始置位");
        //初始一次置位状态
        for (int i = 0; i < static_mac_size; i++) {
          mac_status[i] = 0;
        }

        //初始一次置位状态
        Serial.print("置位结束;真实开始扫描时间");
        Serial.println(currentMillis);
        esppl_set_channel(Wifi_current_channel);  //初使化前定下channel
        esppl_init(cb);
        esppl_sniffing_start();
      }else{
        Serial.println("状态0，无可扫，不扫描");
      }

    } else {


      Serial.print("开始上报MQTT时间");
      Serial.println(currentMillis);
      Serial.print("查看状态MAC——size：");
      Serial.println(static_mac_size);
      wifi_promiscuous_enable(false);
      esppl_sniffing_stop();
      WIFI_reconnect();
      if (static_mac_size != 0) {  //有扫描任意才可上报信息
        MQTT_POST();
      }
    }
  }
  if (!parameterAState && WiFi.status() == WL_CONNECTED) {  // 增加如果WIFI正常则处理MQTT
    //正常处理时间
    // Serial.print("MQTT.loop:");
    // int ss = (WiFi.status() == WL_CONNECTED);
    // Serial.println(ss);
    myclient.loop();  ////处理MQTT消息及保持心跳
  }

  // 注意：这里的循环是空的，因为所有的时间控制都通过millis()在上面的if语句中完成了
  // Arduino将继续执行loop()函数，但不会在if语句内部执行任何操作，直到达到下一个时间间隔
}

void printState() {

  Serial.print("Parameter A State: ");
  Serial.println(parameterAState ? "true" : "false");
}
//新的loop end

//WIFI重连接函数WIFI_reconnect 停止使用 与wifi_init 相差不大
void WIFI_Init() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi Not Connect");
  }
  Serial.println("WiFi Connected OK!");
  Serial.print("WIFI_Init/get channel=");
  Serial.println(WiFi.channel());
  // //置全局变量，在扫描时使用
  Wifi_current_channel = WiFi.channel();
  MQTT_Init();
}
////WIFI重连接函数END

//////////
//WIFI_Init：使用一些连接变量连接wifi（关联）ok
void WIFI_reconnect()  //没有连接上则循环连接 不返回
{

  WiFi.mode(WIFI_STA);                   //模式
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  //开始连接WIFI名，密码
  while (WiFi.status() != WL_CONNECTED)  //循环连接 不返回
  {
    //把初始化（实际是连接，并置回调，订阅
    delay(2000);
    Serial.println("WiFi Not Connect");
  }
  MQTT_reconnect();
  Serial.println("WiFi Connected reconnected OK!");
  // Serial.print("get mode=");
  // Serial.print(WiFi.getMode());
  Serial.print("get channel=");
  Serial.println(WiFi.channel());
  // //置全局变量，在扫描时使用
  Wifi_current_channel = WiFi.channel();
}

//新函数 重连接
void MQTT_reconnect() {
  while (!myclient.connected())  // 执行连接，返回？
  {
    if (myclient.connect(ClientId, Username, Password)) {
      Serial.println("重连接MQTT连接服务器成功.");
    } else {
      Serial.print("重连接MQTT连接服务器出错状态:");
      Serial.println(myclient.state());
      delay(2000);
    }
  }
}
//MQTT_Init：使用一些变量组（关联DEFINE：device_id;secret；关联变量：myclient 关联callback回调函数）连接MQTT
void MQTT_Init() {
  Serial.println("MQTT_Init开始.");
  myclient.setServer(MQTT_Address, MQTT_Port);  //设置云地址和端口
  while (!myclient.connected())                 // 循环执行连接，返回1
  {
    if (myclient.connect(ClientId, Username, Password)) {
      Serial.println("MQTT连接服务器成功.");
      // myclient.setCallback(callback);
    } else {
      Serial.print("MQTT连接服务器出错状态：");
      Serial.println(myclient.state());
      delay(2000);
    }
  }
  Serial.println("MQTT连接服务器setCallback");
  myclient.setCallback(callback);                                        //设定回调方式，当ESP8Iot_link_Body_Format266收到订阅消息时会调用此方法
  boolean res = myclient.subscribe(Iot_link_MQTT_Topic_request_shadow);  //连接成功时订阅主题commands
  if (res != true) {                                                     //订阅错误
    Serial.printf("mqtt subscribe topic [%s] fail\n", Iot_link_MQTT_Topic_request_shadow);
  } else {
    Serial.printf("mqtt subscribe topic [%s] ok\n", Iot_link_MQTT_Topic_request_shadow);
  }
  myclient.publish(Iot_link_MQTT_request_shadow, "{}");
  // boolean res = myclient.subscribe(Iot_link_MQTT_request_shadow);  //连接成功时订阅主题commands
  Serial.printf("mqtt publish topic [%s] ok\n", Iot_link_MQTT_request_shadow);
  //测试 ，及时找回应得到数据
  delay(1000);
  myclient.loop();  ////处理MQTT消息及保持心跳
}

//MQTT_POST：手机MAC数据上报
void MQTT_POST() {
  Serial.println("MQTT_POST()发射中：");
  //组合消息体的临时变量，属性
  char properties[32];  //属性
  char jsonBuf[128];    //大部分消息体
  //组合消息
  // 上报此格式
  // {
  // 	"services":[
  // 	    {
  // 		"service_id":"Provice_001",
  // 		"properties":{
  // 			"phone_mac_status": {"123456789012":"true","123456789012":"true"}
  // 		}
  // 	     }
  // 	]
  // }
  // Iot_link_Body_Format "{\"services\":[{\"service_id\":\"Provice_001\",\"properties\":{\"phone_mac_status\": {\"123456789012\":\"true\",\"003456789012\":\"false\"}}}]}"  //注意修改自己的服务ID
  // sprintf(properties,"\"temp\":%d}}]}",data_temp);

  // sprintf(jsonBuf, Iot_link_Body_Format, properties);
  // [{ phone_mac: "998898878787", status: false }, { phone_mac: "888888888888", status: true }]
  String str_mac = Iot_link_Body_Format
    // String str_mac = "{\"services\":[{\"service_id\":\"Provice_001\",\"properties\":{\"phone_mac_status\": {";
    //  {"services":[{"service_id":"attend-wifi_888888888888","properties":{"phone_mac_status":[ {"phone_mac":"3447D400C6E1","status":true},{"phone_mac":"745909FBA8FC","status":false},{"phone_mac":"C46AB72F8FDB","status":true}}]}}]}
    //  {"services":[{"service_id":"attend-wifi_888888888888","properties":{"phone_mac_status":[ {"phone_mac":"3447D400C6E1","status":true},{"phone_mac":"745909FBA8FC","status":false},{"phone_mac":"C46AB72F8FDB","status":true}}],set_mac:"50/20"}}]}
    for (int i = 0; i < static_mac_size; i++) {
    str_mac += "{\"phone_mac\":\"";
    for (int ii = 0; ii < 6; ii++) {
      char hexStr[3];
      snprintf(hexStr, 3, "%02X", friendmac[i][ii]);  // 3包括'\0'
      str_mac += hexStr;
    }
    str_mac += "\",\"status\":";
    if (mac_status[i]) {
      str_mac += "true}";
    } else {
      str_mac += "false}";
    }
    if (i != (static_mac_size - 1)) {
      str_mac += ",";
    } else {
      str_mac += "]}}]}";
    }
  }

  // if (strlen(macStr) == 12) {
  //   // 注意：strtol需要两个字符来表示一个十六进制数，所以我们每次跳过两个字符
  //   for (int ii = 0; ii < 6; ii++) {
  //     // 注意：strtol需要两个字符来表示一个十六进制数，所以我们每次跳过两个字符
  //     char tempStr[3];
  //     tempStr[0] = macStr[ii * 2];
  //     tempStr[1] = macStr[ii * 2 + 1];
  //     tempStr[3] = '\0';
  //     friendmac[i][ii] = (uint8_t)strtol(tempStr, NULL, 16);  // 转换为uint8_t
  //   }
  // }
  // for (int i = 0; i < static_mac_size; i++) {
  // }
  myclient.publish(Iot_link_MQTT_Topic_Report, str_mac.c_str());
  Serial.println(Iot_link_MQTT_Topic_Report);
  Serial.println(str_mac);
  Serial.println("MQTT Publish OK!");
}

//callback：MQTT初使化用回调
void callback(char* topic, byte* payload, unsigned int length) {
  String recdata = "";
  //这个回调是透明，不需要改的，是接收的回调
  Serial.printf("接收到订阅的消息:主题为：");
  //打印接收到的主题:
  Serial.println(topic);
  Serial.printf("数据内容：");
  for (int i = 0; i < length; i++) {
    recdata += (char)payload[i];
  }
  //打印接收到的内容
  Serial.println(recdata);
  //即时返回，消除影子
  myclient.publish(Iot_link_MQTT_Topic_Report, recdata.c_str());
  //解析包括可以解析主动请求的影子数据和平台下发的属性配置信息

  // 接收到订阅的消息:主题为：$oc/devices/attend-wifi_888888888888/sys/properties/set/request_id=f71d8497-494d-498b-8da4-2b0602631b57
  // 数据内容：{"services":[{"properties":{"set_mac":["5547D400C6E1","3447D400C6E1","745909FBA8FC"]},"service_id":"Provice_001"}]}
  //
  // 接收到订阅的消息:主题为：$oc/devices/attend-wifi_888888888888/sys/shadow/get/response/request_id=998 应该是随机数或是
  // 数据内容：{"shadow":[{"desired":{"properties":{"set01":20,"set_mac":["1547D400C6E1","3447D400C6E1","745909FBA8FC"]},"event_time":"20240720T014842Z"},"reported":{"properties":{"phone_mac_status":{"123456789012":"true"},"set_mac":["1547D400C6E1","3447D400C6E1","745909FBA8FC"],"set01":20},"event_time":"20240720T014842Z"},"version":328,"service_id":"Provice_001"}],"object_device_id":"attend-wifi_888888888888"}
  // //
  parseAndConvertMacs(recdata);
}

// 函数定义
void splitStringIntoNumbers(const String& input, int* output) {
  int pos = input.indexOf('/');               // 查找分隔符'/'的位置
  if (pos != -1) {                            // 如果找到了分隔符
    String part1 = input.substring(0, pos);   // 获取第一部分
    String part2 = input.substring(pos + 1);  // 获取第二部分（跳过'/'）

    // 将字符串转换为整数
    output[0] = part1.toInt();
    output[1] = part2.toInt();
  } else {
    // 如果没有找到分隔符，你可以在这里添加错误处理
    // 例如，将输出数组设置为特定的错误值或打印错误消息
    output[0] = 0;
    output[1] = 0;
    Serial.println("未找到分隔符'/'！");
  }
}

//parseAndConvertMacs：MQTT初使化用回调/下行期望值数据解析给变量friendmac（关联），数据使用：LIST_SIZE；ESPPL_MAC_LEN；friendmac，static_mac_size
void parseAndConvertMacs(String json) {
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, json);
  //解析错误退出
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  //这里以下是解析set_mac 字段的值（平台或应用侧属性下发）
  if (doc.containsKey("services") && doc["services"].is<JsonArray>()) {
    const JsonArray& services = doc["services"];
    if (!services.isNull() && services.size() > 0) {
      const JsonObject& firstService = services[0];
      if (!firstService.isNull()) {
        const JsonArray& macArray = firstService["properties"]["set_mac"];
        if (!macArray.isNull() && macArray.size()!=0) {
          Serial.print("[");
          // 处理macArray,set_mac...先显示一下所有mac
          // 取macArray大小
          static_mac_size = macArray.size();
          for (size_t i = 0; i < macArray.size(); i++) {
            if (i > 0) Serial.print(", ");
            // 这里我们简化了处理，只考虑字符串和数字
            if (macArray[i].is<String>()) {
              Serial.print("\"" + macArray[i].as<String>() + "\"");
            } else if (macArray[i].is<int>() || macArray[i].is<float>() || macArray[i].is<double>()) {
              // 注意：这里我们假设了int、float和double都可以直接转换为String
              // 在实际应用中，你可能需要区分它们以保持精度
              Serial.print(macArray[i].as<String>());
            } else if (macArray[i].is<bool>()) {
              Serial.print(macArray[i].as<bool>() ? "true" : "false");
            }
            // 注意：这里没有处理嵌套的JsonArray或JsonObject
          }
          Serial.println("]");
          //处理macArray,set_mac...END
          //再将其置至数组中，备用 下面有个限制大小
          for (size_t i = 0; i < macArray.size() && i < LIST_SIZE; i++) {
            Serial.print("大小size");
            Serial.println(macArray.size());
            const char* macStr = macArray[i];
            if (strlen(macStr) == 12) {
              // 注意：strtol需要两个字符来表示一个十六进制数，所以我们每次跳过两个字符
              for (int ii = 0; ii < 6; ii++) {
                // 注意：strtol需要两个字符来表示一个十六进制数，所以我们每次跳过两个字符
                char tempStr[3];
                tempStr[0] = macStr[ii * 2];
                tempStr[1] = macStr[ii * 2 + 1];
                tempStr[3] = '\0';
                friendmac[i][ii] = (uint8_t)strtol(tempStr, NULL, 16);  // 转换为uint8_t
              }
            } else {
              // 处理错误或无效的MAC地址
              Serial.print(F("Invalid MAC format for item "));
              Serial.println(i);
            }
          }
        }else{
        Serial.print(F("回调cb 扫描set_mac：数组为空。置static_mac_size = 0;"));
        static_mac_size = 0;
        }

      }
    }
  }
  if (doc.containsKey("shadow") && doc["shadow"].is<JsonArray>()) {
    ////////////
    Serial.println("cb解析影子步骤1");
    const JsonArray& shadowArray = doc["shadow"];
    const JsonObject& shadowObj = shadowArray[0];
    Serial.println("cb解析影子步骤2");
    if (shadowObj.containsKey("desired") && shadowObj["desired"].is<JsonObject>()) {
      Serial.println("cb解析影子步骤3");
      JsonObject desired = shadowObj["desired"].as<JsonObject>();
      if (desired.containsKey("properties") && desired["properties"].is<JsonObject>()) {
        JsonObject propertiesDesired = desired["properties"].as<JsonObject>();
        //
        if (propertiesDesired.containsKey("set_mac") && propertiesDesired["set_mac"].is<JsonArray>()) {
          JsonArray macArray = propertiesDesired["set_mac"].as<JsonArray>();
          if (!macArray.isNull()) {
              /////mac解析
            Serial.print("[");
            // 处理macArray,set_mac...先显示一下所有mac
            // 取macArray大小
            static_mac_size = macArray.size();
            for (size_t i = 0; i < macArray.size(); i++) {
              if (i > 0) Serial.print(", ");
              // 这里我们简化了处理，只考虑字符串和数字
              if (macArray[i].is<String>()) {
                Serial.print("\"" + macArray[i].as<String>() + "\"");
              } else if (macArray[i].is<int>() || macArray[i].is<float>() || macArray[i].is<double>()) {
                // 注意：这里我们假设了int、float和double都可以直接转换为String
                // 在实际应用中，你可能需要区分它们以保持精度
                Serial.print(macArray[i].as<String>());
              } else if (macArray[i].is<bool>()) {
                Serial.print(macArray[i].as<bool>() ? "true" : "false");
              }
              // 注意：这里没有处理嵌套的JsonArray或JsonObject
            }
            Serial.println("]");
            //处理macArray,set_mac...END
            //再将其置至数组中，备用 下面有个限制大小
            for (size_t i = 0; i < macArray.size() && i < LIST_SIZE; i++) {
              Serial.print("大小size");
              Serial.println(macArray.size());
              const char* macStr = macArray[i];
              if (strlen(macStr) == 12) {
                // 注意：strtol需要两个字符来表示一个十六进制数，所以我们每次跳过两个字符
                for (int ii = 0; ii < 6; ii++) {
                  // 注意：strtol需要两个字符来表示一个十六进制数，所以我们每次跳过两个字符
                  char tempStr[3];
                  tempStr[0] = macStr[ii * 2];
                  tempStr[1] = macStr[ii * 2 + 1];
                  tempStr[3] = '\0';
                  friendmac[i][ii] = (uint8_t)strtol(tempStr, NULL, 16);  // 转换为uint8_t
                }
              } else {
                // 处理错误或无效的MAC地址
                Serial.print(F("Invalid MAC format for item "));
                Serial.println(i);
              }
            }
          }
        }
        //解析interval
        if (propertiesDesired.containsKey("interval")) {
          String intervalStr = propertiesDesired["interval"];

          /////内部
          int pos = intervalStr.indexOf('/');               // 查找分隔符'/'的位置
          if (pos != -1) {                                  // 如果找到了分隔符
            String part1 = intervalStr.substring(0, pos);  
             Serial.print("interval0："); // 获取第一部分
              Serial.println(part1);
            String part2 = intervalStr.substring(pos + 1);  // 获取第二部分（跳过'/'）
             Serial.print("interval1："); // 获取第一部分
              Serial.println(part2);
            // 将字符串转换为整数
            interval[0] = part1.toInt();
            interval[1] = part2.toInt();
          } else {
            // 恢复默认：50 / 30
            //
            interval[0] = 50;
            interval[1] = 30;
            Serial.println("未找到分隔符'/'！");
          }

        }
        ///解析interval
      }
    }
  }
}
