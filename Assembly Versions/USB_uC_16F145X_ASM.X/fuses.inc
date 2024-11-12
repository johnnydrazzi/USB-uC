;*******************************************************************************
;                                                                              *
;    Copyright (c) 2024 John Izzard                                            *
;                                                                              *
;    Permission is hereby granted, free of charge, to any person obtaining     *
;    a copy of this software and associated documentation files (the           *
;    "Software"), to deal in the Software without restriction, including       *
;    without limitation the rights to use, copy, modify, merge, publish,       *
;    distribute, sublicense, and/or sell copies of the Software, and to        *
;    permit persons to whom the Software is furnished to do so, subject to     *
;    the following conditions:                                                 *
;                                                                              *
;    The above copyright notice and this permission notice shall be            *
;    included in all copies or substantial portions of the Software.           *
;                                                                              *
;    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,           *
;    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF        *
;    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.    *
;    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY      *
;    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,      *
;    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE         *
;    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                    *
;                                                                              *
;*******************************************************************************
;                                                                              *
;    Filename: fuses.inc                                                       *
;    Date: 2024-11-12                                                          *
;    File Version: 1.0.1                                                       *
;    Author: John Izzard                                                       *
;    Company: N/A                                                              *
;    Description:                                                              *
;                                                                              *
;*******************************************************************************
;                                                                              *
;    Revision History:                                                         *
;    File Version 1.0.1 - 2024-11-12                                           *
;    - Changed: MIT license.                                                   *
;                                                                              *
;    File Version 1.0.0 - 2021-05-01                                           *
;    - Added: Initial release.                                                 *
;                                                                              *
;*******************************************************************************
    
    ifndef FUSES_INC
    #define FUSES_INC
    
    include "config.inc"

    __config _CONFIG1, _FOSC & _WDTE_SWDTEN & _PWRTE_ON & _MCLRE & _CP_OFF & _BOREN_SBODEN & _CLKOUTEN_OFF & _IESO_OFF & _FCMEN_OFF
    __config _CONFIG2, _WRT_OFF & _CPUDIV_NOCLKDIV & _USBLSCLK_48MHz & _PLLMULT_3x & _PLLEN_DISABLED & _STVREN_ON & _BORV_HI & _LPBOR_OFF & _LVP
    
    endif


