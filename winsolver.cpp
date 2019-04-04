/*
Compile Resource File, then Build Application

  rc /Fowinsolver.res winsolver.rc

  cl /nologo /W3 /wd4996 /Ox /Foobj/ /Febin/winsolver /Tp winsolver.cpp /Tc memrealloc.c /Tc mtrx_t.c user32.lib comctl32.lib gdi32.lib winsolver.res

*/

#include "winsolver.h"

/* size_t format specifier for older VS versions */
#if defined (_WIN32)
#define SZTFMT "u"
#elif defined (_WIN64)
#define SZTFMT "lu"
#else
#define SZTFMT "zu"
#endif

#define RESULTSZ        32
#define MAX_LOADSTRING 100

/* Global Variables: */
HINSTANCE   hInst;                      /* current instance of app */
WCHAR szTitle[MAX_LOADSTRING];          /* The title bar text */
WCHAR szWindowClass[MAX_LOADSTRING];    /* the main window class name */
HWND hWndEdit;                          /* edit box instance */
HWND hWndTool;                          /* toolbar instance */
/* monospace font for edit box */
HFONT hFont = CreateFont(13, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, 
                DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
                CLEARTYPE_QUALITY, FIXED_PITCH, TEXT("DejaVu Sans Mono"));
HIMAGELIST g_hImageList = NULL;         /* imagelist for toolbar */

/* function prototypes */
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
HWND                CreateSimpleToolbar(HWND hWndParent);
void                writesolution (void);

/* intro string constant */
const wchar_t *intro = L"Click [Clear] and enter Coefficient Matrix with Constant Vector as Last Column\r\n"
L"\r\n"
L"Example (3 x 4):\r\n"
L"\r\n"
L" 3.0  2.0  -4.0   3.0\r\n"
L" 2.0  3.0   3.0  15.0\r\n"
L" 5.0 -3.0   1.0  14.0\r\n"
L"\r\n"
L"or\r\n"
L"\r\n"
L"3,2,-4,3\r\n"
L"2,3,3,15\r\n"
L"5,-3,1,14\r\n"
L"\r\n"
L"Then click [Solve...]\r\n"
L"\r\n"
L"Solution Vector:\r\n"
L"\r\n"
L" x[  0] :   3.0000000\r\n"
L" x[  1] :   1.0000000\r\n"
L" x[  2] :   2.0000000\r\n";


int APIENTRY WinMain (HINSTANCE hInstance,
              HINSTANCE hPrevInstance,
              LPSTR    lpCmdLine,
              int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    /* initialize global strings */
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINSOLV, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    /* initialize application / validate */
    if (!InitInstance (hInstance, nCmdShow)) {
        return FALSE;
    }
    /* load keyboard accelerators */
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINSOLV));

    MSG msg;

    /* main message loop */
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

/* register the window class */
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINSOLV));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINSOLV);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

/* save instance handle and create/display main window */
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);

    if (!hWnd)  /* validate CreateWindow */
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);

    UpdateWindow(hWnd);

    return TRUE;
}

/* processe messages for the main window. */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        {
        hWndTool = CreateSimpleToolbar(hWnd);
        hWndEdit = CreateWindow (
                    L"edit",           // The class name required is edit
                    L"Linear System Solver", // Default text, styles below
                    WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL| 
                    ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL,
                    0,0,                    // the left and top co-ordinates
                    0,0,                    // size with WM_SIZE message
                    hWnd,                   // parent window handle
                    (HMENU)IDM_EDIT,        // the ID of your editbox
                    hInst,                  // the instance of your application
                    NULL                    // extra bits you dont really need
        
        );
        /* set the font to Fixed (DejaVu Sans Mono) */
        SendMessage (hWndEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true,0));
	    SendMessage (hWndEdit, WM_SETTEXT, 0, (LPARAM)intro);
        return 0;
        }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            /* parse the menu/toolbar selections */
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_CLEAR:
                SendMessage (hWndEdit, WM_SETTEXT, 0, (LPARAM)L"");
                break;
            case IDM_EXIT:
                DeleteObject ((HGDIOBJ)hFont);  /* free font resources */
                DestroyWindow(hWnd);
                break;
            case IDM_HELP:
                SendMessage (hWndEdit, WM_SETTEXT, 0, (LPARAM)intro);
                break;
            case IDM_SOLV:
                // SendMessage (hWndEdit, WM_SETTEXT, 0, (LPARAM)L"Solving...");
                writesolution();
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            /* initial write before edit control added */
            // TextOut (hdc, 5, 15, L"Hello Windows World!", 20);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_SETFOCUS:
        SetFocus(hWndEdit);
        return 0;
    case WM_SIZE:
        /* make the edit control the size of the window's client area. */
        MoveWindow(hWndTool,
                    0, 0,                  // starting x- and y-coordinates
                    LOWORD(lParam),        // width of client area
                    40,                    // height of toolbar
                    TRUE);                 // repaint window
        MoveWindow(hWndEdit,
                    0, 40,                 // starting x- and y-coordinates
                    LOWORD(lParam),        // width of client area
                    HIWORD(lParam) - 40,   // height of client area - toolbar
                    TRUE);                 // repaint window
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

