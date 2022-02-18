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

#include "FSignalUtils.h"
#include "FieldDataWrapper.h"
#include "PciEpnParameter.h"
#include "PciEpnNode.h"
#include "TimeStampWrapper.h"
#include <dirent.h>

using namespace std;


using namespace fsignal;

int main(int argc, char *argv[])
{
    if(argc < 2){
            cout << "Please provide the location of the properties and log file" << endl;		
            return -1;
    }
        
    new PciEpnNode(argv[1], L"0x0000",  argv[2]);
    while(1){
            sleep(1);
    }

    cout << "Exiting" << endl;
    return EXIT_SUCCESS;
}
