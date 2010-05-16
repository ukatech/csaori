#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

#include <string>
#include <vector>
#include <map>

#include <ole2.h>
#include <MLang.h>

#include <stdio.h>

using namespace std;

int main( int argc, char * argv[] )
{
	if(argc>1) {
		vector<char> theData;

		FILE *	pFile;
		size_t	result;
		INT		lSize;

		pFile = fopen(argv[1], "rb");
		if (pFile==NULL) {
			printf("fopen failed");
			return 1;
		}

		// obtain file size:
		fseek(pFile, 0, SEEK_END);
		lSize = ftell(pFile);
		rewind(pFile);

		// allocate memory to contain the whole file:
		theData.resize(lSize);

		// copy the file into the buffer:
		result = fread(&theData[0],1,lSize,pFile);
		if (result != lSize) {
			printf("fread failed");
			return 1;
		}

		/* the whole file is now loaded in the memory buffer. */
		fclose(pFile);	

		//IMultiLanguage生成
		CoInitialize( NULL );
		IMultiLanguage2 *lang = NULL;
		HRESULT hr = CoCreateInstance(CLSID_CMultiLanguage, NULL,
			CLSCTX_ALL, IID_IMultiLanguage2, (LPVOID*)&lang);

		DetectEncodingInfo lpInfo[15];
		int detectEncCount = 15;
		if (SUCCEEDED(hr))
			hr = lang->DetectInputCodepage(MLDETECTCP_DBCS, 0, &theData[0], &lSize, lpInfo, &detectEncCount);

		char sMsg[2048],sTmp[256];
		sprintf(sMsg,"filesize = %d, ret = %X\n",lSize,hr);

		if (FAILED(hr))
		{
			LPSTR lpBuffer;
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, GetLastError(), LANG_NEUTRAL, (LPTSTR) &lpBuffer, 0, NULL );
			strcat(sMsg,lpBuffer);
			LocalFree(lpBuffer);
		}
		else
			strcat(sMsg,"----\n");
		for(int x=0;x<detectEncCount;x++) {
			sprintf(sTmp,"%d Codepage: %d, DocPercent = %d%%, Confidence = %d\n",x,lpInfo[x].nCodePage,lpInfo[x].nDocPercent,lpInfo[x].nConfidence);
			strcat(sMsg,sTmp);
		}
//		printf(sMsg);
		MessageBox(NULL,sMsg,"MLang Detection", MB_OK);

	}
}
