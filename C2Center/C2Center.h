
// C2Center.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CC2CenterApp:
// See C2Center.cpp for the implementation of this class
//

class CC2CenterApp : public CWinApp
{
public:
	CC2CenterApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CC2CenterApp theApp;