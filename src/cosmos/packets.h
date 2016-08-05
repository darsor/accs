#ifndef PACKETS_H
#define PACKETS_H

#define PRES_PKT_ID         0x00
#define PRES_PKT_SIZE       41

#define TEMP_PKT_ID         0x10
#define TEMP_PKT_SIZE       45

#define RPM_PKT_ID          0x20
#define RPM_PKT_SIZE        15

#define PUMP_CMD_ID         0x30
#define PUMP_CMD_SIZE       9

#define LEVEL_PKT_ID        0x40
#define LEVEL_PKT_SIZE      17

#define HK_PKT_ID           0xFF
#define HK_PKT_SIZE         27

#include <cstdint>      // for precise integer definitions

inline void endianSwap(float &f) {
    float temp = f;
    unsigned char* pf = (unsigned char*) &f;
    unsigned char* pt = (unsigned char*) &temp;
    pf[0] = pt[3];
    pf[1] = pt[2];
    pf[2] = pt[1];
    pf[3] = pt[0];
}

class Packet {
    public:
        Packet(const uint32_t length, const uint8_t id, bool cmd = false);
        Packet(const Packet& that);
        Packet& operator=(const Packet& that);
        virtual ~Packet();
        virtual void convert();
        uint32_t length;
        uint8_t id;
        unsigned char* buffer;
};

class PressurePacket: public Packet {
    public:
        PressurePacket();
        void convert();
        uint64_t venturiTime;
        uint32_t venturiPressure;
        uint64_t pumpTime;
        uint32_t pumpPressure;
        uint64_t staticTime;
        uint32_t staticPressure;
};

class TempPacket: public Packet {
    public:
        TempPacket();
        void convert();
        uint64_t time1;
        uint16_t temp1;
        uint64_t time2;
        uint16_t temp2;
        uint64_t time3;
        uint16_t temp3;
        uint64_t time4;
        uint16_t temp4;
};

class RpmPacket: public Packet {
    public:
        RpmPacket();
        void convert();
        uint64_t timestamp;
        uint16_t value;
};

class LevelPacket: public Packet {
    public:
        LevelPacket();
        void convert();
        uint64_t timestamp;
        uint32_t value;
};

class HKPacket: public Packet {
    public:
        HKPacket();
        void convert();
        uint64_t timestamp;
        uint16_t queue_size;
        float cpu_temp;
        float cpu_load;
        uint32_t mem_usage;
};

class PumpCmd: public Packet {
    public:
        PumpCmd();
        void convert();
        float voltage;
};

#endif
