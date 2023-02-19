#include <Wire.h>
#include "Adafruit_VL6180X.h"
#include <SerialCommands.h>

const char* DEVICE_TYPE = "distance";
const int FW_VERSION = 10000; // 1.00.00
const int PROTOCOL_VERSION = 10000; // 1.00.00

Adafruit_VL6180X vl = Adafruit_VL6180X();

char serial_command_buffer[32];
SerialCommands serialCommands(&Serial, serial_command_buffer, sizeof(serial_command_buffer), "\n", " ");
const char* sensorStatus = "unknown";
boolean sensorFound = false;

void setup() {
    Serial.begin(9600);
  
    // wait for serial port to open on native usb devices
    /*while (!Serial) {
      delay(1);
    }*/
    
    if (vl.begin()) {
      sensorFound = true;
    }

    // Add commands
    serialCommands.SetDefaultHandler(commandUnrecognized);
    serialCommands.AddCommand(new SerialCommand("introduce", commandIntroduce));
    serialCommands.AddCommand(new SerialCommand("attributes", commandAttributes));
    serialCommands.AddCommand(new SerialCommand("status", commandStatus));
    serialCommands.AddCommand(new SerialCommand("get-distance", commandGetDistance));
    serialCommands.AddCommand(new SerialCommand("get-sensor", commandGetSensor));
    serialCommands.AddCommand(new SerialCommand("get-lux", commandGetLux));

    serialCommands.GetSerial()->write(0x07);
}

void loop() {
    serialCommands.ReadSerial();
}

void commandUnrecognized(SerialCommands* sender, const char* cmd)
{
    serial_printf(sender->GetSerial(), "Unrecognized command [%s]\n", cmd);
}

void commandIntroduce(SerialCommands* sender) {
    serial_printf(sender->GetSerial(), "introduce;%s,%d,%d\n", DEVICE_TYPE, FW_VERSION, PROTOCOL_VERSION);
}

void commandAttributes(SerialCommands* sender)
{
    serial_printf(sender->GetSerial(), "attributes;sensor:ro[str],distance:ro[int],lux:ro[float]\n");
}

void commandStatus(SerialCommands* sender) {
    if (!sensorFound) {
        sender->GetSerial()->write("status;lux:,distance:,sensor:\n");
    }

    float lux = vl.readLux(VL6180X_ALS_GAIN_5);
    uint8_t range = vl.readRange();
    uint8_t status = vl.readRangeStatus();
    
    updateSensorStatus(status); 
    
    serial_printf(sender->GetSerial(), "status;sensor:%s,distance:%d,lux:%d\n", sensorStatus, range, lux);
}

void commandGetSensor(SerialCommands* sender) {
    if (!sensorFound) {
        sender->GetSerial()->write("get-sensor;;status:failed,reason:sensor_not_found\n");
    }

    uint8_t status = vl.readRangeStatus();
    
    updateSensorStatus(status); 
    
    serial_printf(sender->GetSerial(), "get-sensor;%s;status:successful\n", sensorStatus);
}

void commandGetDistance(SerialCommands* sender) {
    if (!sensorFound) {
        sender->GetSerial()->write("get-distance;;status:failed,reason:sensor_not_found\n");
    }

    uint8_t range = vl.readRange();
    
    serial_printf(sender->GetSerial(), "get-distance;%d;status:successful\n", range);
}

void commandGetLux(SerialCommands* sender) {
    if (!sensorFound) {
        sender->GetSerial()->write("get-sensor;;status:failed,reason:sensor_not_found\n");
    }

    float lux = vl.readLux(VL6180X_ALS_GAIN_5);
    
    serial_printf(sender->GetSerial(), "get-sensor;%d;status:successful\n", lux);
}


void updateSensorStatus(uint8_t status) {
  if (status == VL6180X_ERROR_NONE) {
    sensorStatus = "ok";
  } else if  ((status >= VL6180X_ERROR_SYSERR_1) && (status <= VL6180X_ERROR_SYSERR_5)) {
    sensorStatus = "sys_error";
  } else if (status == VL6180X_ERROR_ECEFAIL) {
    sensorStatus = "ece_fail";
  } else if (status == VL6180X_ERROR_NOCONVERGE) {
    sensorStatus = "no_convergence";
  } else if (status == VL6180X_ERROR_RANGEIGNORE) {
    sensorStatus = "range_ignore";
  } else if (status == VL6180X_ERROR_SNR) {
    sensorStatus = "signal_noise";
  } else if (status == VL6180X_ERROR_RAWUFLOW) {
    sensorStatus = "raw_underflow";
  } else if (status == VL6180X_ERROR_RAWOFLOW) {
    sensorStatus = "raw_overflow";
  } else if (status == VL6180X_ERROR_RANGEUFLOW) {
    sensorStatus = "range_underflow";
  } else if (status == VL6180X_ERROR_RANGEOFLOW) {
    sensorStatus = "range_overflow";
  } else {
    sensorStatus = "unknown";
  }
}
