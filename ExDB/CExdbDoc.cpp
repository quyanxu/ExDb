
// CExdbDoc.cppMainFrm.cppCExdbDoc.cpp : CExdbDoc 클래스의 구현
//

#include "stdafx.h"
#include "ExDB.h"

#include "CExdbDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CExdbDoc

IMPLEMENT_DYNCREATE(CExdbDoc, CDocument)

BEGIN_MESSAGE_MAP(CExdbDoc, CDocument)
END_MESSAGE_MAP()


// CExdbDoc 생성/소멸

CExdbDoc::CExdbDoc()
{
	// TODO: 여기에 일회성 생성 코드를 추가합니다.

}

CExdbDoc::~CExdbDoc()
{
}

BOOL CExdbDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 여기에 재초기화 코드를 추가합니다.
	// SDI 문서는 이 문서를 다시 사용합니다.

	return TRUE;
}




// CExdbDoc serialization

void CExdbDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 여기에 저장 코드를 추가합니다.
	}
	else
	{
		// TODO: 여기에 로딩 코드를 추가합니다.
	}
}


// CExdbDoc 진단

#ifdef _DEBUG
void CExdbDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CExdbDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CExdbDoc 명령
