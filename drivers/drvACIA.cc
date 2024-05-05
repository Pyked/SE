/* \file drvACIA.cc
   \brief Routines of the ACIA device driver
//
//      The ACIA is an asynchronous device (requests return
//      immediately, and an interrupt happens later on).
//      This is a layer on top of the ACIA.
//      Two working modes are to be implemented in assignment 2:
//      a Busy Waiting mode and an Interrupt mode. The Busy Waiting
//      mode implements a synchronous IO whereas IOs are asynchronous
//      IOs are implemented in the Interrupt mode (see the Nachos
//      roadmap for further details).
 * -----------------------------------------------------
 * This file is part of the Nachos-RiscV distribution
 * Copyright (c) 2022 University of Rennes 1.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details
 * (see see <http://www.gnu.org/licenses/>).
 * -----------------------------------------------------
*/

/* Includes */
#include "kernel/system.h" // for the ACIA object
#include "kernel/synch.h"
#include "kernel/msgerror.h"
#include "machine/ACIA.h"
#include "drivers/drvACIA.h"

/* Defines */
#define ETUDIANTS_TP

//-------------------------------------------------------------------------
// DriverACIA::DriverACIA()
/*! Constructor.
  Initialize the ACIA driver.
  In the ACIA Interrupt mode,
  initialize the reception index and semaphores and allow
  reception interrupts.
  In the ACIA Busy Waiting mode, simply inittialize the ACIA
  working mode and create the semaphore.
  */
//-------------------------------------------------------------------------

#ifndef ETUDIANTS_TP
DriverACIA::DriverACIA()
{
  printf("**** Warning: contructor of the ACIA driver not implemented yet\n");

  exit(ERROR);

}
#endif

#ifdef ETUDIANTS_TP
// Constructor
DriverACIA::DriverACIA() {
  /* Je me demande si on doit vraiement initialiser les buffers et l'index d'envoi ici.
  // Initialize send and receive buffers
  for (int i = 0; i < BUFFER_SIZE; i++) {
    send_buffer[i] = '\0';
    receive_buffer[i] = '\0';
  }
  
  // Initialize send buffers index
  ind_send = 0;
  */

  // Check if the ACIA is in Interrupt mode or Busy Waiting mode
  if (g_cfg->ACIA == ACIA_INTERRUPT) {
    // initialize the reception index and semaphores and allow reception interrupts
    ind_rec = 0;
    send_sema = new Semaphore("send_sema", 1);
    receive_sema = new Semaphore("receive_sema", 1);

    // Enable ACIA reception interrupt
    g_machine->acia->SetWorkingMode(REC_INTERRUPT);
  } else if (g_cfg->ACIA == ACIA_BUSY_WAITING) {
    // Initialize the ACIA working mode
    g_machine->acia->SetWorkingMode(ACIA_BUSY_WAITING);
    
    // Initialize the semaphores
    send_sema = new Semaphore("send_sema", 1);
    receive_sema = new Semaphore("receive_sema", 1);
  }
}
#endif

//-------------------------------------------------------------------------
// DriverACIA::TtySend(char* buff)
/*! Routine to send a message through the ACIA (Busy Waiting or Interrupt mode)
 */
//-------------------------------------------------------------------------

#ifndef ETUDIANTS_TP
int DriverACIA::TtySend(char *buff)
{
  printf("**** Warning: method Tty_Send of the ACIA driver not implemented yet\n");

  exit(ERROR);

  return 0;

}
#endif

#ifdef ETUDIANTS_TP
int DriverACIA::TtySend(char *buff) {
  int sent_char = 0;
  // Check the ACIA working mode
  if (g_machine->acia->GetWorkingMode() == BUSY_WAITING) {
    // Loop until the entire buffer is sent
    while (buff[sent_char] != '\0') {
      // Check is ACIA is ready to transmit
      while (!g_machine->acia->GetOutputStateReg()) {}
      // Sent a character and move to the next character
      g_machine->acia->PutChar(buff[sent_char++]);
    }
  } else {
    send_sema->P();
    // In the Interrupt mode, send all the characters in the buffer
    g_machine->acia->SetWorkingMode(SEND_INTERRUPT);
    while (sent_char < strlen(buff)) {
      send_buffer[ind_send] = buff[sent_char++];
      InterruptSend();
    }
  }

  // Return the number of characters sent
  return sent_char; 
}
#endif

//-------------------------------------------------------------------------
// DriverACIA::TtyReceive(char* buff,int length)
/*! Routine to reveive a message through the ACIA
//  (Busy Waiting and Interrupt mode).
  */
//-------------------------------------------------------------------------

#ifndef ETUDIANTS_TP
int DriverACIA::TtyReceive(char *buff, int lg)
{
  printf("**** Warning: method Tty_Receive of the ACIA driver not implemented yet\n");

  exit(ERROR);

  return 0;

}
#endif

#ifdef ETUDIANTS_TP
int DriverACIA::TtyReceive(char *buff, int lg) {
  int received_char = 0;
  if (g_machine->acia->GetWorkingMode() == BUSY_WAITING) {
    while (received_char < lg) {
      // wait until there are a characters to receive available in the input buffer
      while (g_machine->acia->GetInputStateReg() == EMPTY) {}
      // Receive a character from ACIA
      buff[received_char++] = g_machine->acia->GetChar();
      // Check if the end of the message is reached
      if (buff[received_char - 1] == '\0') {
        break;
      }
    }
  } else {
      // Wait until the semaphore is released
      receive_sema->P();
      // Copy the received character in the receive buffer to buff
      while (received_char < lg) {
        InterruptReceive();
        buff[received_char++] = receive_buffer[ind_rec];
      }
    }
  
  // Return the number of characters received
  return received_char;
}
#endif

//-------------------------------------------------------------------------
// DriverACIA::InterruptSend()
/*! Emission interrupt handler.
  Used in the ACIA Interrupt mode only.
  Detects when it's the end of the message (if so, releases the send_sema semaphore), else sends the next character according to index ind_send.
  */
//-------------------------------------------------------------------------

#ifndef ETUDIANTS_TP
void DriverACIA::InterruptSend()
{
  printf("**** Warning: send interrupt handler not implemented yet\n");

  exit(ERROR);

}
#endif

#ifdef ETUDIANTS_TP
void DriverACIA::InterruptSend() {
  // Check if there are more characters to send
  if (send_buffer[ind_send] != '\0') {
    // Send the next character
    g_machine->acia->PutChar(send_buffer[ind_send++]);
  } else {
    // Release the send_sema semaphore
    send_sema->V();
  }
}
#endif

//-------------------------------------------------------------------------
// DriverACIA::Interrupt_receive()
/*! Reception interrupt handler.
  Used in the ACIA Interrupt mode only. Reveices a character through the ACIA.
  Releases the receive_sema semaphore and disables reception
  interrupts when the last character of the message is received
  (character '\0').
  */
//-------------------------------------------------------------------------

#ifndef ETUDIANTS_TP
void DriverACIA::InterruptReceive()
{
  printf("**** Warning: receive interrupt handler not implemented yet\n");

  exit(ERROR);

}
#endif

#ifdef ETUDIANTS_TP
void DriverACIA::InterruptReceive() {
  // Receive a character from ACIA
  char received_char = g_machine->acia->GetChar();
  // Store the received character in the reception buffer
  receive_buffer[ind_rec++] = received_char;
  // Check if the end of the message is reached
  if (received_char == '\0') {
    // Release the receive_sema semaphore
    receive_sema->V();
  }
}
#endif
