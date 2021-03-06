/* ES40 emulator.
 * Copyright (C) 2007-2008 by the ES40 Emulator Project
 *
 * WWW    : http://www.es40.org
 * E-mail : camiel@es40.org
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * Although this is not required, the author would appreciate being notified of, 
 * and receiving any modifications you may make to the source code that might serve
 * the general public.
 *
 * Parts of this file based upon the Poco C++ Libraries, which is Copyright (C) 
 * 2004-2006, Applied Informatics Software Engineering GmbH. and Contributors.
 */

/**
 * $Id: Foundation.h,v 1.2 2010/03/11 22:44:16 iamcamiel Exp $
 *
 * X-1.1        Camiel Vanderhoeven                             31-MAY-2008
 *      Initial version for ES40 emulator.
 **/

//
// Foundation.h
//
// $Id: Foundation.h,v 1.2 2010/03/11 22:44:16 iamcamiel Exp $
//
// Library: Foundation
// Package: Core
// Module:  Foundation
//
// Basic definitions for the POCO Foundation library.
// This file must be the first file included by every other Foundation
// header file.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef Foundation_Foundation_INCLUDED
#define Foundation_Foundation_INCLUDED


//
// Include library configuration
//
//CAVA #include "Config.h"

//
// Include platform-specific definitions
//
#include "Platform.h"
#if defined(_WIN32)
	#include "Platform_WIN32.h"
#elif defined(__VMS)
	#include "Platform_VMS.h"
#elif defined(POCO_OS_FAMILY_UNIX)
	#include "Platform_POSIX.h"
#endif


//
// POCO_JOIN
//
// The following piece of macro magic joins the two
// arguments together, even when one of the arguments is
// itself a macro (see 16.3.1 in C++ standard).  The key
// is that macro expansion of macro arguments does not
// occur in POCO_DO_JOIN2 but does in POCO_DO_JOIN.
//
#define POCO_JOIN(X, Y) POCO_DO_JOIN(X, Y)
#define POCO_DO_JOIN(X, Y) POCO_DO_JOIN2(X, Y)
#define POCO_DO_JOIN2(X, Y) X##Y


//
// Pull in basic definitions
//
#include "Bugcheck.h"
#include "Types.h"
#include <string>
#include <stdio.h>

#endif // Foundation_Foundation_INCLUDED
