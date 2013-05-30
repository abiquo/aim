/**
 * Abiquo community edition
 * cloud management application for hybrid clouds
 * Copyright (C) 2008-2010 - Abiquo Holdings S.L.
 *
 * This application is free software; you can redistribute it and/or
 * modify it under the terms of the GNU LESSER GENERAL PUBLIC
 * LICENSE as published by the Free Software Foundation under
 * version 3 of the License
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * LESSER GENERAL PUBLIC LICENSE v.3 for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef ASCII_LOGO
#define ASCII_LOGO

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <string.h>

#define PRINT_ASCII_LOGO(version) do { syslog(LOG_ERR, one_line_logo, version); ::fprintf(stderr, ascii_logo, version); } while (0)

const char *ascii_logo =
"        __    _                     ___    ____ __  ___\r\n"
" ___ _ / /   (_)___ _ __ __ ___    / _ |  /  _//  |/  /\r\n"
"/ _ `// _ \\ / // _ `// // // _ \\  / __ | _/ / / /|_/ /\r\n"
"\\_,_//_.__//_/ \\_, / \\_,_/ \\___/ /_/ |_|/___//_/  /_/\r\n"
"                /_/ v%s\r\n\r\n";

const char *one_line_logo = "☁  abiquo aim - v%s ☁ ";

#endif

