#pragma once

class CPoint : public POINT
{
public:
	CPoint() {}
	CPoint(LONG x, LONG y) {this->x = x; this->y = y;}
	CPoint(const POINT &pt) {this->x = pt.x; this->y = pt.y;}

	inline operator POINT *() {return this;}
	inline operator const POINT *() const {return this;}
};

class CRect : public RECT
{
public:
	CRect() {}
	CRect(LONG left, LONG top, LONG right, LONG bottom) {this->left = left; this->top = top; this->right = right; this->bottom = bottom;}
	CRect(const RECT &rc) {this->left = rc.left; this->top = rc.top; this->right = rc.right; this->bottom = rc.bottom;}

	inline operator RECT *() {return this;}
	inline operator const RECT *() const {return this;}

	inline int Width() const {return right - left;}
	inline int Height() const {return bottom - top;}
};

class CBitmap
{
public:
	HBITMAP m_hBitmap;

	CBitmap(HBITMAP hBitmap = NULL) : m_hBitmap(hBitmap) {
	}

	~CBitmap() {
		if (m_hBitmap != NULL)
			::DeleteObject(m_hBitmap);
	}

	CBitmap &operator =(HBITMAP hBitmap) {
		Attach(hBitmap);
		return *this;
	}

	void Attach(HBITMAP hBitmap) {
		if (m_hBitmap != NULL && m_hBitmap != hBitmap)
			::DeleteObject(m_hBitmap);
		m_hBitmap = hBitmap;
	}

	HBITMAP Detach() {
		HBITMAP hBitmap = m_hBitmap;
		m_hBitmap = NULL;
		return hBitmap;
	}

	inline operator HBITMAP() const {return m_hBitmap;}
};

class CFont
{
public:
	HFONT m_hFont;

	CFont(HFONT hFont = NULL) : m_hFont(hFont) {
	}

	~CFont() {
		if (m_hFont != NULL)
			::DeleteObject(m_hFont);
	}

	CFont &operator =(HFONT hFont) {
		Attach(hFont);
		return *this;
	}

	void Attach(HFONT hFont) {
		if (m_hFont != NULL && m_hFont != hFont)
			::DeleteObject(m_hFont);
		m_hFont = hFont;
	}

	HFONT Detach() {
		HFONT hFont = m_hFont;
		m_hFont = NULL;
		return hFont;
	}

	inline operator HFONT() const {return m_hFont;}
};

class CDC
{
public:
	HDC m_hDC;

	CDC(HDC hDC = NULL) : m_hDC(hDC) {
	}

	~CDC() {
		if (m_hDC != NULL)
			::DeleteDC(m_hDC);
	}

	CDC &operator =(HDC hDC) {
		Attach(hDC);
		return *this;
	}

	void Attach(HDC hDC) {
		if (m_hDC != hDC && m_hDC != hDC)
			::DeleteDC(m_hDC);
		m_hDC = hDC;
	}

	HDC Detach() {
		HDC hDC = m_hDC;
		m_hDC = NULL;
		return m_hDC;
	}

	inline operator HDC() const {return m_hDC;}
};

class CPaintDC : public CDC
{
public:
	HWND m_hWnd;
	PAINTSTRUCT m_ps;

	CPaintDC(HWND hWnd) {
		m_hWnd = hWnd;
		m_hDC = ::BeginPaint(m_hWnd, &m_ps);
	}

	~CPaintDC() {
		::EndPaint(m_hWnd, &m_ps);
		Detach();
	}
};

class CMemoryDC : public CDC
{
public:
	HDC m_hDCOriginal;
	CRect m_rcPaint;
	CBitmap m_bmp;
	HBITMAP m_hBmpOld;

	CMemoryDC(HDC hDC, RECT &rcPaint) : m_hDCOriginal(hDC), m_hBmpOld(NULL) {
		m_rcPaint = rcPaint;
		Attach(::CreateCompatibleDC(m_hDCOriginal));
		m_bmp = ::CreateCompatibleBitmap(m_hDCOriginal, m_rcPaint.Width(), m_rcPaint.Height());
		m_hBmpOld = (HBITMAP)::SelectObject(m_hDC, m_bmp);
		::SetWindowOrgEx(m_hDC, m_rcPaint.left, m_rcPaint.top, NULL);
	}

