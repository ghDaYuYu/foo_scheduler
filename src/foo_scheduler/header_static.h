#pragma once

class HeaderStatic : public CWindowImpl<HeaderStatic, CStatic>
{
public:
	HeaderStatic();

	BOOL SubclassWindow(HWND hWnd);

private:
	DECLARE_WND_SUPERCLASS(0, CStatic::GetWndClassName())

	BEGIN_MSG_MAP(HeaderStatic)
		MSG_WM_PAINT(OnPaint)
	END_MSG_MAP()

	void OnPaint(CDCHandle dcDummy);
	void PaintGradientHeader();

	void SetTextColor(COLORREF clrText)
	{
		m_clrText = clrText;
	}

protected:
	int	m_iLeftSpacing;

	COLORREF m_clrText;

	CFont m_newFont;

private:
	void DrawGradRect(CPaintDC& dc, const CRect& r, COLORREF clrLeft, COLORREF clrRight);
};

