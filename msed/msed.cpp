/* C:B**************************************************************************
This software is Copyright 2014,2015 Michael Romeo <r0m30@r0m30.com>

This file is part of msed.

msed is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

msed is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with msed.  If not, see <http://www.gnu.org/licenses/>.

* C:E********************************************************************** */
#include <iostream>
#include "os.h"
#include "MsedHashPwd.h"
#include "MsedOptions.h"
#include "MsedLexicon.h"
#include "MsedDev.h"

using namespace std;

int diskScan()
{
	char devname[25];
	int i = 0;
	uint8_t FirmwareRev[8];
	uint8_t ModelNum[40];
	MsedDev * d;
	LOG(D1) << "Creating diskList";
	printf("\nScanning for Opal compliant disks\n");
	while (TRUE) {
		DEVICEMASK;
		//snprintf(devname,23,"/dev/sd%c",(char) 0x61+i) Linux
		//sprintf_s(devname, 23, "\\\\.\\PhysicalDrive%i", i)  Windows
		d = new MsedDev(devname);
		if (d->isPresent()) {
			d->getFirmwareRev(FirmwareRev);
			d->getModelNum(ModelNum);
			printf("%s", devname);
			if (d->isAnySSC())
				printf(" %s%s%s ", (d->isOpal1() ? "1" : " "),
				(d->isOpal2() ? "2" : " "), (d->isEprise() ? "E" : " "));
			else
				printf("%s", " No  ");

			for (int x = 0; x < sizeof(ModelNum); x++) {
				cout << ModelNum[x];
			}
			cout << " ";
			for (int x = 0; x < sizeof(FirmwareRev); x++) {
				//if (0x20 == FirmwareRev[x]) break;
				cout << FirmwareRev[x];
			}
			cout << std::endl;
			if (MAX_DISKS == i) {
				LOG(I) << MAX_DISKS << " disks, really?";
				delete d;
				return 1;
			}
		}
		else break;
		delete d;
		i += 1;
	}
	delete d;
	printf("No more disks present ending scan\n");
	return 0;
}
int main(int argc, char * argv[])
{
	MSED_OPTIONS opts;
	MsedDev * d =NULL;
//	vector<uint8_t> opalTRUE(1, 0x01), opalFALSE(1, 0x00);

	if (MsedOptions(argc, argv, &opts)) {
		//LOG(E) << "Invalid command line options ";
		return 1;
	}
	
	if ((opts.action != msedoption::scan) && (opts.action != msedoption::validatePBKDF2)) {
		if (opts.device > (argc - 1)) opts.device = 0;
		d = new MsedDev(argv[opts.device]);
		if ((!d->isPresent()) || (!d->isAnySSC())) {
			LOG(E) << "Invalid or unsupported disk " << argv[opts.device];
			return 2;
		}
	}
    switch (opts.action) {
 
	case msedoption::initialsetup:
        if (0 == opts.password) {
            LOG(E) << "Initial setup requires a new SID password";
            return 1;
        }
		LOG(D) << "Performing initial setup to use msed on drive " << argv[opts.device];
        return (d->initialsetup(argv[opts.password]));
	case msedoption::setSIDPwd:
        if ((0 == opts.password) || (0 == opts.newpassword)) {
            LOG(E) << "setSIDPwd requires both the old SID password and a new SID password";
            return 1;
        }
        LOG(D) << "Performing setSIDPwd ";
        return d->setSIDPassword(argv[opts.password], argv[opts.newpassword]);
		break;
	case msedoption::setAdmin1Pwd:
        LOG(D) << "Performing setPAdmin1Pwd ";
        return d->setNewPassword(argv[opts.password], (char *) "Admin1",
                            argv[opts.newpassword]);
		break;
	case msedoption::loadPBAimage:
        LOG(D) << "Loading PBA image " << argv[opts.pbafile] << " to " << opts.device;
        return d->loadPBA(argv[opts.password], argv[opts.pbafile]);
		break;
	case msedoption::setLockingRange:
        if (0 == opts.password) {
            LOG(E) << "setLockingRange requires the Admin1 password";
            return 1;
        }
        LOG(D) << "Setting Locking Range " << (uint16_t) opts.lockingrange << " " << (uint16_t) opts.lockingstate;
        return d->setLockingRange(opts.lockingrange, opts.lockingstate, argv[opts.password]);
		break;
	case msedoption::enableLockingRange:
        if (0 == opts.password) {
            LOG(E) << "Enabling a Locking range " <<
                    "requires the Admin1 password";
            return 1;
        }
        LOG(D) << "Enabling Locking Range " << (uint16_t) opts.lockingrange;
        return (d->configureLockingRange(opts.lockingrange,OPAL_TOKEN::OPAL_TRUE,
                           argv[opts.password]));
        break;
	case msedoption::disableLockingRange:
		if (0 == opts.password) {
			LOG(E) << "Disabling a Locking range " <<
				"requires the Admin1 password ";
			return 1;
		}
		LOG(D) << "Disabling Locking Range " << (uint16_t) opts.lockingrange;
		return (d->configureLockingRange(opts.lockingrange, OPAL_TOKEN::OPAL_FALSE,
			argv[opts.password]));
		break;
	case msedoption::setMBRDone:
		if (0 == opts.password) {
			LOG(E) << "Setting MBRDone " <<
				"requires the Admin1 password ";
			return 1;
		}
		LOG(D) << "Setting MBRDone " << (uint16_t)opts.mbrstate;
		return (d->setMBRDone(opts.mbrstate, argv[opts.password]));
		break;
	case msedoption::setMBREnable:
		if (0 == opts.password) {
			LOG(E) << "Setting MBREnable " <<
				"requires the Admin1 password ";
			return 1;
		}
		LOG(D) << "Setting MBREnable " << (uint16_t)opts.mbrstate;
		return (d->setMBREnable(opts.mbrstate, argv[opts.password]));
		break;
	case msedoption::enableuser:
        LOG(D) << "Performing enable user for user " << argv[opts.userid];
        return d->enableUser(argv[opts.password], argv[opts.userid]);
        break;
	case msedoption::activateLockingSP:
        if (0 == opts.password) {
            LOG(E) << "Activating the Locking SP required the SID password ";
            return 1;
        }
		LOG(D) << "Activating the LockingSP on" << argv[opts.device];
        return d->activateLockingSP(argv[opts.password]);
        break;
    case msedoption::query:
		LOG(D) << "Performing diskquery() on " << argv[opts.device];
        d->diskQuery();
        return 0;
        break;
	case msedoption::scan:
        LOG(D) << "Performing diskScan() ";
        diskScan();
        break;
	case msedoption::takeownership:
        if (0 == opts.password) {
            LOG(E) << "Taking ownership requires a *NEW* SID password ";
            return 1;
        }
		LOG(D) << "Taking Ownership of the drive at" << argv[opts.device];
        return d->takeOwnership(argv[opts.password]);
        break;
 //  case msedoption::dumptable:
 //       if (0 == opts.password) {
 //           LOG(E) << "tableDump requires a password";
 //           return 1;
 //       }
 //       LOG(D) << "Performing dumpTable() ";
 //       return dumpTable(argv[opts.password], argv[opts.device]);
 //       break;
	case msedoption::revertLockingSP:
        if (0 == opts.password) {
            LOG(E) << "Reverting the Locking SP requires a password ";
            return 1;
        }
		LOG(D) << "Performing revertLockingSP on " << argv[opts.device];
        return d->revertLockingSP(argv[opts.password]);
        break;
	case msedoption::setPassword:
        LOG(D) << "Performing setPassword for user " << argv[opts.userid];
        return d->setNewPassword(argv[opts.password], argv[opts.userid],
                              argv[opts.newpassword]);
        break;
	case msedoption::reverttper:
        if (0 == opts.password) {
            LOG(E) << "Reverting the TPer requires a the SID password ";
            return 1;
        }
		LOG(D) << "Performing revertTPer on " << argv[opts.device];
        return d->revertTPer(argv[opts.password]);
        break;
	case msedoption::validatePBKDF2:
        LOG(D) << "Performing PBKDF2 validation ";
        MsedTestPBDKF2();
        break;
	case msedoption::yesIreallywanttoERASEALLmydatausingthePSID:
	case msedoption::PSIDrevert:
        if (0 == opts.password) {
            LOG(E) << "PSID Revert requires a password ";
            return 1;
        }
		LOG(D) << "Performing a PSID Revert on " << argv[opts.device] << " with password " << argv[opts.password];
        return d->revertTPer(argv[opts.password], 1);
        break;
    default:
        LOG(E) << "Unable to determine what you want to do ";
        usage();
    }
    return 1;
}