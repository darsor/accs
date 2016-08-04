#include "packets.h"
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>

Packet::Packet(const uint32_t length, const uint8_t id, bool cmd):
    length(length), id(id) {
    if (cmd) buffer = new unsigned char[length];
    else buffer = nullptr;
}
Packet::Packet(const Packet& that): length(that.length), id(that.id) {
    buffer = nullptr;
}
Packet::~Packet() {
    if (buffer) delete[] buffer;
}
Packet& Packet::operator=(const Packet& that) {
    if (this != &that)
    {
        if (buffer) delete[] buffer;
        buffer = nullptr;
        length = that.length;
        id = that.id;
    }
    return *this;
}
void Packet::convert() { printf("Virtual convert() called. That's a problem\n"); }

PressurePacket::PressurePacket() : Packet(PRES_PKT_SIZE, PRES_PKT_ID) {}

void PressurePacket::convert() {
    if (!buffer) buffer = new unsigned char[length];
    memcpy(buffer+0 , &length         , 4);
    memcpy(buffer+4 , &id             , 1);
    memcpy(buffer+5 , &venturiTime    , 8);
    memcpy(buffer+13, &venturiPressure, 4);
    memcpy(buffer+17, &pumpTime       , 8);
    memcpy(buffer+25, &pumpPressure   , 4);
    memcpy(buffer+29, &staticTime     , 8);
    memcpy(buffer+37, &staticPressure , 4);
}

TempPacket::TempPacket() : Packet(TEMP_PKT_SIZE, TEMP_PKT_ID) {}

void TempPacket::convert() {
    if (!buffer) buffer = new unsigned char[length];
    memcpy(buffer+0 , &length, 4);
    memcpy(buffer+4 , &id    , 1);
    memcpy(buffer+5 , &time1 , 8);
    memcpy(buffer+13, &temp1 , 2);
    memcpy(buffer+15, &time2 , 8);
    memcpy(buffer+23, &temp2 , 2);
    memcpy(buffer+25, &time3 , 8);
    memcpy(buffer+33, &temp3 , 2);
    memcpy(buffer+35, &time4 , 8);
    memcpy(buffer+43, &temp4 , 2);
}

RpmPacket::RpmPacket() : Packet(RPM_PKT_SIZE, RPM_PKT_ID) {}

void RpmPacket::convert() {
    if (!buffer) buffer = new unsigned char[length];
    memcpy(buffer+0 , &length   , 4);
    memcpy(buffer+4 , &id       , 1);
    memcpy(buffer+5 , &timestamp, 8);
    memcpy(buffer+13, &value    , 2);
}

HKPacket::HKPacket() : Packet(HK_PKT_SIZE, HK_PKT_ID) {}

void HKPacket::convert() {
    if (!buffer) buffer = new unsigned char[length];
    memcpy(buffer+0 , &length    , 4);
    memcpy(buffer+4 , &id        , 1);
    memcpy(buffer+5 , &timestamp , 8);
    memcpy(buffer+13, &queue_size, 2);
    memcpy(buffer+15, &cpu_temp  , 4);
    memcpy(buffer+19, &cpu_load  , 4);
    memcpy(buffer+23, &mem_usage , 4);
}

PumpCmd::PumpCmd() : Packet(PUMP_CMD_SIZE, PUMP_CMD_ID, true) {}

void PumpCmd::convert() {
    if (!buffer) return;
    memcpy(&voltage, buffer+5, sizeof(voltage));
}