	~CMemoryDC() {
		::BitBlt(m_hDCOriginal, m_rcPaint.left, m_rcPaint.top, m_rcPaint.Width(), m_rcPaint.Height(), m_hDC, m_rcPaint.left, m_rcPaint.top, SRCCOPY);
		::SelectObject(m_hDC, m_hBmpOld);
	}
};

// A light-weight implementation helper for window classes.
template<class T>
class CSimpleWindowImpl
{
private:
	static LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static ATOM g_class_atom;
	static LONG g_refcount;

public:
	HWND m_hWnd;

	CSimpleWindowImpl() {m_hWnd = NULL;}

	HWND Create(HWND hWndParent, LPCTSTR szWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu = (HMENU)NULL);
	void Destroy();

	inline operator HWND() const {return m_hWnd;}
	inline bool IsWindow() const {return ::IsWindow(m_hWnd) != FALSE;}

	// Override these in an implementation class.
	static LPCTSTR GetClassName();
	BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT & lResult);
};

template <class T>
ATOM CSimpleWindowImpl<T>::g_class_atom = 0;

template <class T>
LONG CSimpleWindowImpl<T>::g_refcount = 0;

template <class T>
LPCTSTR CSimpleWindowImpl<T>::GetClassName() {
	// set up a unique class name using the address of a variable in out class.
	static int dummy = 0;
	static pfc::stringcvt::string_os_from_utf8 os_class_name;
	if (os_class_name.is_empty())
	{
		pfc::string_formatter class_name;
		class_name << "CLS" << pfc::format_hex((t_uint64)&dummy, sizeof(&dummy) * 2);
		os_class_name.convert(class_name);
	}
	return os_class_name;
}

template <class T>
HWND CSimpleWindowImpl<T>::Create(HWND hWndParent, LPCTSTR szWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HMENU hMenu) {
	T *_this = static_cast<T *>(this);

	assert(_this->m_hWnd == NULL);

	if (g_refcount == 0) {
		if (g_class_atom == NULL) {
			WNDCLASS wc = { 0 };
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			wc.lpfnWndProc = &WindowProc;
			wc.hInstance = core_api::get_my_instance();
			wc.hCursor = ::LoadCursor(0, IDC_ARROW);
			wc.hbrBackground = 0;
			wc.lpszClassName = T::GetClassName();
			wc.cbWndExtra = sizeof(CSimpleWindowImpl<T> *);
			g_class_atom = ::RegisterClass(&wc);
		}
	}

	if (g_class_atom != NULL) {
		g_refcount++;

		return ::CreateWindowEx(
			dwExStyle,
			(LPCTSTR)g_class_atom,
			szWindowName,
			dwStyle,
			x, y, nWidth, nHeight,
			hWndParent,
			hMenu,
			core_api::get_my_instance(),
			_this );
	}

	return NULL;
}

template <class T>
void CSimpleWindowImpl<T>::Destroy() {
	// Destroy the window.
	if (m_hWnd) {
		::DestroyWindow(m_hWnd);

		g_refcount--;

		if (g_refcount == 0) {
			::UnregisterClass((LPCTSTR)g_class_atom, core_api::get_my_instance());
			g_class_atom = 0;
		}
	}
}

template <class T>
LRESULT WINAPI CSimpleWindowImpl<T>::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	T *_this;

	if (uMsg == WM_NCCREATE) {
		LPCREATESTRUCT lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		_this = reinterpret_cast<T *>(lpCreateStruct->lpCreateParams);
		_this->m_hWnd = hWnd;
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)_this);
	} else {
		 _this = reinterpret_cast<T *>(GetWindowLongPtr(hWnd, 0));
	}

	BOOL bHandled = FALSE;
	LRESULT lResult = 0;
	if (_this != 0) {
		try {
			BOOL bHandled = _this->ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult);
		}
		catch (const std::exception &) {
		}
	}
	if (!bHandled) {
		lResult = DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	if (uMsg == WM_NCDESTROY) {
		_this->m_hWnd = 0;
	}

	return lResult;
}

template <class T>
BOOL CSimpleWindowImpl<T>::ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT & lResult) {
	lResult = ::DefWindowProc(m_hWnd, msg, wParam, lParam);
	return TRUE;
}
