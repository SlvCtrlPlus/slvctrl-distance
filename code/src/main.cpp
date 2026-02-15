#include <Wire.h>
#include <Adafruit_VL6180X.h>

#include <SlvCtrlProtocol.h>
#include <SlvCtrlArduinoSerialCommandsTransport.h>

static constexpr const char* DEVICE_TYPE = "distance";
static constexpr uint32_t FW_VERSION = 20000;

Adafruit_VL6180X vl;
bool sensorFound = false;

static std::optional<int32_t> getDistance(void*)
{
    if (!sensorFound) return std::nullopt;
    return (int32_t)vl.readRange();
}

static std::optional<float> getLux(void*)
{
    if (!sensorFound) return std::nullopt;
    return vl.readLux(VL6180X_ALS_GAIN_5);
}

static std::optional<const char*> getSensor(void*)
{
    if (!sensorFound) return "not_found";

    uint8_t status = vl.readRangeStatus();

    switch (status)
    {
        case VL6180X_ERROR_NONE:         return "ok";
        case VL6180X_ERROR_ECEFAIL:      return "ece_fail";
        case VL6180X_ERROR_NOCONVERGE:   return "no_convergence";
        case VL6180X_ERROR_RANGEIGNORE:  return "range_ignore";
        case VL6180X_ERROR_SNR:          return "signal_noise";
        case VL6180X_ERROR_RAWUFLOW:     return "raw_underflow";
        case VL6180X_ERROR_RAWOFLOW:     return "raw_overflow";
        case VL6180X_ERROR_RANGEUFLOW:   return "range_underflow";
        case VL6180X_ERROR_RANGEOFLOW:   return "range_overflow";
        default:                         return "unknown";
    }
}

StrAttribute sensorAttr( "sensor", getSensor, nullptr);
IntAttribute distanceAttr("distance", getDistance, nullptr);
FloatAttribute luxAttr("lux", getLux, nullptr);

static IAttribute* ATTRS[] = {
  &sensorAttr,
  &distanceAttr,
  &luxAttr
};

static char gCmdBuf[64];

SlvCtrlProtocol proto(DEVICE_TYPE, FW_VERSION, ATTRS);
SlvCtrlSerialCommandsTransport transport(Serial, proto, gCmdBuf, sizeof(gCmdBuf));

void setup()
{
    Serial.begin(9600);

    sensorFound = vl.begin();

    Serial.print(0x07);
}

void loop()
{
    transport.readCommand();
}