/* message handler for about box */
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

/* create simple 4-button toolbar to correspond with menu entries */
HWND CreateSimpleToolbar(HWND hWndParent)
{
    const int ImageListID    = 0;   /* toolbar constants */
    const int numButtons     = 4;
    const int bitmapSize     = 16;
    
    const DWORD buttonStyles = BTNS_AUTOSIZE;

    /* create the toolbar with flat/transparent buttons w/text on side */
    HWND hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, 
                                      WS_CHILD | TBSTYLE_WRAPABLE | 
                                      TBSTYLE_LIST | TBSTYLE_FLAT,
                                      0, 0, 0, 0, 
                                      hWndParent, NULL, hInst, NULL);
        
    if (hWndToolbar == NULL)    /* validate toolbar creation */
        return NULL;

    /* create the image list */
    g_hImageList = ImageList_Create(bitmapSize, bitmapSize,   // Dimensions of individual bitmaps.
                                    ILC_COLOR16 | ILC_MASK,   // Ensures transparent background.
                                    numButtons, 0);

    /* set the image list */
    SendMessage(hWndToolbar, TB_SETIMAGELIST, 
                (WPARAM)ImageListID, 
                (LPARAM)g_hImageList);

    /* load the button images */
    SendMessage(hWndToolbar, TB_LOADIMAGES, 
                (WPARAM)IDB_STD_SMALL_COLOR, 
                (LPARAM)HINST_COMMCTRL);

    /* Initialize button info (can move to resource file) */
    TBBUTTON tbButtons[numButtons] = 
    {
        { MAKELONG(STD_DELETE,  ImageListID), IDM_CLEAR,  TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)L"Clear" },
        { MAKELONG(STD_PROPERTIES,  ImageListID), IDM_SOLV,  TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)L"Solve" },
        { MAKELONG(STD_HELP, ImageListID), IDM_HELP, TBSTATE_ENABLED,               buttonStyles, {0}, 0, (INT_PTR)L"Help"},
        { MAKELONG(STD_FILEOPEN, ImageListID), IDM_EXIT, TBSTATE_ENABLED, buttonStyles, {0}, 0, (INT_PTR)L"Exit"}
    };

    /* add buttons */
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS,       (WPARAM)numButtons,       (LPARAM)&tbButtons);

    /* resize the toolbar, and display */
    SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0); 
    ShowWindow(hWndToolbar,  TRUE);
    
    return hWndToolbar;
}

/* realloc with new/delete to fixed size */
void *szrealloc (void *ptr, const size_t ptrsz, const size_t oldsz, const size_t newsz)
{
    void *memptr = new char[newsz * ptrsz];
    
    if (newsz > oldsz) {
        memcpy (memptr, ptr, oldsz * ptrsz);
        memset ((char*)memptr + (oldsz * ptrsz), 0, (newsz - oldsz) * ptrsz);
    }
    else
        memcpy (memptr, ptr, newsz * ptrsz);
    
    delete [] ptr;
    return memptr;
}

