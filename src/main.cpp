#include <Arduino.h>
#include "led.h"
#include "sd.h"
#include <ESP32CAN.h>
#include <CAN_config.h>

CAN_device_t CAN_cfg;         // CAN Config
const int rx_queue_size = 10; // Receive Queue size

File file_log;
//char message[128];

/*
sd card pinout:
CS GPIO2
SCK GPIO18
MISO GPIO19
MOSI GPIO23
*/

/*
canbus pinout
tx = GPIO_NUM_14;
rx = GPIO_NUM_27;
*/

void canbus_setup()
{
  CAN_cfg.speed = CAN_SPEED_250KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_14;
  CAN_cfg.rx_pin_id = GPIO_NUM_27;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  // Init CAN Module
  ESP32Can.CANInit();
  Serial.println("cabus initialized");
}

void setup()
{
  Serial.begin(115200);
  led_setup();
  canbus_setup();

  if (!SD.begin())
  { // use default pinout
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  int count_files = listDir(SD, "/", 0);
  count_files++;
  char current_filename[128];
  sprintf(current_filename, "/log%04d.log", count_files);
  file_log = SD.open(current_filename, FILE_APPEND);
  if (!file_log)
  {
    Serial.printf("Failed to open %s file for appending\n", (char*)current_filename);
    return;
  }
  else
  {
    Serial.printf("opened %s file for appending\n", (char*)current_filename);
    file_log.flush();
  }

  // createDir(SD, "/mydir");
  // listDir(SD, "/", 0);
  // removeDir(SD, "/mydir");
  // listDir(SD, "/", 2);
  // writeFile(SD, "/hello.txt", "Hello ");
  // appendFile(SD, "/hello.txt", "World!\n");
  // readFile(SD, "/hello.txt");
  // deleteFile(SD, "/foo.txt");
  // renameFile(SD, "/hello.txt", "/foo.txt");
  // readFile(SD, "/foo.txt");
  // testFileIO(SD, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

void loop()
{
  CAN_frame_t rx_frame;

  // Receive next CAN frame from queue
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
  {

    // if (rx_frame.FIR.B.FF == CAN_frame_std)
    // {
    //   printf("New standard frame");
    // }
    // else
    // {
    //   printf("New extended frame");
    // }

    if (rx_frame.FIR.B.RTR == CAN_RTR)
    {
      printf(" RTR from 0x%08X, DLC %d\r\n", rx_frame.MsgID, rx_frame.FIR.B.DLC);
      file_log.printf("%d 0x%08X %d RTR\n", millis(), rx_frame.MsgID, rx_frame.FIR.B.DLC);
    }
    else
    {
      printf(" from 0x%08X, DLC %d, Data ", rx_frame.MsgID, rx_frame.FIR.B.DLC);
      file_log.printf("%d 0x%08X %d ", millis(), rx_frame.MsgID, rx_frame.FIR.B.DLC);
      for (int i = 0; i < rx_frame.FIR.B.DLC; i++)
      {
        printf("0x%02X ", rx_frame.data.u8[i]);
        file_log.printf("0x%02X ", rx_frame.data.u8[i]);
      }
      printf("\n");
      file_log.printf("\n");
    }
    file_log.flush();
  }
}