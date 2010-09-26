#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <comdef.h>
#include <shellapi.h>
#include <mbstring.h>

#include "csaori.h"
#include "util/cs_threadcall.h"

#import "SubWCRevCOM.exe" named_guids

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

#ifndef IID_PPV_ARGS
extern "C++" {
    template<typename T> void** IID_PPV_ARGS_Helper(T** pp) {
        static_cast<IUnknown*>(*pp);    // make sure everyone derives from IUnknown
        return reinterpret_cast<void**>(pp);
    }
}

#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType)
#endif //IID_PPV_ARGS

//Class Definitions
class CWMISAORI : public CSAORI, public CSThreadCallBase {
private:

protected:
	virtual void CSTC_Start(void);
	virtual void CSTC_Exit(void);
	virtual void CSTC_Call(void *p1,void *p2);

public:
	CWMISAORI();
	virtual ~CWMISAORI();

	//à»â∫Ç™é¿ëïÇ∑Ç◊Ç´ä÷êî
	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();
};

//CreateInstance
CSAORIBase* CreateInstance(void)
{
	return new CWMISAORI();
}

CWMISAORI::CWMISAORI(void)
{
	;
}

CWMISAORI::~CWMISAORI()
{
	;
}

bool CWMISAORI::load()
{
	;
}

bool CWMISAORI::unload()
{
	;
}

void CWMISAORI::exec(const CSAORIInput &in, CSAORIOutput &out)
{
	;
}

/*-----------------------------------------------------
	COMó·äOìfÇ´ëŒçÙ
------------------------------------------------------*/
void CWMISAORI::CSTC_Start(void)
{
	;
}
void CWMISAORI::CSTC_Exit(void)
{
	;
}
void CWMISAORI::CSTC_Call(void *p1,void *p2)
{
	;
}
