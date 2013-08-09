#include "Plugin.h"
#include <windowsx.h>

static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
static WNDPROC lpOldProc = NULL;

NPError NS_PluginInitialize()
{
    return NPERR_NO_ERROR;
}

void NS_PluginShutdown()
{
}

nsPluginInstanceBase * NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
    if(!aCreateDataStruct)
    {
        return NULL;
    }

    Plugin * plugin = new Plugin(aCreateDataStruct->instance);

    BOOL bWindowed = TRUE;
    NPN_SetValue(aCreateDataStruct->instance, NPPVpluginWindowBool, (void *)bWindowed);//winless插件需要在这里指出,默认为windowed

    return plugin;
}

void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
    if(aPlugin)
    {
        delete (Plugin *)aPlugin;
    }
}

/// implementation for plugin
Plugin::Plugin(NPP pNPInstance):nsPluginInstanceBase(),
m_pNPInstance(pNPInstance),
m_bInitialized(FALSE)
{
    mWindow = NULL;
    m_hWnd = NULL;
}

Plugin::~Plugin(void)
{
}

NPBool Plugin::init(NPWindow* pNPWindow)
{
    mWindow=pNPWindow;
    m_hWnd =(HWND)pNPWindow->window;
    if (m_hWnd==NULL)
    {
        return FALSE;
    }

    // 对窗口进行子类化，这样就可以对消息进行处理并在窗口中进行绘制
    lpOldProc = SubclassWindow(m_hWnd, (WNDPROC)PluginWinProc);

    // 将窗口与 Plugin 对象相关联，这样就可以在窗口处理中访问Plugin 对象
    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

    //MessageBox(m_hWnd, "hi", "test", MB_OK);
    m_pScriptableObject=NULL;
    m_jsObj = NULL;
    CScriptObject::bar_id = NPN_GetStringIdentifier("bar");
    CScriptObject::func_id = NPN_GetStringIdentifier("func");
    CScriptObject::foo_id = NPN_GetStringIdentifier("foo");
    CScriptObject::str_id = NPN_GetStringIdentifier("str");
    CScriptObject::i2ifunc_id = NPN_GetStringIdentifier("funci2i");
    CScriptObject::s2sfunc_id = NPN_GetStringIdentifier("funcs2s");
    CScriptObject::jsfunc_id = NPN_GetStringIdentifier("OnJsFunc");
    NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, &(CScriptObject::sWindowObj));

    m_bInitialized = TRUE;
    return TRUE;
}

void Plugin::shut() 
{ 
    // 反子类化subclass it back
    SubclassWindow(m_hWnd, lpOldProc);

    if (m_jsObj)
    {
        NPN_ReleaseObject(m_jsObj);
    }

    if (m_pScriptableObject)
    {
        NPN_ReleaseObject(m_pScriptableObject);
    }

    m_hWnd = NULL;
    m_bInitialized = FALSE;
}

NPBool Plugin::isInitialized() 
{ 
    return m_bInitialized; 
}

