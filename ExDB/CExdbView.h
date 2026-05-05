
// CExdbView.h : CExdbView 클래스의 인터페이스
//


#pragma once



class CExdbView : public CView
{
protected: // serialization에서만 만들어집니다.
	CExdbView();
	DECLARE_DYNCREATE(CExdbView)

// 특성입니다.
public:
	CExdbDoc* GetDocument() const;

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// 구현입니다.
public:
	virtual ~CExdbView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	void DrawConfigInfo(CDC* pDC);
private:
	// Any additional member variables
	CFont m_logFont;  // For log display
	int m_logStartY;   // Starting Y position for log
	CFont m_ConfigInfoFont;          // Custom font for config info
	CRect m_ConfigInfoRect;          // Rectangle area to draw in
};

#ifndef _DEBUG  // CExdbView.cpp의 디버그 버전
inline CExdbDoc* CExdbView::GetDocument() const
   { return reinterpret_cast<CExdbDoc*>(m_pDocument); }
#endif

