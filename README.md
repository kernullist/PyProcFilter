# PyProcFilter
Process Filter for Python

# What
프로세스 실행 감시 및 차단을 간단하게 할 수 있도록 해주는 파이썬 도우미

# Target Environment
- Windows (x86 / x64)
- Python 3.5

# Development Environment
- Visual Studio 2015
- WDK 10586
- Python 3.5

# Demo
[![Demo Video](http://img.youtube.com/vi/6GtyZO-OHY8/0.jpg)](http://www.youtube.com/watch?v=6GtyZO-OHY8)


# Example
```python
import sys
from  pyprocfilterhelper import *

# 프로세스 실행시 호출되는 콜백
# True 리턴시 실행 허용
# False 리턴시 실행 차단
def process_callback(parentPid, processPid, processPath):
    print("{:d} ==> {:d} path : {:s}".format(parentPid, processPid, processPath))

    # 메모장 실행 차단!
    if str(processPath).lower().find("c:\\windows\\system32\\notepad.exe") != -1:
        print("Block!!!")
        return False

    return True


if __name__ == '__main__':

    pf = pyprocfilterhelper()

    # 필터 설치
    if pf.install() is False:
        print("install error")
        sys.exit(0)

    # 필터링 시작
    ret = pf.start(process_callback)
    if ret is True:
        print("Process Filtering Started...");
        try:
            while True:
                pass
        except KeyboardInterrupt:
            # 필터링 중지
            pf.stop()
            print("Process Filtering Stoped...");
    else:
        print("Error pyprocfilter()")

    # 필터 제거
    pf.uninstall()
```