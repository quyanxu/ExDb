
// CExdbView.cpp : CExdbView 클래스의 구현
//

#include "stdafx.h"
#include "ExDB.h"

#include "CExdbDoc.h"
#include "CExdbView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//extern CLogProc cLog;

// CExdbView

IMPLEMENT_DYNCREATE(CExdbView, CView)

BEGIN_MESSAGE_MAP(CExdbView, CView)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CExdbView 생성/소멸

CExdbView::CExdbView()
{
	// TODO: 여기에 생성 코드를 추가합니다.
        // Create font for configuration info display
    m_ConfigInfoFont.CreateFont(
        15,                         // nHeight
        0,                          // nWidth (0 = auto)
        0,                          // nEscapement
        0,                          // nOrientation
        FW_BOLD,                    // nWeight (700 = bold)
        FALSE,                      // bItalic
        FALSE,                      // bUnderline
        FALSE,                      // cStrikeOut
        ANSI_CHARSET,               // nCharSet (0 = ANSI)
        OUT_DEVICE_PRECIS,          // nOutPrecision (1)
        CLIP_DEFAULT_PRECIS,        // nClipPrecision (2)
        PROOF_QUALITY,              // nQuality (1)
        DEFAULT_PITCH,              // nPitchAndFamily (0)
        "MS San Serif");            // lpszFacename

    // Set configuration info rectangle
    m_ConfigInfoRect.left = 10;
    m_ConfigInfoRect.top = 10;
    m_ConfigInfoRect.right = 600;
    m_ConfigInfoRect.bottom = 50;
}

CExdbView::~CExdbView()
{
}

BOOL CExdbView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CExdbView 그리기

void CExdbView::OnDraw(CDC* /*pDC*/)
{
	CExdbDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
}
// OnCreate handler - called when the view is created
int CExdbView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    // Call base class first
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Create the log system
    cLog.Create(1);  // Parameter likely indicates log level or type

    return 0;
}

// OnPaint handler - called when the view needs to be repainted
void CExdbView::OnPaint()
{
    CPaintDC dc(this);  // Device context for painting

    // Draw configuration information
    DrawConfigInfo(&dc);

    // Print log information
    cLog.Print(m_hWnd, 0, 50);
}

void CExdbView::DrawConfigInfo(CDC* pDC)
{
#ifdef _CUSTOM_INTERFACE
    if (!pDC) return;

    // Save current DC state
    int nOldBkMode = pDC->SetBkMode(TRANSPARENT);
    CFont* pOldFont = pDC->SelectObject(&m_logFont);

    // Draw server information
    CString strText;
    strText.Format(_T("ExDB Server Status"));
    pDC->TextOut(10, 10, strText);
    strText.Format(_T("=================="));
    pDC->TextOut(10, 30, strText);

    // Add more configuration info here as needed
    // For example:
    // - Database connection status
    // - Chat server IP/Port
    // - Current language
    // - Loaded data counts

    // Draw a separator line
    CPen pen(PS_SOLID, 1, RGB(0, 0, 255));
    CPen* pOldPen = pDC->SelectObject(&pen);
    pDC->MoveTo(10, 45);
    pDC->LineTo(400, 45);

    // Restore DC state
    pDC->SelectObject(pOldPen);
    pDC->SelectObject(pOldFont);
    pDC->SetBkMode(nOldBkMode);
#else
    CString szConfigInfo;
    BOOL bDraw = FALSE;

    szConfigInfo = "Configuration Info : ";

    if (!g_bEnableFriend)
    {
        bDraw = TRUE;
        szConfigInfo += "[ Friend OFF ]";
    }

    if (!g_bEnableFriendMail)
    {
        bDraw = TRUE;
        szConfigInfo += "[ Mail OFF ]";
    }

    if (bDraw)
    {
        CFont* pOldFont = pDC->SelectObject(&m_ConfigInfoFont);
        pDC->SetBkMode(TRANSPARENT);           // 1 = TRANSPARENT
        pDC->SetTextColor(RGB(255, 0, 0));     // 255 = red (0x000000FF)
        pDC->DrawText(szConfigInfo, &m_ConfigInfoRect, DT_SINGLELINE | DT_VCENTER);  // 0x20

        pDC->SelectObject(pOldFont);
    }
#endif
}

void CExdbView::OnRButtonUp(UINT nFlags, CPoint point)
{

}

void CExdbView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	
}


// CExdbView 진단

#ifdef _DEBUG
void CExdbView::AssertValid() const
{
	CView::AssertValid();
}

void CExdbView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CExdbDoc* CExdbView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CExdbDoc)));
	return (CExdbDoc*)m_pDocument;
}
#endif //_DEBUG


// CExdbView 메시지 처리기
