/**
 * @file    board.h
 * @brief   Board initialization header file.
 * @author  andreika <prometheus.pcb@gmail.com>
 */
 
#ifndef _BOARD_H_
#define _BOARD_H_

/**
 * @brief	The board name 
 */
#define BOARD_NAME "Deucalion/Kinetis"

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
  void setPinConfigurationOverrides(void);
  void setSerialConfigurationOverrides(void);
  void setSdCardConfigurationOverrides(void);
  void setAdcChannelOverrides(void);

/**
 * @brief 	Initialize board specific settings.
 */
  void BOARD_InitDebugConsole(void);

  void __blink(int n);

#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */


