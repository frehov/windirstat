// OwnerDrawnListControl.cpp	- Implementation of COwnerDrawnListItem and COwnerDrawnListControl
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003 Bernhard Seifert
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: bseifert@users.sourceforge.net, bseifert@daccord.net

#include "stdafx.h"
#include "windirstat.h"
#include ".\ownerdrawnlistcontrol.h"

namespace
{
	const int TEXT_X_MARGIN = 6;	// horizontaler Abstand der Texte vom Rand des Item-Rechtecks

	const UINT LABEL_INFLATE_CX = 3;// um soviel wird das Label vergr��ert, um das Selektions- und Fokus-
	const UINT LABEL_INFLATE_CY = 1;// rechteck zu erhalten

	const UINT GENERAL_INDENT = 5;
}

/////////////////////////////////////////////////////////////////////////////

COwnerDrawnListItem::COwnerDrawnListItem()
{
}

COwnerDrawnListItem::~COwnerDrawnListItem()
{
}

void COwnerDrawnListItem::DrawLabel(COwnerDrawnListControl *list, CImageList *il, CDC *pdc, CRect& rc, UINT state, int *width, bool indent) const
{
	CRect rcRest= rc;
	if (indent)
		rcRest.left+= GENERAL_INDENT;

	ASSERT(GetImage() < il->GetImageCount());

	IMAGEINFO ii;
	il->GetImageInfo(GetImage(), &ii);
	CRect rcImage(ii.rcImage);

	if (width == NULL)
	{
		CPoint pt(rcRest.left, rcRest.top + rcRest.Height() / 2 - rcImage.Height() / 2);
		il->Draw(pdc, GetImage(), pt, ILD_NORMAL);
	}

	rcRest.left+= rcImage.Width();

	CSelectObject sofont(pdc, list->GetFont());

	rcRest.DeflateRect(list->GetTextXMargin(), 0);

	CRect rcLabel= rcRest;
	pdc->DrawText(GetText(0), rcLabel, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_CALCRECT);
	rcLabel.top= rcRest.top + (rcRest.bottom - rcLabel.bottom);
	rcLabel.InflateRect(LABEL_INFLATE_CX, LABEL_INFLATE_CY);

	CSetBkMode bk(pdc, TRANSPARENT);
	COLORREF textColor= RGB(0,0,0);
	if (width == NULL && (state & ODS_SELECTED) != 0 && (GetFocus() == list->m_hWnd || (list->GetStyle() & LVS_SHOWSELALWAYS) != 0))
	{
		pdc->FillSolidRect(rcLabel, RGB(0,0,180));
		textColor= GetSysColor(COLOR_WINDOW);
	}
	CSetTextColor stc(pdc, textColor);
	if (width == NULL)
		pdc->DrawText(GetText(0), rcRest, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS);
	else
		pdc->DrawText(GetText(0), rcRest, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);

	rcLabel.InflateRect(1, 1);

	if (state & ODS_FOCUS && width == NULL)
		pdc->DrawFocusRect(rcLabel);

	if (width == NULL)
		DrawAdditionalState(pdc, rcLabel);

	rcLabel.left= rc.left;
	rc= rcLabel;

	if (width != NULL)
		*width= rcLabel.Width() + 5; // Don't know, why +5
}


/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(COwnerDrawnListControl, CSortingListControl)

COwnerDrawnListControl::COwnerDrawnListControl(LPCTSTR name, int rowHeight)
	: CSortingListControl(name)
{
	ASSERT(rowHeight > 0);
	m_rowHeight= rowHeight;
	m_showGrid= false;
}

COwnerDrawnListControl::~COwnerDrawnListControl()
{
}

// This method MUST be called before the Control is shown.
void COwnerDrawnListControl::OnColumnsInserted()
{
	// The pacmen shall not draw over our header control.
	ModifyStyle(0, WS_CLIPCHILDREN);

	// Where does the 1st Item begin vertically?
	if (GetItemCount() > 0)
	{
		CRect rc;
		GetItemRect(0, rc, LVIR_BOUNDS);
		m_yFirstItem= rc.top;
	}
	else
	{
		InsertItem(0, _T("_tmp"), 0);
		CRect rc;
		GetItemRect(0, rc, LVIR_BOUNDS);
		DeleteItem(0);
		m_yFirstItem= rc.top;
	}

	LoadPersistentAttributes();
}

int COwnerDrawnListControl::GetRowHeight()
{
	return m_rowHeight;
}

void COwnerDrawnListControl::ShowGrid(bool show)
{
	m_showGrid= show;
	if (IsWindow(m_hWnd))
		InvalidateRect(NULL);
}

int COwnerDrawnListControl::GetTextXMargin()
{
	return TEXT_X_MARGIN;
}

int COwnerDrawnListControl::GetGeneralLeftIndent()
{
	return GENERAL_INDENT;
}

COwnerDrawnListItem *COwnerDrawnListControl::GetItem(int i)
{
	COwnerDrawnListItem *item= (COwnerDrawnListItem *)GetItemData(i);
	return item;
}

int COwnerDrawnListControl::FindListItem(const COwnerDrawnListItem *item)
{
	LVFINDINFO fi;
	ZeroMemory(&fi, sizeof(fi));
	fi.flags= LVFI_PARAM;
	fi.lParam= (LPARAM)item;

	int i= FindItem(&fi);

	return i;
}

void COwnerDrawnListControl::DrawItem(LPDRAWITEMSTRUCT pdis)
{
	COwnerDrawnListItem *item= (COwnerDrawnListItem *)(pdis->itemData);
	CDC *pdc= CDC::FromHandle(pdis->hDC);
	CRect rcItem(pdis->rcItem);
	if (m_showGrid)
	{
		rcItem.bottom--;
		rcItem.right--;
	}
	
	CDC dcmem;

	dcmem.CreateCompatibleDC(pdc);
	CBitmap bm;
	bm.CreateCompatibleBitmap(pdc, rcItem.Width(), rcItem.Height());
	CSelectObject sobm(&dcmem, &bm);

	dcmem.FillSolidRect(rcItem - rcItem.TopLeft(), GetSysColor(COLOR_WINDOW));

	for (int i=0; i < GetHeaderCtrl()->GetItemCount(); i++)
	{
		CRect rc= GetWholeSubitemRect(pdis->itemID, i);

		CRect rcDraw= rc - rcItem.TopLeft();

		if (!item->DrawSubitem(i, &dcmem, rcDraw, pdis->itemState, NULL))
		{
			CRect rcText= rcDraw;
			rcText.DeflateRect(TEXT_X_MARGIN, 0);
			CSetBkMode bk(&dcmem, TRANSPARENT);
			CSelectObject sofont(&dcmem, GetFont());
			CString s= item->GetText(i);
			UINT align= IsColumnRightAligned(i) ? DT_RIGHT : DT_LEFT;
			dcmem.DrawText(s, rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | align);
			// Test: dcmem.FillSolidRect(rcDraw, 0);
		}

		pdc->BitBlt(rcItem.left + rcDraw.left, rcItem.top + rcDraw.top, rcDraw.Width(), rcDraw.Height(), &dcmem, rcDraw.left, rcDraw.top, SRCCOPY);
	}
}

bool COwnerDrawnListControl::IsColumnRightAligned(int col)
{
	HDITEM hditem;
	ZeroMemory(&hditem, sizeof(hditem));
	hditem.mask= HDI_FORMAT;

	GetHeaderCtrl()->GetItem(col, &hditem);

	return (hditem.fmt & HDF_RIGHT) != 0;
}

CRect COwnerDrawnListControl::GetWholeSubitemRect(int item, int subitem)
{
	CRect rc;
	if (subitem == 0)
	{
		// Special case column 0:
		// If we did GetSubItemRect(item 0, LVIR_LABEL, rc)
		// and we have an image list, then we would get the rectangle
		// excluding the image.
		HDITEM hditem;
		hditem.mask= HDI_WIDTH;
		GetHeaderCtrl()->GetItem(0, &hditem);

		VERIFY(GetItemRect(item, rc, LVIR_LABEL));
		rc.left= rc.right - hditem.cxy;
	}
	else
	{
		VERIFY(GetSubItemRect(item, subitem, LVIR_LABEL, rc));
	}

	if (m_showGrid)
	{
		rc.right--;
		rc.bottom--;
	}
	return rc;
}

int COwnerDrawnListControl::GetSubItemWidth(COwnerDrawnListItem *item, int subitem)
{
	int width;

	CClientDC dc(this);
	CRect rc(0, 0, 1000, 1000);
	
	if (item->DrawSubitem(subitem, &dc, rc, 0, &width))
		return width;

	CString s= item->GetText(subitem);
	if (s.IsEmpty())
	{
		// DrawText(..DT_CALCRECT) scheint mit Leerstrings nicht klarzukommen
		return 0;
	}

	CSelectObject sofont(&dc, GetFont());
	UINT align= IsColumnRightAligned(subitem) ? DT_RIGHT : DT_LEFT;
	dc.DrawText(s, rc, DT_SINGLELINE | DT_VCENTER | align | DT_CALCRECT);

	rc.InflateRect(TEXT_X_MARGIN, 0);
	return rc.Width();
}


BEGIN_MESSAGE_MAP(COwnerDrawnListControl, CSortingListControl)
	ON_WM_ERASEBKGND()
	ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, OnHdnDividerdblclick)
	ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, OnHdnDividerdblclick)
	ON_WM_VSCROLL()
	ON_NOTIFY(HDN_ITEMCHANGINGA, 0, OnHdnItemchanging)
	ON_NOTIFY(HDN_ITEMCHANGINGW, 0, OnHdnItemchanging)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

BOOL COwnerDrawnListControl::OnEraseBkgnd(CDC* pDC)
{
	ASSERT(GetHeaderCtrl()->GetItemCount() > 0);

	// We should recalculate m_yFirstItem here (could have changed e.g. when
	// the XP-Theme changed).
	if (GetItemCount() > 0)
	{
		CRect rc;
		GetItemRect(0, rc, LVIR_BOUNDS);
		m_yFirstItem= rc.top;
	}
	// else: if we did the same thing as in OnColumnsCreated(), we get
	// repaint problems.

	const COLORREF gridColor= RGB(212,208,200);

	CRect rcClient;
	GetClientRect(rcClient);

	CRect rcHeader;
	GetHeaderCtrl()->GetWindowRect(rcHeader);
	ScreenToClient(rcHeader);

	CRect rcBetween= rcClient;	// between header and first item
	rcBetween.top= rcHeader.bottom;
	rcBetween.bottom= m_yFirstItem;
	pDC->FillSolidRect(rcBetween, gridColor);

	CArray<int, int> columnOrder;
	columnOrder.SetSize(GetHeaderCtrl()->GetItemCount());
	GetColumnOrderArray(columnOrder.GetData(), columnOrder.GetSize());

	CArray<int, int> vertical;
	vertical.SetSize(GetHeaderCtrl()->GetItemCount());
	
	int x= - GetScrollPos(SB_HORZ);
	HDITEM hdi;
	ZeroMemory(&hdi, sizeof(hdi));
	hdi.mask= HDI_WIDTH;
	for (int i=0; i < GetHeaderCtrl()->GetItemCount(); i++)
	{
		GetHeaderCtrl()->GetItem(columnOrder[i], &hdi);
		x+= hdi.cxy;
		vertical[i]= x;
	}

	if (m_showGrid)
	{
		CPen pen(PS_SOLID, 1, gridColor);
		CSelectObject sopen(pDC, &pen);

		for (int y=m_yFirstItem + GetRowHeight() - 1; y < rcClient.bottom; y+= GetRowHeight())
		{
			pDC->MoveTo(rcClient.left, y);
			pDC->LineTo(rcClient.right, y);
		}

		for (int i=0; i < vertical.GetSize(); i++)
		{
			pDC->MoveTo(vertical[i] - 1, rcClient.top);
			pDC->LineTo(vertical[i] - 1, rcClient.bottom);
		}
	}

	const int gridWidth= m_showGrid ? 1 : 0;
	const COLORREF bgcolor= GetSysColor(COLOR_WINDOW);

	const int lineCount= GetCountPerPage() + 1;
	const int firstItem= GetTopIndex();
	const int lastItem= min(firstItem + lineCount, GetItemCount()) - 1;

	ASSERT(GetItemCount() == 0 || firstItem < GetItemCount());
	ASSERT(GetItemCount() == 0 || lastItem < GetItemCount());
	ASSERT(GetItemCount() == 0 || lastItem >= firstItem);

	const int itemCount= lastItem - firstItem + 1;

	CRect fill;
	fill.left= vertical[vertical.GetSize() - 1];
	fill.right= rcClient.right;
	fill.top= m_yFirstItem;
	fill.bottom= fill.top + GetRowHeight() - gridWidth;
	for (i=0; i < itemCount; i++)
	{
		pDC->FillSolidRect(fill, bgcolor);
		fill.OffsetRect(0, GetRowHeight());
	}

	int top= fill.top;
	while (top < rcClient.bottom)
	{
		fill.top= top;
		fill.bottom= top + GetRowHeight() - gridWidth;
		
		int left= 0;
		for (int i=0; i < vertical.GetSize(); i++)
		{
			fill.left= left;
			fill.right= vertical[i] - gridWidth;

			pDC->FillSolidRect(fill, bgcolor);

			left= vertical[i];
		}
		fill.left= left;
		fill.right= rcClient.right;
		pDC->FillSolidRect(fill, bgcolor);

		top+= GetRowHeight();
	}
	return true;
}

void COwnerDrawnListControl::OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	CWaitCursor wc;
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	int subitem= phdr->iItem;

	AdjustColumnWidth(subitem);

	*pResult = 0;
}

void COwnerDrawnListControl::AdjustColumnWidth(int col)
{
	CWaitCursor wc;

	int width= 10;
	for (int i=0; i < GetItemCount(); i++)
	{
		int w= GetSubItemWidth(GetItem(i), col);
		if (w > width)
			width= w;
	}
	SetColumnWidth(col, width + 5);
}

void COwnerDrawnListControl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

	// Owner drawn list controls with LVS_EX_GRIDLINES don't repaint correctly
	// when scrolled (under Windows XP). So we fource a complete repaint here.
	InvalidateRect(NULL);
}

void COwnerDrawnListControl::OnHdnItemchanging(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	// Unused: LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	Default();
	InvalidateRect(NULL);
	
	*pResult = 0;
}
