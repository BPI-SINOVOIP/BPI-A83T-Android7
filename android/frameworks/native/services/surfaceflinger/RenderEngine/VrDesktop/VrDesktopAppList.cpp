#include "VrDesktopAppList.h"

VrDesktopAppList::VrDesktopAppList()
:   mAutoLock(){
    mVrAppList.add(String8("com.softwinner.VrVideoPlayer"));
    mVrAppList.add(String8("com.softwinner.VrLaunch"));
    mVrAppList.add(String8("CustomDialog"));
    mVrAppList.add(String8("com.softwinner.ATWTool"));
    mVrAppList.add(String8("com.softwinner.VrStore"));
    mVrAppList.add(String8("com.softwinner.spherevideo"));

    mVrFixAppList.add(String8("com.softwinner.VrVideoPlayer"));
    mVrFixAppList.add(String8("com.softwinner.VrLaunch"));
    mVrFixAppList.add(String8("CustomDialog"));
    mVrFixAppList.add(String8("com.softwinner.ATWTool"));
    mVrFixAppList.add(String8("com.softwinner.VrStore"));
    mVrFixAppList.add(String8("com.softwinner.spherevideo"));
    mVrFixAppList.add(String8("com.allwinner.startupGuide"));
    mVrFixAppList.add(String8("com.softwinner.VrModeSelector"));
    mVrFixAppList.add(String8("com.android.settings"));
    mVrFixAppList.add(String8("com.estrongs.android.pop"));
    mVrFixAppList.add(String8("com.android.chrome"));
    mVrFixAppList.add(String8("com.google.android.apps.photos"));
}

VrDesktopAppList::~VrDesktopAppList(){

}

void VrDesktopAppList::addVrApp(String8 app){
    Mutex::Autolock lock(mAutoLock);
    for(size_t i = 0; i < mVrAppList.size(); i++){
        if(app.find(mVrAppList[i].string()) > -1){
            ALOGD("%s has already add in vr app list", app.string());
            return;
        }
    }
    mVrAppList.add(app);
}

void VrDesktopAppList::addTempVrApp(String8 app){
    Mutex::Autolock lock(mAutoLock);
    for(size_t i = 0; i < mVrTempAppList.size(); i++){
        if(app.find(mVrTempAppList[i].string()) > -1){
            ALOGD("%s has already add in vr app temp list", app.string());
            return;
        }
    }
    mVrTempAppList.add(app);
}

void VrDesktopAppList::deleteTempVrApp(String8 app)
{
    Mutex::Autolock lock(mAutoLock);
    for(size_t i = 0; i < mVrTempAppList.size(); i++) {
        if(app.find(mVrTempAppList[i].string()) > -1) {
            ALOGV("deleteTempVrApp: find vr app %s in list", app.string());
            mVrTempAppList.removeAt(i);
        }
    }
}

bool VrDesktopAppList::isVrApp(String8 app){
    Mutex::Autolock lock(mAutoLock);
    for(size_t i = 0; i < mVrAppList.size(); i++){
        if(app.find(mVrAppList[i].string()) > -1){
            ALOGV("IS VR APP: find vr app %s in list", app.string());
            return true;
        }
    }

    for(size_t i = 0; i < mVrTempAppList.size(); i++){
        if(app.find(mVrTempAppList[i].string()) > -1){
            ALOGV("IS VR APP: find vr app %s in temp list", app.string());
            return true;
        }
    }
    return false;
}

bool VrDesktopAppList::isFixedApp(String8 app){
    Mutex::Autolock lock(mAutoLock);
    for(size_t i = 0; i < mVrFixAppList.size(); i++){
        if(app.find(mVrFixAppList[i].string()) > -1){
            ALOGV("IS FIX APP: find vr app %s in list", app.string());
            return true;
        }
    }
    return false;
}

