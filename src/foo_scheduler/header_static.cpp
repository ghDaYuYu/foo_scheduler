#include "pch.h"
#include "libPPUI/PaintUtils.h"
#include "libPPUI/wtl-pp.h"
#include "header_static.h"

using PaintUtils::PaintSeparatorControl;

HeaderStatic::HeaderStatic() :
	m_iLeftSpacing(8),
	m_clrText(GetSysColor(COLOR_CAPTIONTEXT))
{
}

void HeaderStatic::OnPaint(CDCHandle dcDummy)
{
	PaintSeparatorControl(*this);
}

void HeaderStatic::DrawGradRect(CPaintDC& dc, const CRect& r, COLORREF clrLeft, COLORREF clrRight)
{
	float fStep = static_cast<float>(r.Width()) / 255.0f;

	for(int iOnBand = 0; iOnBand < 255; ++iOnBand)
	{
		CRect rectStep(
			r.left + static_cast<int>(std::floor(iOnBand * fStep + 0.5)),
			r.top,
			r.left + static_cast<int>(std::floor((iOnBand + 1) * fStep + 0.5)),
			r.bottom);

		BYTE btNewR = static_cast<BYTE>(
			(GetRValue(clrRight) - GetRValue(clrLeft)) *
			static_cast<float>(iOnBand) / 255.0f + GetRValue(clrLeft));

		BYTE btNewG = static_cast<BYTE>(
			(GetGValue(clrRight) - GetGValue(clrLeft)) *
			static_cast<float>(iOnBand) / 255.0f + GetGValue(clrLeft));

		BYTE btNewB = static_cast<BYTE>(
			(GetBValue(clrRight) - GetBValue(clrLeft)) *
			static_cast<float>(iOnBand) / 255.0f + GetBValue(clrLeft));

		dc.FillSolidRect(rectStep, RGB(btNewR, btNewG, btNewB));
	}
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

	if (m_newFont)
		m_newFont.DeleteObject();
	m_newFont = CreateFontIndirectW(&lf);

	LOGFONT new_lf;
	m_newFont.GetLogFont(&new_lf);

	CLogFont newcFont(m_newFont);
	newcFont.MakeLarger(2);
	newcFont.MakeBolder(2);
	m_newFont.DeleteObject();
	auto hf = newcFont.CreateFontIndirect();
	m_newFont.Attach(hf);

	return TRUE;
}

void HeaderStatic::PaintGradientHeader()
{
	CPaintDC dc(m_hWnd);

	CRect rect;
	GetClientRect(&rect);

	CString strText;
	GetWindowText(strText);

	SetFont(m_newFont);
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
