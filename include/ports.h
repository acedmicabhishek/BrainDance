#pragma once

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline void outb(unsigned short port, unsigned char data) {
    asm volatile ( "outb %0, %1"
                   :
                   : "a"(data), "Nd"(port) );
}