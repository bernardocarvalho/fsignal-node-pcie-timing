/***************************************************************************
 *   Copyright (C) 2006 by Andre Neto   *
 *   andre.neto@cfn.ist.utl.pt   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <ctime>
#include <pthread.h>
#include "xml/HardwareXmlHandler.h"
#include "xml/HardwareXml.h"
#include "xml/NodeXmlHandler.h"
#include "xml/XmlParser.h"
#include "xml/NodeXml.h"
#include "xml/PersistParameterHandler.h"
#include "FSNode.h"
#include "FSParameter.h"
#include "FSHardware.h"
#include "Properties.h"
#include "utils/FSignalUtils.h"
#include "utils/FieldDataWrapper.h"
//#include "test/TestParameter.h"
//#include "test/TestNode.h"
#include "tr512/Tr512Parameter.h"
#include "tr512/Tr512Node.h"
#include "utils/TimeStampWrapper.h"
#include <dirent.h>

using namespace std;
using namespace fsignal;

void *fileChecker(void* args)
{
	while(1)
	{
		fstream fin;
  		fin.open("/home/bernardo/exit", ios::in);
  		if( fin.is_open() )
  		{
    			fin.close();
			cout << "File found... exiting" << endl;
    			exit(1);
  		}
		fin.close();
		cout << "Going to sleep" << endl;
		sleep(5);		
		cout << "Waked" << endl;  		
	}
}

int main(int argc, char *argv[])
{
	Tr512Node* node = new Tr512Node("tr512/fsignal.props", L"0x0000");
	char c;
	cin >> c;	
       
	return EXIT_SUCCESS;
}