/* realloc with new/delete */
void *szrealloc2 (void *ptr, const size_t ptrsz, size_t *nelem)
{
    void *memptr = new char[2 * *nelem * ptrsz];
    
    memcpy (memptr, ptr, *nelem * ptrsz);
    memset ((char*)memptr + (*nelem * ptrsz), 0, *nelem * ptrsz);
    *nelem *= 2;
    
    delete [] ptr;
    return memptr;
}

/* retrieve coefficient matrix from editbox, convert to C-string, and
 * pass to equation solver. retrieve the solution vector, append to 
 * editbox content with header, convert to wchar_t and write back to
 * editbox.
 */
void writesolution (void)
{
	/* get number of characters in edit box */
    LRESULT iTextSize = SendMessage(hWndEdit, EM_GETLIMITTEXT, 0, 0);
	wchar_t *szText = new wchar_t[iTextSize],  /* allocate buffer */
            *szSolv;    /* pointer to text + solution */
    char *cstr,         /* pointer to conversion to string */
        *p,             /* pointer to beginning of system of equations */
        line[1024];     /* static line to build solution vector */
    size_t clen = 0, wlen = 0, ccvted = 0, wcvted = 0, i;
    bool havevalue = FALSE;
	
    /* get text from edit box */
    SendMessage(hWndEdit, WM_GETTEXT, iTextSize, (LPARAM)szText);
    
    wlen = wcslen (szText);     /* get wide character length */
    clen = wlen * 2;            /* double size for C-string */
    cstr = new char[clen];      /* allocate storage for C-string */
    
    /* convert wide-char to C-string */
    wcstombs_s (&ccvted, cstr, wlen, szText, _TRUNCATE);
    p = cstr;
    
    /* advance start of first value in system of equations */
    do {
        if ( *p == '.' || *p == '-' || *p == '+' || isdigit (*p)) {
            havevalue = TRUE;
            break;
        }
    } while (*p++);
    
    if (havevalue) {
        /* fill matrix from values in buffer */
        mtrx_t *m = mtrx_read_alloc_buf (p);
        
        /* solve system by gauss-jordan elimintation will full-pivoting */
        mtrx_solv_gaussj (m->mtrx, m->rows);    /* m->mtrx now contains
                                                * inverse + solution vector
                                                */
        
        /* realloc C-string to append Solution Vector */
        cstr = (char *)szrealloc (cstr, sizeof *cstr, clen, 
                                    clen + (m->rows + 1) * RESULTSZ);
        
        /* append solution vector heading */
        strcat (cstr, "\r\n\r\nSolution Vector:\r\n\r\n");
        
        for (i = 0; i < m->rows; i++) { /* appending individual components */
            sprintf (line, " x[%3" SZTFMT "] : % 11.7f\r\n", 
                        i, m->mtrx[i][m->rows]);
            strcat (cstr, line);
        }
        
        clen = strlen (cstr);           /* get new length of C-string */
        szSolv = new wchar_t[clen + 1]; /* allocate for conversion to wchar_t */
        
        /* convert C-string to wchar_t */
        mbstowcs_s (&wcvted, szSolv, clen + 1, cstr, _TRUNCATE);
        
        /* write to editbox */
        SendMessage (hWndEdit, WM_SETTEXT, 0, (LPARAM)szSolv);
    }
    else {
        clen = strlen (cstr);   /* get new length of C-string */
        
        /* realloc C-string to append error message */
        cstr = (char *)szrealloc (cstr, sizeof *cstr, clen, 
                                    clen + 2 * RESULTSZ);
        
        /* append error text */
        strcat (cstr, "\r\n\r\n ERROR Invalid Matrix Format\r\n");
        
        clen = strlen (cstr);           /* get new length of C-string */
        szSolv = new wchar_t[clen + 1]; /* allocate for conversion to wchar_t */
        
        /* convert C-string to wchar_t */
        mbstowcs_s (&wcvted, szSolv, clen + 1, cstr, _TRUNCATE);
        
        /* write to editbox */
        SendMessage (hWndEdit, WM_SETTEXT, 0, (LPARAM)szSolv);
    }
    
    delete [] szText;   /* free allocated memory */
    delete [] szSolv;
    delete [] cstr;
}
