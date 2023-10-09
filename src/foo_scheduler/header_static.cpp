#include "pch.h"
#include "libPPUI/PaintUtils.h"
#include "libPPUI/wtl-pp.h"
#include "header_static.h"

using PaintUtils::PaintSeparatorControl;

HeaderStatic::HeaderStatic() :
	m_iLeftSpacing(8),
	m_clrText(GetSysColor(COLOR_CAPTIONTEXT))
{
	//..
}

void HeaderStatic::OnPaint(CDCHandle dcDummy)
{
	PaintSeparatorControl(*this);
}

BOOL HeaderStatic::SubclassWindow(HWND hWnd)
{
	if (!CWindowImpl<HeaderStatic, CStatic>::SubclassWindow(hWnd))
		return FALSE;

	SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));

	LOGFONTW lf;
	CWindowDC dc(core_api::get_main_window());
	CTheme wtheme;
	HTHEME theme = wtheme.OpenThemeData(core_api::get_main_window(), L"TEXTSTYLE");
	GetThemeFont(theme, dc, TEXT_BODYTEXT, 0, TMT_FONT, &lf);

	if (m_headerFont) {
		m_headerFont.DeleteObject();
	}
	m_headerFont = CreateFontIndirectW(&lf);
	CLogFont largeFont(m_headerFont);
	largeFont.MakeLarger(2);
	largeFont.MakeBolder(2);
	m_headerFont.DeleteObject();
	auto hFont = largeFont.CreateFontIndirect();
	m_headerFont.Attach(hFont);

	return TRUE;
}

void HeaderStatic::PaintHeader()
{
	CPaintDC dc(m_hWnd);

	CRect rect;
	GetClientRect(&rect);

	CString strText;
	GetWindowText(strText);

	SetFont(m_headerFont);
	DWORD dwStyle = GetStyle();
	if ((dwStyle & SS_CENTER) == SS_CENTER)
		dc.DrawText(strText, -1, rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	else if ((dwStyle & SS_LEFT) == SS_LEFT)
	{
		rect.left += m_iLeftSpacing;
		dc.DrawText(strText, -1, rect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
	}
	else // Right
	{
		rect.right -= m_iLeftSpacing;
		dc.DrawText(strText, -1, rect, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
	}
}
