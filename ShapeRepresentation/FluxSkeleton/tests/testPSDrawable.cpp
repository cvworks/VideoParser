/************************************************************************

   File:		testPSDrawable.cpp

   Author(s):		Pavel Dimitrov

   Created:		12 Jun 2002

   Last Revision:	$Date: 2002/06/23 05:42:09 $

   Description:	

   $Revision: 1.1.1.1 $

   $Log: testPSDrawable.cpp,v $
   Revision 1.1.1.1  2002/06/23 05:42:09  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 ***********************************************************************/

#define TESTPSDRAWABLE_CPP

#include "PSDrawable.h"

using namespace sg;

int main()
{
  PSDrawable psd;
  
  psd.drawLine(0,0, 100,100);

  psd.print();

  return 0;
}
