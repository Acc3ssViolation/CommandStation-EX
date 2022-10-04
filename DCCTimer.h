/*
 *  © 2022 Paul M Antoine
 *  © 2021 Mike S
 *  © 2021-2022 Harald Barth
 *  © 2021 Fred Decker
 *  All rights reserved.
 *  
 *  This file is part of CommandStation-EX
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */

/* There are several different implementations of this class which the compiler will select 
   according to the hardware.
   */

/* This timer class is used to manage the single timer required to handle the DCC waveform.
 *  All timer access comes through this class so that it can be compiled for 
 *  various hardware CPU types. 
 *  
 *  DCCEX works on a single timer interrupt at a regular 58uS interval.
 *  The DCCWaveform class generates the signals to the motor shield  
 *  based on this timer. 
 *  
 *  If the motor drivers are BOTH configured to use the correct 2 pins for the architecture,
 *  (see isPWMPin() function. )
 *  then this allows us to use a hardware driven pin switching arrangement which is
 *  achieved by setting the duty cycle of the NEXT clock interrupt to 0% or 100% depending on 
 *  the required pin state. (see setPWM())  
 *  This is more accurate than the software interrupt but at the expense of 
 *  limiting the choice of available pins. 
 *  Fortunately, a standard motor shield on a Mega uses pins that qualify for PWM... 
 *  Other shields may be jumpered to PWM pins or run directly using the software interrupt.
 *  
 *  Because the PWM-based waveform is effectively set half a cycle after the software version,
 *  it is not acceptable to drive the two tracks on different methiods or it would cause
 *  problems for <1 JOIN> etc.
 *  
 */

#ifndef DCCTimer_h
#define DCCTimer_h
#include "Arduino.h"

typedef void (*INTERRUPT_CALLBACK)();

class DCCTimer {
  public:
  static void begin(INTERRUPT_CALLBACK interrupt);
  static void getSimulatedMacAddress(byte mac[6]);
  static bool isPWMPin(byte pin);
  static void setPWM(byte pin, bool high);
  static void clearPWM();
// Update low ram level.  Allow for extra bytes to be specified
// by estimation or inspection, that may be used by other 
// called subroutines.  Must be called with interrupts disabled.
// 
// Although __brkval may go up and down as heap memory is allocated
// and freed, this function records only the worst case encountered.
// So even if all of the heap is freed, the reported minimum free 
// memory will not increase.
//
  static void inline updateMinimumFreeMemoryISR(unsigned char extraBytes=0)
    __attribute__((always_inline)) {
    int spare = freeMemory()-extraBytes;
    if (spare < 0) spare = 0;
    if (spare < minimum_free_memory) minimum_free_memory = spare;
  };

  static int  getMinimumFreeMemory();
  static void reset();
  
private:
  static int freeMemory();
  static volatile int minimum_free_memory;
  static const int DCC_SIGNAL_TIME=58;  // this is the 58uS DCC 1-bit waveform half-cycle 
#if defined(ARDUINO_ARCH_STM32)  // TODO: PMA temporary hack - assumes 100Mhz F_CPU as STM32 can change frequency
  static const long CLOCK_CYCLES=(100000000L / 1000000 * DCC_SIGNAL_TIME) >>1;
#else
  static const long CLOCK_CYCLES=(F_CPU / 1000000 * DCC_SIGNAL_TIME) >>1;
#endif

};

class ADCee {
public:
  // On architectures that use the analog read during DCC waveform
  // with specially configured ADC, for example AVR, init must be
  // called PRIOR to the start of the waveform. It returns the
  // current value so that an offset can be initialized.
  static int init(uint8_t pin);
  static int read(uint8_t pin, bool fromISR);
private:
  static void scan();
  static void begin();
  static uint16_t usedpins;
  static int *analogvals;
  friend class DCCWaveform;
  };
#endif
