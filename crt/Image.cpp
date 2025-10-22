
// Tested on Windows 98 SE / Windows 7 Pro 64-bit
extern HBITMAP __fastcall ConvertMenuIconToBitmap(HICON hIcon) {
	DWORD MenuColor, MaskColor;
	ICONINFO icoInfo;
    HBITMAP pNewBMP;
    BITMAP bmpInfo;
    HDC hMemDC;
	int x, y;

	if ( !GetIconInfo(hIcon, &icoInfo) )
		return NULL;
	if ( !GetObjectA(icoInfo.hbmColor, sizeof(BITMAP), &bmpInfo) )
		return NULL;
	hMemDC = CreateCompatibleDC(NULL);
	if ( !hMemDC )
		return NULL;
	pNewBMP = CreateBitmap(bmpInfo.bmWidth, bmpInfo.bmHeight, bmpInfo.bmPlanes, bmpInfo.bmBitsPixel, NULL);
	if ( pNewBMP ) {
		SelectObject(hMemDC, pNewBMP);
		DrawIconEx(hMemDC, 0, 0, hIcon, bmpInfo.bmWidth, bmpInfo.bmHeight, 0, NULL, DI_IMAGE);
		MenuColor = GetSysColor(COLOR_MENU); // Default menu color
		MaskColor = GetPixel(hMemDC, 0, 0);  // Assume 1st corner pixel is transparent bit
		bmpInfo.bmWidth--;
		for ( x = bmpInfo.bmHeight-1; x >= 0; x-- )
			for ( y = bmpInfo.bmWidth; y >= 0; y-- )
				if ( GetPixel(hMemDC, x, y) == MaskColor )
					SetPixel(hMemDC, x, y, MenuColor);
	}
	DeleteDC(hMemDC);
	return pNewBMP;
}
