'''
Created on 2016. 1.18.

@author: kernullist.gloryo
'''

import os
import subprocess
import shutil
import shlex
import pyprocfilter


# status
ERROR_SUCCESS                   = 0
ERROR_SERVICE_ALREADY_RUNNING   = 1056
ERROR_SERVICE_DOES_NOT_EXIST    = 1060
ERROR_SERVICE_NOT_ACTIVE        = 1062
ERROR_SERVICE_EXISTS            = 1073


# pyprocfilterhelper
class pyprocfilterhelper(object):
    def __init__(self):
        self.sysname = "knprocfilter"
        self.windows_path = os.environ.get("windir")
        self.drivers_path = os.path.join(self.windows_path, "system32\\drivers")
        self.knprocfilter_path = os.path.join(self.drivers_path, "knprocfilter.sys")
        self.kncomm_path = os.path.join(self.drivers_path, "kncomm.sys")

    def __del__(self):
        self.__stop_service(self.sysname)

    def print_paths(self):
        print("windows_path : ", self.windows_path)
        print("drivers_path : ", self.drivers_path)
        print("knprocfilter_path : ", self.knprocfilter_path)
        print("kncomm_path : ", self.kncomm_path)

    def __copy_sys_to_drivers(self):
        shutil.copy2("knprocfilter.sys", self.drivers_path)
        shutil.copy2("kncomm.sys", self.drivers_path)

    def __delete_sys_from_drivers(self):
        os.remove(self.knprocfilter_path)
        os.remove(self.kncomm_path)

    def __execute(self, cmd):
        sc = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE)
        sc.communicate()
        rc = sc.wait()
        return rc

    def __create_service(self, svcname, binpath):
        cmd = "sc create {:s} binPath= '{:s}' type= kernel start= demand displayname= {:s}".format(svcname, binpath, svcname)
        rc = self.__execute(cmd)
        if rc == ERROR_SUCCESS or rc == ERROR_SERVICE_EXISTS:
            return True
        return False

    def __delete_service(self, svcname):
        cmd = "sc delete {:s}".format(svcname)
        rc = self.__execute(cmd)
        if rc == ERROR_SUCCESS or rc == ERROR_SERVICE_DOES_NOT_EXIST:
            return True
        return False

    def __start_service(self, svcname):
        cmd = "sc start {:s}".format(svcname)
        rc = self.__execute(cmd)
        if rc == ERROR_SUCCESS or rc == ERROR_SERVICE_ALREADY_RUNNING:
            return True
        return False

    def __stop_service(self, svcname):
        cmd = "sc stop {:s}".format(svcname)
        rc = self.__execute(cmd)
        if rc == ERROR_SUCCESS or rc == ERROR_SERVICE_NOT_ACTIVE:
            return True
        return False

    def install(self):
        self.__copy_sys_to_drivers()
        ret = self.__create_service(self.sysname, self.knprocfilter_path)
        if ret is not True:
            return False
        ret = self.__start_service(self.sysname)
        if ret is not True:
            return False
        return True

    def uninstall(self):
        self.__stop_service(self.sysname)
        self.__delete_service(self.sysname)
        self.__delete_sys_from_drivers()

    def start(self, process_callback):
        return pyprocfilter.start(process_callback)

    def stop(self):
        return pyprocfilter.stop()
