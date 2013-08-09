#pragma once
#include "pluginbase.h"
#include <string>

class Plugin : public nsPluginInstanceBase
{
public:
    NPP m_pNPInstance;
    NPBool m_bInitialized;
    HWND m_hWnd;
    NPWindow* mWindow;
    NPObject* m_pScriptableObject;
    NPObject* m_jsObj;

public:
    Plugin(NPP pNPInstance);
    ~Plugin();

    NPBool init(NPWindow* pNPWindow);
    void shut();
    NPBool isInitialized();

    virtual NPObject *GetScriptableObject();
};

class CScriptObject : public nsScriptObjectBase
{
private:
    int m_vfoo;
    std::string m_str;

public:
    static NPIdentifier bar_id;
    static NPIdentifier func_id;
    static NPObject *sWindowObj;
    static NPIdentifier foo_id;
    static NPIdentifier str_id;
    static NPIdentifier i2ifunc_id;
    static NPIdentifier s2sfunc_id;
    static NPIdentifier jsfunc_id;

    CScriptObject(NPP npp):nsScriptObjectBase(npp),m_vfoo(0),m_str(""){}
    ~CScriptObject(){};

    virtual bool HasProperty(NPIdentifier name);
    virtual bool HasMethod(NPIdentifier name);
    virtual bool GetProperty(NPIdentifier name, NPVariant *result);
    virtual bool SetProperty(NPIdentifier name, const NPVariant *value);
    virtual bool Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
    virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result);
};
