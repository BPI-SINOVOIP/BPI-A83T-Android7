#ifndef _VRDESKTOP_APPLIST
#define _VRDESKTOP_APPLIST

#include <utils/Vector.h>
#include <utils/String8.h>
#include <utils/Mutex.h>

using namespace android;

//class String8;

class VrDesktopAppList{
public:
        VrDesktopAppList();
        ~VrDesktopAppList();
        void addVrApp(String8 appName);
        void addTempVrApp(String8 appName);
        void deleteTempVrApp(String8 appName);
        bool isVrApp(String8 appName);
        bool isFixedApp(String8 appName);
private:
        Vector<String8> mVrAppList;
        Vector<String8> mVrTempAppList;
        Vector<String8> mVrFixAppList;
        mutable Mutex mAutoLock;
};

#endif