static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) 
    {
    case WM_PAINT:
        {
            // draw a frame and display some string
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rc;
            GetClientRect(hWnd, &rc);
            FillRect( hdc, &rc, (HBRUSH) (COLOR_WINDOW));
            FrameRect(hdc, &rc, GetStockBrush(BLACK_BRUSH));
            Plugin * p = (Plugin*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if(p) 
            {
                char *s = "Hello , MY FIRST PLUGIN!";//p->GetGuiText();
                DrawText(hdc, s, strlen(s), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            }

            EndPaint(hWnd, &ps);
        }
        break;
    default:
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

NPObject* Plugin::GetScriptableObject()
{
    if (!m_pScriptableObject) 
    {
        m_pScriptableObject = NPN_CreateObject(m_pNPInstance,	&CScriptObject::nsScriptObjectClass);
    }

    if (m_pScriptableObject) 
    {
        NPN_RetainObject(m_pScriptableObject);
    }

    return m_pScriptableObject;
}


/// implementation for nsScriptObjectBase
NPIdentifier CScriptObject::bar_id;
NPIdentifier CScriptObject::func_id;
NPIdentifier CScriptObject::foo_id;
NPIdentifier CScriptObject::str_id;
NPIdentifier CScriptObject::i2ifunc_id;
NPIdentifier CScriptObject::s2sfunc_id;
NPIdentifier CScriptObject::jsfunc_id;
NPObject * CScriptObject::sWindowObj;

NPObject *nsScriptObjectBase::AllocateScriptPluginObject(NPP npp, NPClass *aClass)
{
    return (NPObject*)new CScriptObject(npp);
}

bool CScriptObject::HasMethod(NPIdentifier name)
{
    return name == func_id || name == i2ifunc_id || name == s2sfunc_id;
}

bool CScriptObject::HasProperty(NPIdentifier name)
{
    return name == bar_id || name == foo_id || name == str_id
        || name == jsfunc_id;
}

bool CScriptObject::GetProperty(NPIdentifier name, NPVariant *result)
{
    if (name == foo_id)
    {
        INT32_TO_NPVARIANT(m_vfoo,*result);
        m_vfoo++;
        return true;
    }
    else if (name == str_id)
    {
        if(m_str=="") 
        {
            VOID_TO_NPVARIANT(*result); 
            m_str ="s"; 
            return true;
        }
        char* npOutString = (char*) NPN_MemAlloc(m_str.length() + 1);
        if (!npOutString)
        {
            return false;
        }
        strcpy(npOutString, m_str.c_str());
        STRINGZ_TO_NPVARIANT(npOutString,*result);
        m_str += "s";
        return true;
    }
    return NPN_GetProperty(m_npp,sWindowObj,name,result);
}

bool CScriptObject::SetProperty(NPIdentifier name, const NPVariant *value)
{
    if(name==foo_id)
    {
        if(value->type == NPVariantType_Int32)
        {
            m_vfoo = NPVARIANT_TO_INT32(*value);
        }
        //else m_vfoo = boost::lexical_cast<int>(value);
        return true;
    }
    else if(name == str_id)
    {
        if (value->type == NPVariantType_String) m_str = std::string(value->value.stringValue.UTF8Characters);
        //else if(value->type == NPVariantType_Int32) m_str = boost::lexical_cast<std::string>(value->value.intValue);
        //else if(value->type == NPVariantType_Double) m_str = boost::lexical_cast<std::string>(value->value.doubleValue);
        //else if(value->type == NPVariantType_Bool) m_str == boost::lexical_cast<std::string>(value->value.boolValue);
        else m_str = "";
        return true;
    }
    if (name == jsfunc_id)
    {
        Plugin * plugin = (Plugin*) m_npp->pdata;
        if (plugin->m_jsObj == NULL)
        {
            plugin->m_jsObj = NPN_RetainObject(NPVARIANT_TO_OBJECT(*value));
        }
        return true;
    }
    return NPN_SetProperty(m_npp,sWindowObj,name,value);
}

bool CScriptObject::Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    if (name == func_id)
    {
        Plugin* plugin = (Plugin*) m_npp->pdata;
        if (!(!plugin->m_jsObj))
        {
            NPVariant result;
            NPN_InvokeDefault(m_npp,plugin->m_jsObj, NULL, 0, &result);
            NPN_ReleaseVariantValue(&result);
        }
        return true;
    }
    if (name == i2ifunc_id)
    {
        int t = 0;
        if(args[0].type == NPVariantType_Int32)
            t = NPVARIANT_TO_INT32(args[0]);
        //else t = boost::lexical_cast<int>(&args[0]);
        INT32_TO_NPVARIANT(-t, *result);
        return true;
    }
    if (name == s2sfunc_id)
    {
        std::string firstr = "";
        if (args[0].type == NPVariantType_String)
        {
            firstr = std::string(args[0].value.stringValue.UTF8Characters);
        }
        std::string tmpstr = "processed string:"+ firstr;// 
        char* npOutString = (char*) NPN_MemAlloc(tmpstr.length() + 1);
        if (!npOutString)
        {
            return false;
        }
        strcpy(npOutString, tmpstr.c_str());
        STRINGZ_TO_NPVARIANT(npOutString,*result);
        return true;
    }
    return false;
}

bool CScriptObject::InvokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    return true;
}
