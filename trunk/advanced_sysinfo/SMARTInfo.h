// SMARTInfo.h: CSMARTInfo クラスのインターフェイス

#if !defined(AFX_SMARTINFO_H__9EED1D90_5944_4B4D_AE7D_1EA1A06D793B__INCLUDED_)
#define AFX_SMARTINFO_H__9EED1D90_5944_4B4D_AE7D_1EA1A06D793B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSMARTInfo  
{
private:

public:
	void Release(void);
	bool Init(void);
	CSMARTInfo();
	virtual ~CSMARTInfo();

};

#endif // !defined(AFX_SMARTINFO_H__9EED1D90_5944_4B4D_AE7D_1EA1A06D793B__INCLUDED_)
