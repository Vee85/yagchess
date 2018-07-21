/*
 * chess_dconst.hpp
 * 
 * Copyright 2017 Valentino Esposito <valentinoe85@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#ifndef CHESS_DCONST_H_DEF
#define CHESS_DCONST_H_DEF 1

//enumerations
enum c_color {black, white};
enum wpiece {generic, pawn, rock, knight, bishop, queen, king, fakepawn};
enum gamefinal {notfinished, tie, blackwins, whitewins};
enum howplopt {huhu, huce, cehu, cece, alte};

//preprocessor constants
#define MINX 0
#define MAXX 8
#define MINY 0
#define MAXY 8

#endif
