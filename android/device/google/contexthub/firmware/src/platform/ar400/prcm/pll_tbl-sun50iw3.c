/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                         clock control unit module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : pll_cfg_tbl.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-7
* Descript: clock control unit module.
* Update  : date                auther      ver     notes
*           2012-5-7 8:43:10    Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "ccu_i-sun50iw3.h"

#if (defined CONFIG_ARCH_SUN50IW3P1)

//core pll parameter table,
//cacl format: freq = (24MHZ * (N + 1) / ((M + 1) * (1 << P)).
//N  >= 11
//use P factor if freq < 288M

static struct ccu_pll1_factor pll1_table[] =
{      //N             K      M      P
	{11,    0,  3,  2},    //Theory Freq1 = 0   , Actual Freq2 = 22  , Index = 0
	{11,    0,  3,  2},    //Theory Freq1 = 6   , Actual Freq2 = 22  , Index = 1
	{11,    0,  3,  2},    //Theory Freq1 = 12  , Actual Freq2 = 22  , Index = 2
	{11,    0,  3,  2},    //Theory Freq1 = 18  , Actual Freq2 = 22  , Index = 3
	{11,    0,  2,  2},    //Theory Freq1 = 24  , Actual Freq2 = 24  , Index = 4
	{11,    0,  1,  2},    //Theory Freq1 = 30  , Actual Freq2 = 36  , Index = 5
	{11,    0,  1,  2},    //Theory Freq1 = 36  , Actual Freq2 = 36  , Index = 6
	{11,    0,  2,  1},    //Theory Freq1 = 42  , Actual Freq2 = 60  , Index = 7
	{11,    0,  2,  1},    //Theory Freq1 = 48  , Actual Freq2 = 60  , Index = 8
	{11,    0,  2,  1},    //Theory Freq1 = 54  , Actual Freq2 = 60  , Index = 9
	{11,    0,  2,  1},    //Theory Freq1 = 60  , Actual Freq2 = 60  , Index = 10
	{11,    0,  0,  2},    //Theory Freq1 = 66  , Actual Freq2 = 72  , Index = 11
	{11,    0,  0,  2},    //Theory Freq1 = 72  , Actual Freq2 = 72  , Index = 12
	{12,    0,  0,  2},    //Theory Freq1 = 78  , Actual Freq2 = 78  , Index = 13
	{13,    0,  0,  2},    //Theory Freq1 = 84  , Actual Freq2 = 84  , Index = 14
	{14,    0,  0,  2},    //Theory Freq1 = 90  , Actual Freq2 = 90  , Index = 15
	{15,    0,  0,  2},    //Theory Freq1 = 96  , Actual Freq2 = 96  , Index = 16
	{16,    0,  0,  2},    //Theory Freq1 = 102 , Actual Freq2 = 102 , Index = 17
	{17,    0,  0,  2},    //Theory Freq1 = 108 , Actual Freq2 = 108 , Index = 18
	{18,    0,  0,  2},    //Theory Freq1 = 114 , Actual Freq2 = 114 , Index = 19
	{19,    0,  0,  2},    //Theory Freq1 = 120 , Actual Freq2 = 120 , Index = 20
	{20,    0,  0,  2},    //Theory Freq1 = 126 , Actual Freq2 = 132 , Index = 21
	{21,    0,  0,  2},    //Theory Freq1 = 132 , Actual Freq2 = 132 , Index = 22
	{11,    0,  0,  1},    //Theory Freq1 = 138 , Actual Freq2 = 144 , Index = 23
	{11,    0,  0,  1},    //Theory Freq1 = 144 , Actual Freq2 = 144 , Index = 24
	{12,    0,  0,  1},    //Theory Freq1 = 150 , Actual Freq2 = 156 , Index = 25
	{12,    0,  0,  1},    //Theory Freq1 = 156 , Actual Freq2 = 156 , Index = 26
	{13,    0,  0,  1},    //Theory Freq1 = 162 , Actual Freq2 = 168 , Index = 27
	{13,    0,  0,  1},    //Theory Freq1 = 168 , Actual Freq2 = 168 , Index = 28
	{14,    0,  0,  1},    //Theory Freq1 = 174 , Actual Freq2 = 180 , Index = 29
	{14,    0,  0,  1},    //Theory Freq1 = 180 , Actual Freq2 = 180 , Index = 30
	{15,    0,  0,  1},    //Theory Freq1 = 186 , Actual Freq2 = 192 , Index = 31
	{15,    0,  0,  1},    //Theory Freq1 = 192 , Actual Freq2 = 192 , Index = 32
	{16,    0,  0,  1},    //Theory Freq1 = 198 , Actual Freq2 = 204 , Index = 33
	{16,    0,  0,  1},    //Theory Freq1 = 204 , Actual Freq2 = 204 , Index = 34
	{17,    0,  0,  1},    //Theory Freq1 = 210 , Actual Freq2 = 216 , Index = 35
	{17,    0,  0,  1},    //Theory Freq1 = 216 , Actual Freq2 = 216 , Index = 36
	{18,    0,  0,  1},    //Theory Freq1 = 222 , Actual Freq2 = 228 , Index = 37
	{18,    0,  0,  1},    //Theory Freq1 = 228 , Actual Freq2 = 228 , Index = 38
	{19,    0,  0,  1},    //Theory Freq1 = 234 , Actual Freq2 = 240 , Index = 39
	{19,    0,  0,  1},    //Theory Freq1 = 240 , Actual Freq2 = 240 , Index = 40
	{20,    0,  0,  1},    //Theory Freq1 = 246 , Actual Freq2 = 264 , Index = 41
	{20,    0,  0,  1},    //Theory Freq1 = 252 , Actual Freq2 = 264 , Index = 42
	{21,    0,  0,  1},    //Theory Freq1 = 258 , Actual Freq2 = 264 , Index = 43
	{21,    0,  0,  1},    //Theory Freq1 = 264 , Actual Freq2 = 264 , Index = 44
	{11,    0,  0,  0},    //Theory Freq1 = 270 , Actual Freq2 = 288 , Index = 45
	{11,    0,  0,  0},    //Theory Freq1 = 276 , Actual Freq2 = 288 , Index = 46
	{11,    0,  0,  0},    //Theory Freq1 = 282 , Actual Freq2 = 288 , Index = 47
	{11,    0,  0,  0},    //Theory Freq1 = 288 , Actual Freq2 = 288 , Index = 48
	{12,    0,  0,  0},    //Theory Freq1 = 294 , Actual Freq2 = 312 , Index = 49
	{12,    0,  0,  0},    //Theory Freq1 = 300 , Actual Freq2 = 312 , Index = 50
	{12,    0,  0,  0},    //Theory Freq1 = 306 , Actual Freq2 = 312 , Index = 51
	{12,    0,  0,  0},    //Theory Freq1 = 312 , Actual Freq2 = 312 , Index = 52
	{13,    0,  0,  0},    //Theory Freq1 = 318 , Actual Freq2 = 336 , Index = 53
	{13,    0,  0,  0},    //Theory Freq1 = 324 , Actual Freq2 = 336 , Index = 54
	{13,    0,  0,  0},    //Theory Freq1 = 330 , Actual Freq2 = 336 , Index = 55
	{13,    0,  0,  0},    //Theory Freq1 = 336 , Actual Freq2 = 336 , Index = 56
	{14,    0,  0,  0},    //Theory Freq1 = 342 , Actual Freq2 = 360 , Index = 57
	{14,    0,  0,  0},    //Theory Freq1 = 348 , Actual Freq2 = 360 , Index = 58
	{14,    0,  0,  0},    //Theory Freq1 = 354 , Actual Freq2 = 360 , Index = 59
	{14,    0,  0,  0},    //Theory Freq1 = 360 , Actual Freq2 = 360 , Index = 60
	{15,    0,  0,  0},    //Theory Freq1 = 366 , Actual Freq2 = 384 , Index = 61
	{15,    0,  0,  0},    //Theory Freq1 = 372 , Actual Freq2 = 384 , Index = 62
	{15,    0,  0,  0},    //Theory Freq1 = 378 , Actual Freq2 = 384 , Index = 63
	{15,    0,  0,  0},    //Theory Freq1 = 384 , Actual Freq2 = 384 , Index = 64
	{16,    0,  0,  0},    //Theory Freq1 = 390 , Actual Freq2 = 408 , Index = 65
	{16,    0,  0,  0},    //Theory Freq1 = 396 , Actual Freq2 = 408 , Index = 66
	{16,    0,  0,  0},    //Theory Freq1 = 402 , Actual Freq2 = 408 , Index = 67
	{16,    0,  0,  0},    //Theory Freq1 = 408 , Actual Freq2 = 408 , Index = 68
	{17,    0,  0,  0},    //Theory Freq1 = 414 , Actual Freq2 = 432 , Index = 69
	{17,    0,  0,  0},    //Theory Freq1 = 420 , Actual Freq2 = 432 , Index = 70
	{17,    0,  0,  0},    //Theory Freq1 = 426 , Actual Freq2 = 432 , Index = 71
	{17,    0,  0,  0},    //Theory Freq1 = 432 , Actual Freq2 = 432 , Index = 72
	{18,    0,  0,  0},    //Theory Freq1 = 438 , Actual Freq2 = 456 , Index = 73
	{18,    0,  0,  0},    //Theory Freq1 = 444 , Actual Freq2 = 456 , Index = 74
	{18,    0,  0,  0},    //Theory Freq1 = 450 , Actual Freq2 = 456 , Index = 75
	{18,    0,  0,  0},    //Theory Freq1 = 456 , Actual Freq2 = 456 , Index = 76
	{19,    0,  0,  0},    //Theory Freq1 = 462 , Actual Freq2 = 480 , Index = 77
	{19,    0,  0,  0},    //Theory Freq1 = 468 , Actual Freq2 = 480 , Index = 78
	{19,    0,  0,  0},    //Theory Freq1 = 474 , Actual Freq2 = 480 , Index = 79
	{19,    0,  0,  0},    //Theory Freq1 = 480 , Actual Freq2 = 480 , Index = 80
	{20,    0,  0,  0},    //Theory Freq1 = 486 , Actual Freq2 = 504 , Index = 81
	{20,    0,  0,  0},    //Theory Freq1 = 492 , Actual Freq2 = 504 , Index = 82
	{20,    0,  0,  0},    //Theory Freq1 = 498 , Actual Freq2 = 504 , Index = 83
	{20,    0,  0,  0},    //Theory Freq1 = 504 , Actual Freq2 = 504 , Index = 84
	{21,    0,  0,  0},    //Theory Freq1 = 510 , Actual Freq2 = 528 , Index = 85
	{21,    0,  0,  0},    //Theory Freq1 = 516 , Actual Freq2 = 528 , Index = 86
	{21,    0,  0,  0},    //Theory Freq1 = 522 , Actual Freq2 = 528 , Index = 87
	{21,    0,  0,  0},    //Theory Freq1 = 528 , Actual Freq2 = 528 , Index = 88
	{22,    0,  0,  0},    //Theory Freq1 = 534 , Actual Freq2 = 552 , Index = 89
	{22,    0,  0,  0},    //Theory Freq1 = 540 , Actual Freq2 = 552 , Index = 90
	{22,    0,  0,  0},    //Theory Freq1 = 546 , Actual Freq2 = 552 , Index = 91
	{22,    0,  0,  0},    //Theory Freq1 = 552 , Actual Freq2 = 552 , Index = 92
	{23,    0,  0,  0},    //Theory Freq1 = 558 , Actual Freq2 = 576 , Index = 93
	{23,    0,  0,  0},    //Theory Freq1 = 564 , Actual Freq2 = 576 , Index = 94
	{23,    0,  0,  0},    //Theory Freq1 = 570 , Actual Freq2 = 576 , Index = 95
	{23,    0,  0,  0},    //Theory Freq1 = 576 , Actual Freq2 = 576 , Index = 96
	{24,    0,  0,  0},    //Theory Freq1 = 582 , Actual Freq2 = 600 , Index = 97
	{24,    0,  0,  0},    //Theory Freq1 = 588 , Actual Freq2 = 600 , Index = 98
	{24,    0,  0,  0},    //Theory Freq1 = 594 , Actual Freq2 = 600 , Index = 99
	{24,    0,  0,  0},    //Theory Freq1 = 600 , Actual Freq2 = 600 , Index = 100
	{25,    0,  0,  0},    //Theory Freq1 = 606 , Actual Freq2 = 624 , Index = 101
	{25,    0,  0,  0},    //Theory Freq1 = 612 , Actual Freq2 = 624 , Index = 102
	{25,    0,  0,  0},    //Theory Freq1 = 618 , Actual Freq2 = 624 , Index = 103
	{25,    0,  0,  0},    //Theory Freq1 = 624 , Actual Freq2 = 624 , Index = 104
	{26,    0,  0,  0},    //Theory Freq1 = 630 , Actual Freq2 = 648 , Index = 105
	{26,    0,  0,  0},    //Theory Freq1 = 636 , Actual Freq2 = 648 , Index = 106
	{26,    0,  0,  0},    //Theory Freq1 = 642 , Actual Freq2 = 648 , Index = 107
	{26,    0,  0,  0},    //Theory Freq1 = 648 , Actual Freq2 = 648 , Index = 108
	{27,    0,  0,  0},    //Theory Freq1 = 654 , Actual Freq2 = 672 , Index = 109
	{27,    0,  0,  0},    //Theory Freq1 = 660 , Actual Freq2 = 672 , Index = 110
	{27,    0,  0,  0},    //Theory Freq1 = 666 , Actual Freq2 = 672 , Index = 111
	{27,    0,  0,  0},    //Theory Freq1 = 672 , Actual Freq2 = 672 , Index = 112
	{28,    0,  0,  0},    //Theory Freq1 = 678 , Actual Freq2 = 696 , Index = 113
	{28,    0,  0,  0},    //Theory Freq1 = 684 , Actual Freq2 = 696 , Index = 114
	{28,    0,  0,  0},    //Theory Freq1 = 690 , Actual Freq2 = 696 , Index = 115
	{28,    0,  0,  0},    //Theory Freq1 = 696 , Actual Freq2 = 696 , Index = 116
	{29,    0,  0,  0},    //Theory Freq1 = 702 , Actual Freq2 = 720 , Index = 117
	{29,    0,  0,  0},    //Theory Freq1 = 708 , Actual Freq2 = 720 , Index = 118
	{29,    0,  0,  0},    //Theory Freq1 = 714 , Actual Freq2 = 720 , Index = 119
	{29,    0,  0,  0},    //Theory Freq1 = 720 , Actual Freq2 = 720 , Index = 120
	{30,    0,  0,  0},    //Theory Freq1 = 726 , Actual Freq2 = 744 , Index = 121
	{30,    0,  0,  0},    //Theory Freq1 = 732 , Actual Freq2 = 744 , Index = 122
	{30,    0,  0,  0},    //Theory Freq1 = 738 , Actual Freq2 = 744 , Index = 123
	{30,    0,  0,  0},    //Theory Freq1 = 744 , Actual Freq2 = 744 , Index = 124
	{31,    0,  0,  0},    //Theory Freq1 = 750 , Actual Freq2 = 768 , Index = 125
	{31,    0,  0,  0},    //Theory Freq1 = 756 , Actual Freq2 = 768 , Index = 126
	{31,    0,  0,  0},    //Theory Freq1 = 762 , Actual Freq2 = 768 , Index = 127
	{31,    0,  0,  0},    //Theory Freq1 = 768 , Actual Freq2 = 768 , Index = 128
	{32,    0,  0,  0},    //Theory Freq1 = 774 , Actual Freq2 = 792 , Index = 129
	{32,    0,  0,  0},    //Theory Freq1 = 780 , Actual Freq2 = 792 , Index = 130
	{32,    0,  0,  0},    //Theory Freq1 = 786 , Actual Freq2 = 792 , Index = 131
	{32,    0,  0,  0},    //Theory Freq1 = 792 , Actual Freq2 = 792 , Index = 132
	{33,    0,  0,  0},    //Theory Freq1 = 798 , Actual Freq2 = 816 , Index = 133
	{33,    0,  0,  0},    //Theory Freq1 = 804 , Actual Freq2 = 816 , Index = 134
	{33,    0,  0,  0},    //Theory Freq1 = 810 , Actual Freq2 = 816 , Index = 135
	{33,    0,  0,  0},    //Theory Freq1 = 816 , Actual Freq2 = 816 , Index = 136
	{34,    0,  0,  0},    //Theory Freq1 = 822 , Actual Freq2 = 840 , Index = 137
	{34,    0,  0,  0},    //Theory Freq1 = 828 , Actual Freq2 = 840 , Index = 138
	{34,    0,  0,  0},    //Theory Freq1 = 834 , Actual Freq2 = 840 , Index = 139
	{34,    0,  0,  0},    //Theory Freq1 = 840 , Actual Freq2 = 840 , Index = 140
	{35,    0,  0,  0},    //Theory Freq1 = 846 , Actual Freq2 = 864 , Index = 141
	{35,    0,  0,  0},    //Theory Freq1 = 852 , Actual Freq2 = 864 , Index = 142
	{35,    0,  0,  0},    //Theory Freq1 = 858 , Actual Freq2 = 864 , Index = 143
	{35,    0,  0,  0},    //Theory Freq1 = 864 , Actual Freq2 = 864 , Index = 144
	{36,    0,  0,  0},    //Theory Freq1 = 870 , Actual Freq2 = 888 , Index = 145
	{36,    0,  0,  0},    //Theory Freq1 = 876 , Actual Freq2 = 888 , Index = 146
	{36,    0,  0,  0},    //Theory Freq1 = 882 , Actual Freq2 = 888 , Index = 147
	{36,    0,  0,  0},    //Theory Freq1 = 888 , Actual Freq2 = 888 , Index = 148
	{37,    0,  0,  0},    //Theory Freq1 = 894 , Actual Freq2 = 912 , Index = 149
	{37,    0,  0,  0},    //Theory Freq1 = 900 , Actual Freq2 = 912 , Index = 150
	{37,    0,  0,  0},    //Theory Freq1 = 906 , Actual Freq2 = 912 , Index = 151
	{37,    0,  0,  0},    //Theory Freq1 = 912 , Actual Freq2 = 912 , Index = 152
	{38,    0,  0,  0},    //Theory Freq1 = 918 , Actual Freq2 = 936 , Index = 153
	{38,    0,  0,  0},    //Theory Freq1 = 924 , Actual Freq2 = 936 , Index = 154
	{38,    0,  0,  0},    //Theory Freq1 = 930 , Actual Freq2 = 936 , Index = 155
	{38,    0,  0,  0},    //Theory Freq1 = 936 , Actual Freq2 = 936 , Index = 156
	{39,    0,  0,  0},    //Theory Freq1 = 942 , Actual Freq2 = 960 , Index = 157
	{39,    0,  0,  0},    //Theory Freq1 = 948 , Actual Freq2 = 960 , Index = 158
	{39,    0,  0,  0},    //Theory Freq1 = 954 , Actual Freq2 = 960 , Index = 159
	{39,    0,  0,  0},    //Theory Freq1 = 960 , Actual Freq2 = 960 , Index = 160
	{40,    0,  0,  0},    //Theory Freq1 = 966 , Actual Freq2 = 984 , Index = 161
	{40,    0,  0,  0},    //Theory Freq1 = 972 , Actual Freq2 = 984 , Index = 162
	{40,    0,  0,  0},    //Theory Freq1 = 978 , Actual Freq2 = 984 , Index = 163
	{40,    0,  0,  0},    //Theory Freq1 = 984 , Actual Freq2 = 984 , Index = 164
	{41,    0,  0,  0},    //Theory Freq1 = 990 , Actual Freq2 = 1008, Index = 165
	{41,    0,  0,  0},    //Theory Freq1 = 996 , Actual Freq2 = 1008, Index = 166
	{41,    0,  0,  0},    //Theory Freq1 = 1002, Actual Freq2 = 1008, Index = 167
	{41,    0,  0,  0},    //Theory Freq1 = 1008, Actual Freq2 = 1008, Index = 168
	{42,    0,  0,  0},    //Theory Freq1 = 1014, Actual Freq2 = 1032, Index = 169
	{42,    0,  0,  0},    //Theory Freq1 = 1020, Actual Freq2 = 1032, Index = 170
	{42,    0,  0,  0},    //Theory Freq1 = 1026, Actual Freq2 = 1032, Index = 171
	{42,    0,  0,  0},    //Theory Freq1 = 1032, Actual Freq2 = 1032, Index = 172
	{43,    0,  0,  0},    //Theory Freq1 = 1038, Actual Freq2 = 1056, Index = 173
	{43,    0,  0,  0},    //Theory Freq1 = 1044, Actual Freq2 = 1056, Index = 174
	{43,    0,  0,  0},    //Theory Freq1 = 1050, Actual Freq2 = 1056, Index = 175
	{43,    0,  0,  0},    //Theory Freq1 = 1056, Actual Freq2 = 1056, Index = 176
	{44,    0,  0,  0},    //Theory Freq1 = 1062, Actual Freq2 = 1080, Index = 177
	{44,    0,  0,  0},    //Theory Freq1 = 1068, Actual Freq2 = 1080, Index = 178
	{44,    0,  0,  0},    //Theory Freq1 = 1074, Actual Freq2 = 1080, Index = 179
	{44,    0,  0,  0},    //Theory Freq1 = 1080, Actual Freq2 = 1080, Index = 180
	{45,    0,  0,  0},    //Theory Freq1 = 1086, Actual Freq2 = 1104, Index = 181
	{45,    0,  0,  0},    //Theory Freq1 = 1092, Actual Freq2 = 1104, Index = 182
	{45,    0,  0,  0},    //Theory Freq1 = 1098, Actual Freq2 = 1104, Index = 183
	{45,    0,  0,  0},    //Theory Freq1 = 1104, Actual Freq2 = 1104, Index = 184
	{46,    0,  0,  0},    //Theory Freq1 = 1110, Actual Freq2 = 1128, Index = 185
	{46,    0,  0,  0},    //Theory Freq1 = 1116, Actual Freq2 = 1128, Index = 186
	{46,    0,  0,  0},    //Theory Freq1 = 1122, Actual Freq2 = 1128, Index = 187
	{46,    0,  0,  0},    //Theory Freq1 = 1128, Actual Freq2 = 1128, Index = 188
	{47,    0,  0,  0},    //Theory Freq1 = 1134, Actual Freq2 = 1152, Index = 189
	{47,    0,  0,  0},    //Theory Freq1 = 1140, Actual Freq2 = 1152, Index = 190
	{47,    0,  0,  0},    //Theory Freq1 = 1146, Actual Freq2 = 1152, Index = 191
	{47,    0,  0,  0},    //Theory Freq1 = 1152, Actual Freq2 = 1152, Index = 192
	{48,    0,  0,  0},    //Theory Freq1 = 1158, Actual Freq2 = 1176, Index = 193
	{48,    0,  0,  0},    //Theory Freq1 = 1164, Actual Freq2 = 1176, Index = 194
	{48,    0,  0,  0},    //Theory Freq1 = 1170, Actual Freq2 = 1176, Index = 195
	{48,    0,  0,  0},    //Theory Freq1 = 1176, Actual Freq2 = 1176, Index = 196
	{49,    0,  0,  0},    //Theory Freq1 = 1182, Actual Freq2 = 1200, Index = 197
	{49,    0,  0,  0},    //Theory Freq1 = 1188, Actual Freq2 = 1200, Index = 198
	{49,    0,  0,  0},    //Theory Freq1 = 1194, Actual Freq2 = 1200, Index = 199
	{49,    0,  0,  0},    //Theory Freq1 = 1200, Actual Freq2 = 1200, Index = 200
	{50,    0,  0,  0},    //Theory Freq1 = 1206, Actual Freq2 = 1224, Index = 201
	{50,    0,  0,  0},    //Theory Freq1 = 1212, Actual Freq2 = 1224, Index = 202
	{50,    0,  0,  0},    //Theory Freq1 = 1218, Actual Freq2 = 1224, Index = 203
	{50,    0,  0,  0},    //Theory Freq1 = 1224, Actual Freq2 = 1224, Index = 204
	{51,    0,  0,  0},    //Theory Freq1 = 1230, Actual Freq2 = 1248, Index = 205
	{51,    0,  0,  0},    //Theory Freq1 = 1236, Actual Freq2 = 1248, Index = 206
	{51,    0,  0,  0},    //Theory Freq1 = 1242, Actual Freq2 = 1248, Index = 207
	{51,    0,  0,  0},    //Theory Freq1 = 1248, Actual Freq2 = 1248, Index = 208
	{52,    0,  0,  0},    //Theory Freq1 = 1254, Actual Freq2 = 1272, Index = 209
	{52,    0,  0,  0},    //Theory Freq1 = 1260, Actual Freq2 = 1272, Index = 210
	{52,    0,  0,  0},    //Theory Freq1 = 1266, Actual Freq2 = 1272, Index = 211
	{52,    0,  0,  0},    //Theory Freq1 = 1272, Actual Freq2 = 1272, Index = 212
	{53,    0,  0,  0},    //Theory Freq1 = 1278, Actual Freq2 = 1296, Index = 213
	{53,    0,  0,  0},    //Theory Freq1 = 1284, Actual Freq2 = 1296, Index = 214
	{53,    0,  0,  0},    //Theory Freq1 = 1290, Actual Freq2 = 1296, Index = 215
	{53,    0,  0,  0},    //Theory Freq1 = 1296, Actual Freq2 = 1296, Index = 216
	{54,    0,  0,  0},    //Theory Freq1 = 1302, Actual Freq2 = 1320, Index = 217
	{54,    0,  0,  0},    //Theory Freq1 = 1308, Actual Freq2 = 1320, Index = 218
	{54,    0,  0,  0},    //Theory Freq1 = 1314, Actual Freq2 = 1320, Index = 219
	{54,    0,  0,  0},    //Theory Freq1 = 1320, Actual Freq2 = 1320, Index = 220
	{55,    0,  0,  0},    //Theory Freq1 = 1326, Actual Freq2 = 1344, Index = 221
	{55,    0,  0,  0},    //Theory Freq1 = 1332, Actual Freq2 = 1344, Index = 222
	{55,    0,  0,  0},    //Theory Freq1 = 1338, Actual Freq2 = 1344, Index = 223
	{55,    0,  0,  0},    //Theory Freq1 = 1344, Actual Freq2 = 1344, Index = 224
	{56,    0,  0,  0},    //Theory Freq1 = 1350, Actual Freq2 = 1368, Index = 225
	{56,    0,  0,  0},    //Theory Freq1 = 1356, Actual Freq2 = 1368, Index = 226
	{56,    0,  0,  0},    //Theory Freq1 = 1362, Actual Freq2 = 1368, Index = 227
	{56,    0,  0,  0},    //Theory Freq1 = 1368, Actual Freq2 = 1368, Index = 228
	{57,    0,  0,  0},    //Theory Freq1 = 1374, Actual Freq2 = 1392, Index = 229
	{57,    0,  0,  0},    //Theory Freq1 = 1380, Actual Freq2 = 1392, Index = 230
	{57,    0,  0,  0},    //Theory Freq1 = 1386, Actual Freq2 = 1392, Index = 231
	{57,    0,  0,  0},    //Theory Freq1 = 1392, Actual Freq2 = 1392, Index = 232
	{58,    0,  0,  0},    //Theory Freq1 = 1398, Actual Freq2 = 1416, Index = 233
	{58,    0,  0,  0},    //Theory Freq1 = 1404, Actual Freq2 = 1416, Index = 234
	{58,    0,  0,  0},    //Theory Freq1 = 1410, Actual Freq2 = 1416, Index = 235
	{58,    0,  0,  0},    //Theory Freq1 = 1416, Actual Freq2 = 1416, Index = 236
	{59,    0,  0,  0},    //Theory Freq1 = 1422, Actual Freq2 = 1440, Index = 237
	{59,    0,  0,  0},    //Theory Freq1 = 1428, Actual Freq2 = 1440, Index = 238
	{59,    0,  0,  0},    //Theory Freq1 = 1434, Actual Freq2 = 1440, Index = 239
	{59,    0,  0,  0},    //Theory Freq1 = 1440, Actual Freq2 = 1440, Index = 240
	{60,    0,  0,  0},    //Theory Freq1 = 1446, Actual Freq2 = 1464, Index = 241
	{60,    0,  0,  0},    //Theory Freq1 = 1452, Actual Freq2 = 1464, Index = 242
	{60,    0,  0,  0},    //Theory Freq1 = 1458, Actual Freq2 = 1464, Index = 243
	{60,    0,  0,  0},    //Theory Freq1 = 1464, Actual Freq2 = 1464, Index = 244
	{61,    0,  0,  0},    //Theory Freq1 = 1470, Actual Freq2 = 1488, Index = 245
	{61,    0,  0,  0},    //Theory Freq1 = 1476, Actual Freq2 = 1488, Index = 246
	{61,    0,  0,  0},    //Theory Freq1 = 1482, Actual Freq2 = 1488, Index = 247
	{61,    0,  0,  0},    //Theory Freq1 = 1488, Actual Freq2 = 1488, Index = 248
	{62,    0,  0,  0},    //Theory Freq1 = 1494, Actual Freq2 = 1512, Index = 249
	{62,    0,  0,  0},    //Theory Freq1 = 1500, Actual Freq2 = 1512, Index = 250
	{62,    0,  0,  0},    //Theory Freq1 = 1506, Actual Freq2 = 1512, Index = 251
	{62,    0,  0,  0},    //Theory Freq1 = 1512, Actual Freq2 = 1512, Index = 252
	{63,    0,  0,  0},    //Theory Freq1 = 1518, Actual Freq2 = 1536, Index = 253
	{63,    0,  0,  0},    //Theory Freq1 = 1524, Actual Freq2 = 1536, Index = 254
	{63,    0,  0,  0},    //Theory Freq1 = 1530, Actual Freq2 = 1536, Index = 255
	{63,    0,  0,  0},    //Theory Freq1 = 1536, Actual Freq2 = 1536, Index = 256
	{64,    0,  0,  0},    //Theory Freq1 = 1542, Actual Freq2 = 1560, Index = 257
	{64,    0,  0,  0},    //Theory Freq1 = 1548, Actual Freq2 = 1560, Index = 258
	{64,    0,  0,  0},    //Theory Freq1 = 1554, Actual Freq2 = 1560, Index = 259
	{64,    0,  0,  0},    //Theory Freq1 = 1560, Actual Freq2 = 1560, Index = 260
	{65,    0,  0,  0},    //Theory Freq1 = 1566, Actual Freq2 = 1584, Index = 261
	{65,    0,  0,  0},    //Theory Freq1 = 1572, Actual Freq2 = 1584, Index = 262
	{65,    0,  0,  0},    //Theory Freq1 = 1578, Actual Freq2 = 1584, Index = 263
	{65,    0,  0,  0},    //Theory Freq1 = 1584, Actual Freq2 = 1584, Index = 264
	{66,    0,  0,  0},    //Theory Freq1 = 1590, Actual Freq2 = 1608, Index = 265
	{66,    0,  0,  0},    //Theory Freq1 = 1596, Actual Freq2 = 1608, Index = 266
	{66,    0,  0,  0},    //Theory Freq1 = 1602, Actual Freq2 = 1608, Index = 267
	{66,    0,  0,  0},    //Theory Freq1 = 1608, Actual Freq2 = 1608, Index = 268
	{67,    0,  0,  0},    //Theory Freq1 = 1614, Actual Freq2 = 1632, Index = 269
	{67,    0,  0,  0},    //Theory Freq1 = 1620, Actual Freq2 = 1632, Index = 270
	{67,    0,  0,  0},    //Theory Freq1 = 1626, Actual Freq2 = 1632, Index = 271
	{67,    0,  0,  0},    //Theory Freq1 = 1632, Actual Freq2 = 1632, Index = 272
	{68,    0,  0,  0},    //Theory Freq1 = 1638, Actual Freq2 = 1656, Index = 273
	{68,    0,  0,  0},    //Theory Freq1 = 1644, Actual Freq2 = 1656, Index = 274
	{68,    0,  0,  0},    //Theory Freq1 = 1650, Actual Freq2 = 1656, Index = 275
	{68,    0,  0,  0},    //Theory Freq1 = 1656, Actual Freq2 = 1656, Index = 276
	{69,    0,  0,  0},    //Theory Freq1 = 1662, Actual Freq2 = 1680, Index = 277
	{69,    0,  0,  0},    //Theory Freq1 = 1668, Actual Freq2 = 1680, Index = 278
	{69,    0,  0,  0},    //Theory Freq1 = 1674, Actual Freq2 = 1680, Index = 279
	{69,    0,  0,  0},    //Theory Freq1 = 1680, Actual Freq2 = 1680, Index = 280
	{70,    0,  0,  0},    //Theory Freq1 = 1686, Actual Freq2 = 1704, Index = 281
	{70,    0,  0,  0},    //Theory Freq1 = 1692, Actual Freq2 = 1704, Index = 282
	{70,    0,  0,  0},    //Theory Freq1 = 1698, Actual Freq2 = 1704, Index = 283
	{70,    0,  0,  0},    //Theory Freq1 = 1704, Actual Freq2 = 1704, Index = 284
	{71,    0,  0,  0},    //Theory Freq1 = 1710, Actual Freq2 = 1728, Index = 285
	{71,    0,  0,  0},    //Theory Freq1 = 1716, Actual Freq2 = 1728, Index = 286
	{71,    0,  0,  0},    //Theory Freq1 = 1722, Actual Freq2 = 1728, Index = 287
	{71,    0,  0,  0},    //Theory Freq1 = 1728, Actual Freq2 = 1728, Index = 288
	{72,    0,  0,  0},    //Theory Freq1 = 1734, Actual Freq2 = 1752, Index = 289
	{72,    0,  0,  0},    //Theory Freq1 = 1740, Actual Freq2 = 1752, Index = 290
	{72,    0,  0,  0},    //Theory Freq1 = 1746, Actual Freq2 = 1752, Index = 291
	{72,    0,  0,  0},    //Theory Freq1 = 1752, Actual Freq2 = 1752, Index = 292
	{73,    0,  0,  0},    //Theory Freq1 = 1758, Actual Freq2 = 1776, Index = 293
	{73,    0,  0,  0},    //Theory Freq1 = 1764, Actual Freq2 = 1776, Index = 294
	{73,    0,  0,  0},    //Theory Freq1 = 1770, Actual Freq2 = 1776, Index = 295
	{73,    0,  0,  0},    //Theory Freq1 = 1776, Actual Freq2 = 1776, Index = 296
	{74,    0,  0,  0},    //Theory Freq1 = 1782, Actual Freq2 = 1800, Index = 297
	{74,    0,  0,  0},    //Theory Freq1 = 1788, Actual Freq2 = 1800, Index = 298
	{74,    0,  0,  0},    //Theory Freq1 = 1794, Actual Freq2 = 1800, Index = 299
	{74,    0,  0,  0},    //Theory Freq1 = 1800, Actual Freq2 = 1800, Index = 300
	{75,    0,  0,  0},    //Theory Freq1 = 1806, Actual Freq2 = 1824, Index = 301
	{75,    0,  0,  0},    //Theory Freq1 = 1812, Actual Freq2 = 1824, Index = 302
	{75,    0,  0,  0},    //Theory Freq1 = 1818, Actual Freq2 = 1824, Index = 303
	{75,    0,  0,  0},    //Theory Freq1 = 1824, Actual Freq2 = 1824, Index = 304
	{76,    0,  0,  0},    //Theory Freq1 = 1830, Actual Freq2 = 1848, Index = 305
	{76,    0,  0,  0},    //Theory Freq1 = 1836, Actual Freq2 = 1848, Index = 306
	{76,    0,  0,  0},    //Theory Freq1 = 1842, Actual Freq2 = 1848, Index = 307
	{76,    0,  0,  0},    //Theory Freq1 = 1848, Actual Freq2 = 1848, Index = 308
	{77,    0,  0,  0},    //Theory Freq1 = 1854, Actual Freq2 = 1872, Index = 309
	{77,    0,  0,  0},    //Theory Freq1 = 1860, Actual Freq2 = 1872, Index = 310
	{77,    0,  0,  0},    //Theory Freq1 = 1866, Actual Freq2 = 1872, Index = 311
	{77,    0,  0,  0},    //Theory Freq1 = 1872, Actual Freq2 = 1872, Index = 312
	{78,    0,  0,  0},    //Theory Freq1 = 1878, Actual Freq2 = 1896, Index = 313
	{78,    0,  0,  0},    //Theory Freq1 = 1884, Actual Freq2 = 1896, Index = 314
	{78,    0,  0,  0},    //Theory Freq1 = 1890, Actual Freq2 = 1896, Index = 315
	{78,    0,  0,  0},    //Theory Freq1 = 1896, Actual Freq2 = 1896, Index = 316
	{79,    0,  0,  0},    //Theory Freq1 = 1902, Actual Freq2 = 1920, Index = 317
	{79,    0,  0,  0},    //Theory Freq1 = 1908, Actual Freq2 = 1920, Index = 318
	{79,    0,  0,  0},    //Theory Freq1 = 1914, Actual Freq2 = 1920, Index = 319
	{79,    0,  0,  0},    //Theory Freq1 = 1920, Actual Freq2 = 1920, Index = 320
	{80,    0,  0,  0},    //Theory Freq1 = 1926, Actual Freq2 = 1944, Index = 321
	{80,    0,  0,  0},    //Theory Freq1 = 1932, Actual Freq2 = 1944, Index = 322
	{80,    0,  0,  0},    //Theory Freq1 = 1938, Actual Freq2 = 1944, Index = 323
	{80,    0,  0,  0},    //Theory Freq1 = 1944, Actual Freq2 = 1944, Index = 324
	{81,    0,  0,  0},    //Theory Freq1 = 1950, Actual Freq2 = 1968, Index = 325
	{81,    0,  0,  0},    //Theory Freq1 = 1956, Actual Freq2 = 1968, Index = 326
	{81,    0,  0,  0},    //Theory Freq1 = 1962, Actual Freq2 = 1968, Index = 327
	{81,    0,  0,  0},    //Theory Freq1 = 1968, Actual Freq2 = 1968, Index = 328
	{82,    0,  0,  0},    //Theory Freq1 = 1974, Actual Freq2 = 1992, Index = 329
	{82,    0,  0,  0},    //Theory Freq1 = 1980, Actual Freq2 = 1992, Index = 330
	{82,    0,  0,  0},    //Theory Freq1 = 1986, Actual Freq2 = 1992, Index = 331
	{82,    0,  0,  0},    //Theory Freq1 = 1992, Actual Freq2 = 1992, Index = 332
	{83,    0,  0,  0},    //Theory Freq1 = 1998, Actual Freq2 = 2016, Index = 333
	{83,    0,  0,  0},    //Theory Freq1 = 2004, Actual Freq2 = 2016, Index = 334
	{83,    0,  0,  0},    //Theory Freq1 = 2010, Actual Freq2 = 2016, Index = 335
	{83,    0,  0,  0},    //Theory Freq1 = 2016, Actual Freq2 = 2016, Index = 336
};

s32 ccu_calc_pll1_factor(struct ccu_pll1_factor *factor, u32 rate)
{
	s32 index;

	if(rate > 2016000000)
	{
		rate = 2016000000;
	}

	/* div round up */
	index = (rate + 6000000 - 1) / 6000000;

	factor->factor_n = pll1_table[index].factor_n;
	factor->factor_k = pll1_table[index].factor_k;
	factor->factor_m = pll1_table[index].factor_m;
	factor->factor_p = pll1_table[index].factor_p;

	return OK;
}
#endif // sun50iw3
