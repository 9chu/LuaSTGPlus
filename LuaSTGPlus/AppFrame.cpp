#include "AppFrame.h"
#include "Utility.h"
#include "LuaWrapper.h"

#include "resource.h"

#include "D3D9.H"  // for SetFog

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

// ����lua��չ
extern "C" int luaopen_lfs(lua_State *L);
extern "C" int luaopen_cjson(lua_State* L);

using namespace std;
using namespace LuaSTGPlus;

#ifdef LSHOWFONTBASELINE
class FontBaseLineDebugHelper :
	public f2dFontRendererListener
{
private:
	f2dGraphics2D* m_pGraph2D;
	f2dGeometryRenderer* m_pGRenderer;
protected:
	fBool OnGlyphBeginDraw(fuInt Index, fCharW Character, fcyVec2& DrawPos, fcyVec2& Adv)
	{
		m_pGRenderer->SetPenColor(0, fcyColor(0xFF00FFFF));
		m_pGRenderer->SetPenColor(1, fcyColor(0xFF00FFFF));
		m_pGRenderer->SetPenColor(2, fcyColor(0xFF00FFFF));
		m_pGRenderer->SetPenColor(3, fcyColor(0xFF00FFFF));
		m_pGRenderer->SetPenSize(3.f);
		m_pGRenderer->DrawCircle(m_pGraph2D, DrawPos, 2.f, 12);
		m_pGRenderer->SetPenColor(0, fcyColor(0xFF00FF00));
		m_pGRenderer->SetPenColor(1, fcyColor(0xFF00FF00));
		m_pGRenderer->SetPenColor(2, fcyColor(0xFF00FF00));
		m_pGRenderer->SetPenColor(3, fcyColor(0xFF00FF00));
		m_pGRenderer->DrawLine(m_pGraph2D, DrawPos, DrawPos + Adv);
		return true;
	}
	void OnGlyphCalcuCoord(f2dGraphics2DVertex pVerts[]) { }
public:
	FontBaseLineDebugHelper(f2dGraphics2D* G, f2dGeometryRenderer* GR, fcyRect BaseRect)
		: m_pGraph2D(G), m_pGRenderer(GR)
	{
		m_pGRenderer->SetPenColor(0, fcyColor(0xFFFF0000));
		m_pGRenderer->SetPenColor(1, fcyColor(0xFFFF0000));
		m_pGRenderer->SetPenColor(2, fcyColor(0xFFFF0000));
		m_pGRenderer->SetPenColor(3, fcyColor(0xFFFF0000));
		m_pGRenderer->SetPenSize(2.f);
		m_pGRenderer->DrawRectangle(G, BaseRect);
	}
};
#endif

////////////////////////////////////////////////////////////////////////////////
/// VKCode F2DKeyCode ת����
////////////////////////////////////////////////////////////////////////////////
const F2DINPUTKEYCODE VKCodeToF2DKeyCodeTable[256] =
{
	F2DINPUTKEYCODE_UNKNOWN,
	F2DINPUTKEYCODE_UNKNOWN,    // 1 VK_LBUTTON
	F2DINPUTKEYCODE_UNKNOWN,    // 2 VK_RBUTTON
	F2DINPUTKEYCODE_UNKNOWN,    // 3 VK_CANCEL
	F2DINPUTKEYCODE_UNKNOWN,    // 4 VK_MBUTTON
	F2DINPUTKEYCODE_UNKNOWN,    // 5 VK_XBUTTON1
	F2DINPUTKEYCODE_UNKNOWN,    // 6 VK_XBUTTON2
	F2DINPUTKEYCODE_UNKNOWN,    // 7
	F2DINPUTKEYCODE_BACK,       // 8 VK_BACK = Backspace
	F2DINPUTKEYCODE_TAB,        // 9 VK_TAB = Tab
	F2DINPUTKEYCODE_UNKNOWN,    // 10
	F2DINPUTKEYCODE_UNKNOWN,    // 11
	F2DINPUTKEYCODE_UNKNOWN,    // 12 VK_CLEAR = Clear
	F2DINPUTKEYCODE_RETURN,     // 13 VK_RETURN = Enter
	F2DINPUTKEYCODE_UNKNOWN,    // 14
	F2DINPUTKEYCODE_UNKNOWN,    // 15
	F2DINPUTKEYCODE_LSHIFT,     // 16 VK_SHIFT = Shift
	F2DINPUTKEYCODE_LCONTROL,   // 17 VK_CONTROL = Ctrl
	F2DINPUTKEYCODE_LMENU,      // 18 VK_MENU = Alt
	F2DINPUTKEYCODE_PAUSE,      // 19 VK_PAUSE = Pause
	F2DINPUTKEYCODE_CAPITAL,    // 20 VK_CAPITAL = Caps Lock
	F2DINPUTKEYCODE_KANA,       // 21 VK_KANA
	F2DINPUTKEYCODE_UNKNOWN,    // 22
	F2DINPUTKEYCODE_UNKNOWN,    // 23 VK_JUNJA
	F2DINPUTKEYCODE_UNKNOWN,    // 24 VK_FINAL
	F2DINPUTKEYCODE_KANJI,      // 25 VK_HANJA
	F2DINPUTKEYCODE_UNKNOWN,    // 26
	F2DINPUTKEYCODE_ESCAPE,     // 27 VK_ESCAPE = Esc
	F2DINPUTKEYCODE_CONVERT,    // 28 VK_CONVERT
	F2DINPUTKEYCODE_NOCONVERT,  // 29 VK_NONCONVERT
	F2DINPUTKEYCODE_UNKNOWN,    // 30 VK_ACCEPT
	F2DINPUTKEYCODE_UNKNOWN,    // 31 VK_MODECHANGE
	F2DINPUTKEYCODE_SPACE,      // 32 VK_SPACE = Space
	F2DINPUTKEYCODE_PRIOR,      // 33 VK_PRIOR = Page Up
	F2DINPUTKEYCODE_NEXT,       // 34 VK_NEXT = Page Down
	F2DINPUTKEYCODE_END,        // 35 VK_END = End
	F2DINPUTKEYCODE_HOME,       // 36 VK_HOME = Home
	F2DINPUTKEYCODE_LEFT,       // 37 VK_LEFT = Left Arrow
	F2DINPUTKEYCODE_UP,         // 38 VK_UP	= Up Arrow
	F2DINPUTKEYCODE_RIGHT,      // 39 VK_RIGHT = Right Arrow
	F2DINPUTKEYCODE_DOWN,       // 40 VK_DOWN = Down Arrow
	F2DINPUTKEYCODE_UNKNOWN,    // 41 VK_SELECT = Select
	F2DINPUTKEYCODE_UNKNOWN,    // 42 VK_PRINT = Print
	F2DINPUTKEYCODE_UNKNOWN,    // 43 VK_EXECUTE = Execute
	F2DINPUTKEYCODE_UNKNOWN,    // 44 VK_SNAPSHOT = Snapshot
	F2DINPUTKEYCODE_INSERT,     // 45 VK_INSERT = Insert
	F2DINPUTKEYCODE_DELETE,     // 46 VK_DELETE = Delete
	F2DINPUTKEYCODE_UNKNOWN,    // 47 VK_HELP = Help
	F2DINPUTKEYCODE_0,          // 48 0
	F2DINPUTKEYCODE_1,          // 49 1
	F2DINPUTKEYCODE_2,          // 50 2
	F2DINPUTKEYCODE_3,          // 51 3
	F2DINPUTKEYCODE_4,          // 52 4
	F2DINPUTKEYCODE_5,          // 53 5
	F2DINPUTKEYCODE_6,          // 54 6
	F2DINPUTKEYCODE_7,          // 55 7
	F2DINPUTKEYCODE_8,          // 56 8
	F2DINPUTKEYCODE_9,          // 57 9
	F2DINPUTKEYCODE_UNKNOWN,    // 58
	F2DINPUTKEYCODE_UNKNOWN,    // 59
	F2DINPUTKEYCODE_UNKNOWN,    // 60
	F2DINPUTKEYCODE_UNKNOWN,    // 61
	F2DINPUTKEYCODE_UNKNOWN,    // 62
	F2DINPUTKEYCODE_UNKNOWN,    // 63
	F2DINPUTKEYCODE_UNKNOWN,    // 64
	F2DINPUTKEYCODE_A,          // 65 A
	F2DINPUTKEYCODE_B,          // 66 B
	F2DINPUTKEYCODE_C,          // 67 C
	F2DINPUTKEYCODE_D,          // 68 D
	F2DINPUTKEYCODE_E,          // 69 E
	F2DINPUTKEYCODE_F,          // 70 F
	F2DINPUTKEYCODE_G,          // 71 G
	F2DINPUTKEYCODE_H,          // 72 H
	F2DINPUTKEYCODE_I,          // 73 I
	F2DINPUTKEYCODE_J,          // 74 J
	F2DINPUTKEYCODE_K,          // 75 K
	F2DINPUTKEYCODE_L,          // 76 L
	F2DINPUTKEYCODE_M,          // 77 M
	F2DINPUTKEYCODE_N,          // 78 N
	F2DINPUTKEYCODE_O,          // 79 O
	F2DINPUTKEYCODE_P,          // 80 P
	F2DINPUTKEYCODE_Q,          // 81 Q
	F2DINPUTKEYCODE_R,          // 82 R
	F2DINPUTKEYCODE_S,          // 83 S
	F2DINPUTKEYCODE_T,          // 84 T
	F2DINPUTKEYCODE_U,          // 85 U
	F2DINPUTKEYCODE_V,          // 86 V
	F2DINPUTKEYCODE_W,          // 87 W
	F2DINPUTKEYCODE_X,          // 88 X
	F2DINPUTKEYCODE_Y,          // 89 Y
	F2DINPUTKEYCODE_Z,          // 90 Z
	F2DINPUTKEYCODE_LWIN,       // 91 VK_LWIN
	F2DINPUTKEYCODE_RWIN,       // 92 VK_RWIN
	F2DINPUTKEYCODE_APPS,       // 93 VK_APPS
	F2DINPUTKEYCODE_UNKNOWN,    // 94
	F2DINPUTKEYCODE_SLEEP,      // 95 VK_SLEEP
	F2DINPUTKEYCODE_NUMPAD0,    // 96 VK_NUMPAD0 = С���� 0
	F2DINPUTKEYCODE_NUMPAD1,    // 97 VK_NUMPAD1 = С���� 1
	F2DINPUTKEYCODE_NUMPAD2,    // 98 VK_NUMPAD2 = С���� 2
	F2DINPUTKEYCODE_NUMPAD3,    // 99 VK_NUMPAD3 = С���� 3
	F2DINPUTKEYCODE_NUMPAD4,    // 100 VK_NUMPAD4 = С���� 4
	F2DINPUTKEYCODE_NUMPAD5,    // 101 VK_NUMPAD5 = С���� 5
	F2DINPUTKEYCODE_NUMPAD6,    // 102 VK_NUMPAD6 = С���� 6
	F2DINPUTKEYCODE_NUMPAD7,    // 103 VK_NUMPAD7 = С���� 7
	F2DINPUTKEYCODE_NUMPAD8,    // 104 VK_NUMPAD8 = С���� 8
	F2DINPUTKEYCODE_NUMPAD9,    // 105 VK_NUMPAD9 = С���� 9
	F2DINPUTKEYCODE_MULTIPLY,   // 106 VK_MULTIPLY = С���� *
	F2DINPUTKEYCODE_ADD,        // 107 VK_ADD = С���� +
	F2DINPUTKEYCODE_NUMPADENTER,// 108 VK_SEPARATOR = С���� Enter
	F2DINPUTKEYCODE_SUBTRACT,   // 109 VK_SUBTRACT = С���� -
	F2DINPUTKEYCODE_DECIMAL,    // 110 VK_DECIMAL = С���� .
	F2DINPUTKEYCODE_DIVIDE,     // 111 VK_DIVIDE = С���� /
	F2DINPUTKEYCODE_F1,         // 112 VK_F1 = F1
	F2DINPUTKEYCODE_F2,         // 113 VK_F2 = F2
	F2DINPUTKEYCODE_F3,         // 114 VK_F3 = F3
	F2DINPUTKEYCODE_F4,         // 115 VK_F4 = F4
	F2DINPUTKEYCODE_F5,         // 116 VK_F5 = F5
	F2DINPUTKEYCODE_F6,         // 117 VK_F6 = F6
	F2DINPUTKEYCODE_F7,         // 118 VK_F7 = F7
	F2DINPUTKEYCODE_F8,         // 119 VK_F8 = F8
	F2DINPUTKEYCODE_F9,         // 120 VK_F9 = F9
	F2DINPUTKEYCODE_F10,        // 121 VK_F10 = F10
	F2DINPUTKEYCODE_F11,        // 122 VK_F11 = F11
	F2DINPUTKEYCODE_F12,        // 123 VK_F12 = F12
	F2DINPUTKEYCODE_F13,        // 124 VK_F13
	F2DINPUTKEYCODE_F14,        // 125 VK_F14
	F2DINPUTKEYCODE_F15,        // 126 VK_F15
	F2DINPUTKEYCODE_UNKNOWN,    // 127 VK_F16
	F2DINPUTKEYCODE_UNKNOWN,    // 128 VK_F17
	F2DINPUTKEYCODE_UNKNOWN,    // 129 VK_F18
	F2DINPUTKEYCODE_UNKNOWN,    // 130 VK_F19
	F2DINPUTKEYCODE_UNKNOWN,    // 131 VK_F20
	F2DINPUTKEYCODE_UNKNOWN,    // 132 VK_F21
	F2DINPUTKEYCODE_UNKNOWN,    // 133 VK_F22
	F2DINPUTKEYCODE_UNKNOWN,    // 134 VK_F23
	F2DINPUTKEYCODE_UNKNOWN,    // 135 VK_F24
	F2DINPUTKEYCODE_UNKNOWN,    // 136
	F2DINPUTKEYCODE_UNKNOWN,    // 137
	F2DINPUTKEYCODE_UNKNOWN,    // 138
	F2DINPUTKEYCODE_UNKNOWN,    // 139
	F2DINPUTKEYCODE_UNKNOWN,    // 140
	F2DINPUTKEYCODE_UNKNOWN,    // 141
	F2DINPUTKEYCODE_UNKNOWN,    // 142
	F2DINPUTKEYCODE_UNKNOWN,    // 143
	F2DINPUTKEYCODE_NUMLOCK,    // 144 VK_NUMLOCK = Num Lock
	F2DINPUTKEYCODE_SCROLL,     // 145 VK_SCROLL = Scroll
	F2DINPUTKEYCODE_UNKNOWN,    // 146
	F2DINPUTKEYCODE_UNKNOWN,    // 147
	F2DINPUTKEYCODE_UNKNOWN,    // 148
	F2DINPUTKEYCODE_UNKNOWN,    // 149
	F2DINPUTKEYCODE_UNKNOWN,    // 150
	F2DINPUTKEYCODE_UNKNOWN,    // 151
	F2DINPUTKEYCODE_UNKNOWN,    // 152
	F2DINPUTKEYCODE_UNKNOWN,    // 153
	F2DINPUTKEYCODE_UNKNOWN,    // 154
	F2DINPUTKEYCODE_UNKNOWN,    // 155
	F2DINPUTKEYCODE_UNKNOWN,    // 156
	F2DINPUTKEYCODE_UNKNOWN,    // 157
	F2DINPUTKEYCODE_UNKNOWN,    // 158
	F2DINPUTKEYCODE_UNKNOWN,    // 159
	F2DINPUTKEYCODE_LSHIFT,     // 160 VK_LSHIFT
	F2DINPUTKEYCODE_RSHIFT,     // 161 VK_RSHIFT
	F2DINPUTKEYCODE_LCONTROL,   // 162 VK_LCONTROL
	F2DINPUTKEYCODE_RCONTROL,   // 163 VK_RCONTROL
	F2DINPUTKEYCODE_LMENU,      // 164 VK_LMENU
	F2DINPUTKEYCODE_RMENU,      // 165 VK_RMENU
	F2DINPUTKEYCODE_WEBBACK,    // 166 VK_BROWSER_BACK
	F2DINPUTKEYCODE_WEBFORWARD, // 167 VK_BROWSER_FORWARD
	F2DINPUTKEYCODE_WEBREFRESH, // 168 VK_BROWSER_REFRESH
	F2DINPUTKEYCODE_WEBSTOP,    // 169 VK_BROWSER_STOP
	F2DINPUTKEYCODE_WEBSEARCH,  // 170 VK_BROWSER_SEARCH
	F2DINPUTKEYCODE_WEBFAVORITES, // 171 VK_BROWSER_FAVORITES
	F2DINPUTKEYCODE_WEBHOME,    // 172 VK_BROWSER_HOME
	F2DINPUTKEYCODE_MUTE,       // 173 VK_VOLUME_MUTE
	F2DINPUTKEYCODE_VOLUMEDOWN, // 174 VK_VOLUME_DOWN
	F2DINPUTKEYCODE_VOLUMEUP,   // 175 VK_VOLUME_UP
	F2DINPUTKEYCODE_NEXTTRACK,  // 176 VK_MEDIA_NEXT_TRACK
	F2DINPUTKEYCODE_PREVTRACK,  // 177 VK_MEDIA_PREV_TRACK
	F2DINPUTKEYCODE_MEDIASTOP,  // 178 VK_MEDIA_STOP
	F2DINPUTKEYCODE_PLAYPAUSE,  // 179 VK_MEDIA_PLAY_PAUSE
	F2DINPUTKEYCODE_MAIL,       // 180 VK_LAUNCH_MAIL
	F2DINPUTKEYCODE_MEDIASELECT,// 181 VK_LAUNCH_MEDIA_SELECT
	F2DINPUTKEYCODE_UNKNOWN,    // 182 VK_LAUNCH_APP1
	F2DINPUTKEYCODE_UNKNOWN,    // 183 VK_LAUNCH_APP2
	F2DINPUTKEYCODE_UNKNOWN,    // 184
	F2DINPUTKEYCODE_UNKNOWN,    // 185
	F2DINPUTKEYCODE_SEMICOLON,  // 186 VK_OEM_1	= ; :
	F2DINPUTKEYCODE_EQUALS,     // 187 VK_OEM_PLUS = = +
	F2DINPUTKEYCODE_COMMA,      // 188 VK_OEM_COMMA = ,
	F2DINPUTKEYCODE_MINUS,      // 189 VK_OEM_MINUS = - _
	F2DINPUTKEYCODE_PERIOD,     // 190 VK_OEM_PERIOD = .
	F2DINPUTKEYCODE_SLASH,      // 191 VK_OEM_2 = / ?
	F2DINPUTKEYCODE_GRAVE,      // 192 VK_OEM_3 = ` ~
	F2DINPUTKEYCODE_UNKNOWN,    // 193
	F2DINPUTKEYCODE_UNKNOWN,    // 194
	F2DINPUTKEYCODE_UNKNOWN,    // 195
	F2DINPUTKEYCODE_UNKNOWN,    // 196
	F2DINPUTKEYCODE_UNKNOWN,    // 197
	F2DINPUTKEYCODE_UNKNOWN,    // 198
	F2DINPUTKEYCODE_UNKNOWN,    // 199
	F2DINPUTKEYCODE_UNKNOWN,    // 200
	F2DINPUTKEYCODE_UNKNOWN,    // 201
	F2DINPUTKEYCODE_UNKNOWN,    // 202
	F2DINPUTKEYCODE_UNKNOWN,    // 203
	F2DINPUTKEYCODE_UNKNOWN,    // 204
	F2DINPUTKEYCODE_UNKNOWN,    // 205
	F2DINPUTKEYCODE_UNKNOWN,    // 206
	F2DINPUTKEYCODE_UNKNOWN,    // 207
	F2DINPUTKEYCODE_UNKNOWN,    // 208
	F2DINPUTKEYCODE_UNKNOWN,    // 209
	F2DINPUTKEYCODE_UNKNOWN,    // 210
	F2DINPUTKEYCODE_UNKNOWN,    // 211
	F2DINPUTKEYCODE_UNKNOWN,    // 212
	F2DINPUTKEYCODE_UNKNOWN,    // 213
	F2DINPUTKEYCODE_UNKNOWN,    // 214
	F2DINPUTKEYCODE_UNKNOWN,    // 215
	F2DINPUTKEYCODE_UNKNOWN,    // 216
	F2DINPUTKEYCODE_UNKNOWN,    // 217
	F2DINPUTKEYCODE_UNKNOWN,    // 218
	F2DINPUTKEYCODE_LBRACKET,   // 219 VK_OEM_4 = [ {
	F2DINPUTKEYCODE_BACKSLASH,  // 220 VK_OEM_5 = \ |
	F2DINPUTKEYCODE_RBRACKET,   // 221 VK_OEM_6 = ] }
	F2DINPUTKEYCODE_APOSTROPHE, // 222 VK_OEM_7 = ' "
	F2DINPUTKEYCODE_UNKNOWN,    // 223 VK_OEM_8
	F2DINPUTKEYCODE_UNKNOWN,    // 224
	F2DINPUTKEYCODE_UNKNOWN,    // 225
	F2DINPUTKEYCODE_OEM_102,    // 226 VK_OEM_102
	F2DINPUTKEYCODE_UNKNOWN,    // 227
	F2DINPUTKEYCODE_UNKNOWN,    // 228
	F2DINPUTKEYCODE_UNKNOWN,    // 229 VK_PROCESSKEY
	F2DINPUTKEYCODE_UNKNOWN,    // 230
	F2DINPUTKEYCODE_UNKNOWN,    // 231 VK_PACKET
	F2DINPUTKEYCODE_UNKNOWN,    // 232
	F2DINPUTKEYCODE_UNKNOWN,    // 233
	F2DINPUTKEYCODE_UNKNOWN,    // 234
	F2DINPUTKEYCODE_UNKNOWN,    // 235
	F2DINPUTKEYCODE_UNKNOWN,    // 236
	F2DINPUTKEYCODE_UNKNOWN,    // 237
	F2DINPUTKEYCODE_UNKNOWN,    // 238
	F2DINPUTKEYCODE_UNKNOWN,    // 239
	F2DINPUTKEYCODE_UNKNOWN,    // 240
	F2DINPUTKEYCODE_UNKNOWN,    // 241
	F2DINPUTKEYCODE_UNKNOWN,    // 242
	F2DINPUTKEYCODE_UNKNOWN,    // 243
	F2DINPUTKEYCODE_UNKNOWN,    // 244
	F2DINPUTKEYCODE_UNKNOWN,    // 245
	F2DINPUTKEYCODE_UNKNOWN,    // 246 VK_ATTN
	F2DINPUTKEYCODE_UNKNOWN,    // 247 VK_CRSEL
	F2DINPUTKEYCODE_UNKNOWN,    // 248 VK_EXSEL
	F2DINPUTKEYCODE_UNKNOWN,    // 249 VK_EREOF
	F2DINPUTKEYCODE_UNKNOWN,    // 250 VK_PLAY
	F2DINPUTKEYCODE_UNKNOWN,    // 251 VK_ZOOM
	F2DINPUTKEYCODE_UNKNOWN,    // 252 VK_NONAME
	F2DINPUTKEYCODE_UNKNOWN,    // 253 VK_PA1	
	F2DINPUTKEYCODE_UNKNOWN,    // 254 VK_OEM_CLEAR
	F2DINPUTKEYCODE_UNKNOWN     // 255
};

static inline F2DINPUTKEYCODE VKKeyToF2DKey(fuInt VKCode)
{
	return VKCodeToF2DKeyCodeTable[VKCode];
}

////////////////////////////////////////////////////////////////////////////////
/// AppFrame
////////////////////////////////////////////////////////////////////////////////
LNOINLINE AppFrame& AppFrame::GetInstance()
{
	static AppFrame s_Instance;
	return s_Instance;
}

AppFrame::AppFrame()
{
}

AppFrame::~AppFrame()
{
	if (m_iStatus != AppStatus::NotInitialized && m_iStatus != AppStatus::Destroyed)
	{
		// ��û�����ٿ�ܣ���ִ������
		Shutdown();
	}
}

#pragma region �ű��ӿ�
LNOINLINE void AppFrame::ShowSplashWindow(const char* imgPath)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
	{
		try
		{
			Gdiplus::Image* pImg = nullptr;

			// ����ͼƬ�������
			if (imgPath)
			{
				std::vector<char> tDataBuf;
				if (m_ResourceMgr.LoadFile(imgPath, tDataBuf))
					pImg = SplashWindow::LoadImageFromMemory((fcData)tDataBuf.data(), tDataBuf.size());
				
				if (!pImg)
					LERROR("ShowSplashWindow: �޷�����ͼƬ'%m'", imgPath);
			}

			// ��ʾ����
			m_SplashWindow.ShowSplashWindow(pImg);

			FCYSAFEDEL(pImg);
		}
		catch (const bad_alloc&)
		{
			LERROR("ShowSplashWindow: �ڴ治��");
			return;
		}
		m_bSplashWindowEnabled = true;
	}
	else
		LWARNING("ShowSplashWindow: �޷��ڴ�ʱװ�ش���");
}

void AppFrame::SetWindowed(bool v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionWindowed = v;
	else if (m_iStatus == AppStatus::Running)
		LWARNING("��ͼ������ʱ���Ĵ��ڻ�ģʽ");
}

void AppFrame::SetFPS(fuInt v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionFPSLimit = v;
	else if (m_iStatus == AppStatus::Running)
		LWARNING("��ͼ������ʱ����FPS����");
}

void AppFrame::SetVsync(bool v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionVsync = v;
	else if (m_iStatus == AppStatus::Running)
		LWARNING("��ͼ������ʱ���Ĵ�ֱͬ��ģʽ");
}

void AppFrame::SetResolution(fuInt width, fuInt height)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionResolution.Set((float)width, (float)height);
	else if (m_iStatus == AppStatus::Running)
		LWARNING("��ͼ������ʱ���ķֱ���");
}

void AppFrame::SetSplash(bool v)LNOEXCEPT
{
	m_OptionSplash = v;
	if (m_pMainWindow)
		m_pMainWindow->HideMouse(!m_OptionSplash);
}

LNOINLINE void AppFrame::SetTitle(const char* v)LNOEXCEPT
{
	try
	{
		m_OptionTitle = std::move(fcyStringHelper::MultiByteToWideChar(v, CP_UTF8));
		if (m_pMainWindow)
			m_pMainWindow->SetCaption(m_OptionTitle.c_str());
	}
	catch (const bad_alloc&)
	{
		LERROR("�޸Ĵ��ڱ���ʱ�޷������ڴ�");
	}
}

LNOINLINE bool AppFrame::ChangeVideoMode(int width, int height, bool windowed, bool vsync)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initialized)
	{
		// �л����µ���Ƶѡ��
		if (FCYOK(m_pRenderDev->SetBufferSize(
			(fuInt)width,
			(fuInt)height,
			windowed,
			vsync,
			F2DAALEVEL_NONE)))
		{
			LINFO("��Ƶģʽ�л��ɹ� (%dx%d Vsync:%b Windowed:%b) -> (%dx%d Vsync:%b Windowed:%b)",
				(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed,
				width, height, vsync, windowed);

			m_OptionResolution.Set((float)width, (float)height);
			m_OptionWindowed = windowed;
			m_OptionVsync = vsync;

			// �л����ڴ�С
			m_pMainWindow->SetBorderType(m_OptionWindowed ? F2DWINBORDERTYPE_FIXED : F2DWINBORDERTYPE_NONE);
			m_pMainWindow->SetClientRect(
				fcyRect(10.f, 10.f, 10.f + m_OptionResolution.x, 10.f + m_OptionResolution.y)
			);
			m_pMainWindow->SetTopMost(!m_OptionWindowed);
			m_pMainWindow->MoveToCenter();
			return true;
		}
		else
		{
			// ! ����ʾ���ش��ڶ�������Ϸ���Ϊȫ��ʧ��ʱ�򽫴���ģʽ��Ϊtrue
			if (m_bSplashWindowEnabled)
				m_OptionWindowed = true;

			// �л����ڴ�С
			m_pMainWindow->SetBorderType(m_OptionWindowed ? F2DWINBORDERTYPE_FIXED : F2DWINBORDERTYPE_NONE);
			m_pMainWindow->SetClientRect(
				fcyRect(10.f, 10.f, 10.f + m_OptionResolution.x, 10.f + m_OptionResolution.y)
				);
			m_pMainWindow->SetTopMost(!m_OptionWindowed);
			m_pMainWindow->MoveToCenter();

			LINFO("��Ƶģʽ�л�ʧ�� (%dx%d Vsync:%b Windowed:%b) -> (%dx%d Vsync:%b Windowed:%b)",
				(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed,
				width, height, vsync, windowed);
		}
	}
	return false;
}

LNOINLINE void AppFrame::LoadScript(const char* path)LNOEXCEPT
{
	LINFO("װ�ؽű�'%m'", path);
	if (!m_ResourceMgr.LoadFile(path, m_TempBuffer))
	{
		luaL_error(L, "can't load script '%s'", path);
		return;
	}
	if (luaL_loadbuffer(L, m_TempBuffer.data(), m_TempBuffer.size(), luaL_checkstring(L, 1)))
	{
		const char* tDetail = lua_tostring(L, -1);
		LERROR("����ű�'%m'ʧ��: %m", path, tDetail);
		luaL_error(L, "failed to compile '%s': %s", path, tDetail);
		return;
	}
	if (lua_pcall(L, 0, 0, 0))
	{
		const char* tDetail = lua_tostring(L, -1);
		LERROR("ִ�нű�'%m'ʧ��", path);
		luaL_error(L, "failed to execute '%s':\n\t%s", path, tDetail);
		return;
	}
}

fBool AppFrame::GetKeyState(int VKCode)LNOEXCEPT
{
	if (VKCode > 0 && VKCode < _countof(m_KeyStateMap))
	{
		if (m_Keyboard)
			return m_Keyboard->IsKeyDown(VKKeyToF2DKey(VKCode));
		else
			return m_KeyStateMap[VKCode];
	}
	return false;
}

LNOINLINE int AppFrame::GetLastChar(lua_State* L)LNOEXCEPT
{
	if (m_LastChar)
	{
		try
		{
			fCharW tBuf[2] = { m_LastChar, 0 };
			lua_pushstring(L,
				fcyStringHelper::WideCharToMultiByte(tBuf, CP_UTF8).c_str());
		}
		catch (const bad_alloc&)
		{
			LERROR("GetLastChar: �ڴ治��");
			return 0;
		}
	}
	else
		lua_pushstring(L, "");
	return 1;
}

bool AppFrame::BeginScene()LNOEXCEPT
{
	if (!m_bRenderStarted)
	{
		LERROR("������RenderFunc����ĵط�ִ����Ⱦ");
		return false;
	}

	if (m_GraphType == GraphicsType::Graph2D)
	{
		if (FCYFAILED(m_Graph2D->Begin()))
		{
			LERROR("ִ��f2dGraphics2D::Beginʧ��");
			return false;
		}
	}

	return true;
}

bool AppFrame::EndScene()LNOEXCEPT
{
	if (m_GraphType == GraphicsType::Graph2D)
	{
		if (FCYFAILED(m_Graph2D->End()))
		{
			LERROR("ִ��f2dGraphics2D::Endʧ��");
			return false;
		}
	}

	return true;
}

void AppFrame::SetFog(float start, float end, fcyColor color)
{
	if (m_Graph2D->IsInRender())
		m_Graph2D->Flush();

	// ��f2dRenderDevice��ȡ��D3D�豸
	IDirect3DDevice9* pDev = (IDirect3DDevice9*)m_pRenderDev->GetHandle();

	if (start != end)
	{
		pDev->SetRenderState(D3DRS_FOGENABLE, TRUE);
		pDev->SetRenderState(D3DRS_FOGCOLOR, color.argb);
		if (start == -1.0f)
		{
			pDev->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP);
			pDev->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)(&end));
		}
		else if (start == -2.0f)
		{
			pDev->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP2);
			pDev->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)(&end));
		}
		else
		{
			pDev->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
			pDev->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&start));
			pDev->SetRenderState(D3DRS_FOGEND, *(DWORD *)(&end));
		}
	}
	else
		pDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
}

bool AppFrame::RenderText(ResFont* p, wchar_t* strBuf, fcyRect rect, fcyVec2 scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign, bool bWordBreak)LNOEXCEPT
{
	if (m_GraphType != GraphicsType::Graph2D)
	{
		LERROR("RenderText: ֻ��2D��Ⱦ������ִ�и÷���");
		return false;
	}

	f2dFontProvider* pFontProvider = p->GetFontProvider();

	// ׼����Ⱦ����
	m_FontRenderer->SetFontProvider(pFontProvider);
	m_FontRenderer->SetScale(scale);
#ifdef LSHOWFONTBASELINE
	FontBaseLineDebugHelper tDebugger(m_Graph2D, m_GRenderer, rect);
	m_FontRenderer->SetListener(&tDebugger);
#endif

	// ���û�Ϻ���ɫ
	updateGraph2DBlendMode(p->GetBlendMode());
	m_FontRenderer->SetColor(p->GetBlendColor());

	// ��һ�α�������Ҫ��Ⱦ������
	const wchar_t* pText = strBuf;
	int iLineCount = 1;
	float fLineWidth = 0.f;
	while (*pText)
	{
		bool bNewLine = false;
		if (*pText == L'\n')
			bNewLine = true;
		else
		{
			f2dGlyphInfo tGlyphInfo;
			if (FCYOK(pFontProvider->QueryGlyph(m_Graph2D, *pText, &tGlyphInfo)))
			{
				float adv = tGlyphInfo.Advance.x * scale.x;
				if (bWordBreak && fLineWidth + adv > rect.GetWidth())  // �ض�ģʽ
				{
					if (pText == strBuf || *(pText - 1) == L'\n')
					{
						++pText;  // ��ֹһ���ַ�������Ⱦ������ѭ��
						if (*pText == L'\0')
							break;
					}
					bNewLine = true;
				}
				else
					fLineWidth += adv;
			}
		}
		if (bNewLine)
		{
			++iLineCount;
			fLineWidth = 0.f;
		}
		if (*pText != L'\0')
			++pText;
	}
	
	// �������λ��
	float fTotalLineHeight = pFontProvider->GetLineHeight() * iLineCount * scale.y;
	fcyVec2 vRenderPos;
	switch (valign)
	{
	case ResFont::FontAlignVertical::Bottom:
		vRenderPos.y = rect.b.y + fTotalLineHeight;
		break;
	case ResFont::FontAlignVertical::Middle:
		vRenderPos.y = rect.a.y - rect.GetHeight() / 2.f + fTotalLineHeight / 2.f;
		break;
	case ResFont::FontAlignVertical::Top:
	default:
		vRenderPos.y = rect.a.y;
		break;
	}
	vRenderPos.x = rect.a.x;
	vRenderPos.y -= pFontProvider->GetAscender() * scale.y;

	// ������Ⱦ����
	wchar_t* pScanner = strBuf;
	wchar_t c = 0;
	bool bEOS = false;
	fLineWidth = 0.f;
	pText = pScanner;
	while (!bEOS)
	{
		// Ѱ�ҶϾ�λ�ã����С�EOF�����������
		while (*pScanner != L'\0' && *pScanner != '\n')
		{
			f2dGlyphInfo tGlyphInfo;
			if (FCYOK(pFontProvider->QueryGlyph(m_Graph2D, *pScanner, &tGlyphInfo)))
			{
				float adv = tGlyphInfo.Advance.x * scale.x;

				// ��鵱ǰ�ַ���Ⱦ��᲻�ᵼ�������
				if (bWordBreak && fLineWidth + adv > rect.GetWidth())
				{
					if (pScanner == pText)  // ��ֹһ���ַ�������Ⱦ������ѭ��
						++pScanner;
					break;
				}
				fLineWidth += adv;
			}
			++pScanner;
		}
		
		// �ڶϾ�λ��д��\0
		c = *pScanner;
		if (c == L'\0')
			bEOS = true;
		else
			*pScanner = L'\0';
		
		// ��Ⱦ��pText~pScanner������
		switch (halign)
		{
		case ResFont::FontAlignHorizontal::Right:
			m_FontRenderer->DrawTextW2(m_Graph2D, pText,
				fcyVec2(vRenderPos.x + rect.GetWidth() - fLineWidth, vRenderPos.y));
			break;
		case ResFont::FontAlignHorizontal::Center:
			m_FontRenderer->DrawTextW2(m_Graph2D, pText,
				fcyVec2(vRenderPos.x + rect.GetWidth() / 2.f - fLineWidth / 2.f, vRenderPos.y));
			break;
		case ResFont::FontAlignHorizontal::Left:
		default:
			m_FontRenderer->DrawTextW2(m_Graph2D, pText, vRenderPos);
			break;
		}

		// �ָ��Ͼ䴦�ַ�
		*pScanner = c;
		fLineWidth = 0.f;
		if (c == L'\n')
			pText = ++pScanner;
		else
			pText = pScanner;
		
		// �ƶ�y��
		vRenderPos.y -= p->GetFontProvider()->GetLineHeight() * scale.y;
	}

#ifdef LSHOWFONTBASELINE
	m_FontRenderer->SetListener(nullptr);
#endif
	return true;
}

fcyVec2 AppFrame::CalcuTextSize(ResFont* p, const wchar_t* strBuf, fcyVec2 scale)LNOEXCEPT
{
	if (m_GraphType != GraphicsType::Graph2D)
	{
		LERROR("RenderText: ֻ��2D��Ⱦ������ִ�и÷���");
		return false;
	}

	f2dFontProvider* pFontProvider = p->GetFontProvider();

	int iLineCount = 1;
	float fLineWidth = 0.f;
	float fMaxLineWidth = 0.f;
	while (*strBuf)
	{
		if (*strBuf == L'\n')
		{
			++iLineCount;
			fMaxLineWidth = max(fMaxLineWidth, fLineWidth);
			fLineWidth = 0.f;
		}
		else
		{
			f2dGlyphInfo tGlyphInfo;
			if (FCYOK(pFontProvider->QueryGlyph(m_Graph2D, *strBuf, &tGlyphInfo)))
				fLineWidth += tGlyphInfo.Advance.x * scale.x;
		}
		++strBuf;
	}
	fMaxLineWidth = max(fMaxLineWidth, fLineWidth);

	return fcyVec2(fMaxLineWidth, iLineCount * pFontProvider->GetLineHeight() * scale.y);
}

LNOINLINE bool AppFrame::RenderText(const char* name, const char* str, float x, float y, float scale, ResFont::FontAlignHorizontal halign, ResFont::FontAlignVertical valign)LNOEXCEPT
{
	fcyRefPointer<ResFont> p = m_ResourceMgr.FindSpriteFont(name);
	if (!p)
	{
		LERROR("RenderText: �Ҳ���������Դ'%m'", name);
		return false;
	}

	// ����ת��
	static std::wstring s_TempStringBuf;
	try
	{
		Utf8ToUtf16(str, s_TempStringBuf);
	}
	catch (const bad_alloc&)
	{
		LERROR("RenderText: �ڴ治��");
		return false;
	}

	// ������Ⱦλ��
	fcyVec2 tSize = CalcuTextSize(p, s_TempStringBuf.c_str(), fcyVec2(scale, scale));
	switch (halign)
	{
	case ResFont::FontAlignHorizontal::Right:
		x -= tSize.x;
		break;
	case ResFont::FontAlignHorizontal::Center:
		x -= tSize.x / 2.f;
		break;
	case ResFont::FontAlignHorizontal::Left:
	default:
		break;
	}
	switch (valign)
	{
	case ResFont::FontAlignVertical::Bottom:
		y += tSize.y;
		break;
	case ResFont::FontAlignVertical::Middle:
		y += tSize.y / 2.f;
		break;
	case ResFont::FontAlignVertical::Top:
	default:
		break;
	}

	return RenderText(
		p,
		const_cast<wchar_t*>(s_TempStringBuf.data()),
		fcyRect(x, y, x + tSize.x, y - tSize.y),
		fcyVec2(scale, scale),
		halign,
		valign,
		false
		);
}

LNOINLINE bool AppFrame::RenderTTF(const char* name, const char* str, float left, float right, float bottom, float top, float scale, int format, fcyColor c)LNOEXCEPT
{
	fcyRefPointer<ResFont> p = m_ResourceMgr.FindTTFFont(name);
	if (!p)
	{
		LERROR("RenderTTF: �Ҳ���������Դ'%m'", name);
		return false;
	}

	// ����ת��
	static std::wstring s_TempStringBuf;
	try
	{
		Utf8ToUtf16(str, s_TempStringBuf);
	}
	catch (const bad_alloc&)
	{
		LERROR("RenderTTF: �ڴ治��");
		return false;
	}

	p->SetBlendColor(c);

	bool bWordBreak = false;
	ResFont::FontAlignHorizontal halign = ResFont::FontAlignHorizontal::Left;
	ResFont::FontAlignVertical valign = ResFont::FontAlignVertical::Top;

	if ((format & DT_CENTER) == DT_CENTER)
		halign = ResFont::FontAlignHorizontal::Center;
	else if ((format & DT_RIGHT) == DT_RIGHT)
		halign = ResFont::FontAlignHorizontal::Right;

	if ((format & DT_VCENTER) == DT_VCENTER)
		valign = ResFont::FontAlignVertical::Middle;
	else if ((format & DT_BOTTOM) == DT_BOTTOM)
		valign = ResFont::FontAlignVertical::Bottom;

	if ((format & DT_WORDBREAK) == DT_WORDBREAK)
		bWordBreak = true;

	return RenderText(
		p,
		const_cast<wchar_t*>(s_TempStringBuf.data()),
		fcyRect(left, top, right, bottom),
		fcyVec2(scale, scale) * 0.5f,  // ����ϵ��=0.5
		halign,
		valign,
		bWordBreak
		);
}

LNOINLINE void AppFrame::SnapShot(const char* path)LNOEXCEPT
{
	LASSERT(m_pRenderDev);

	try
	{
		fcyRefPointer<fcyFileStream> tOutputFile;
		tOutputFile.DirectSet(new fcyFileStream(fcyStringHelper::MultiByteToWideChar(path, CP_UTF8).c_str(), true));
		tOutputFile->SetLength(0);

		if (FCYFAILED(m_pRenderDev->SaveScreen(tOutputFile)))
			LERROR("Snapshot: �����ͼ��'%m'ʧ��", path);
	}
	catch (const bad_alloc&)
	{
		LERROR("Snapshot: �ڴ治��");
	}
	catch (const fcyException& e)
	{
		LERROR("Snapshot: �����ͼʧ�� (�쳣��Ϣ'%m' Դ'%m')", e.GetDesc(), e.GetSrc());
	}
}

LNOINLINE bool AppFrame::PostEffectCapture()LNOEXCEPT
{
	if (!m_bRenderStarted || m_bPostEffectCaptureStarted)
	{
		LERROR("PostEffectCapture: �Ƿ�����");
		return false;
	}

	fcyRect orgVP = m_pRenderDev->GetViewport();
	if (FCYFAILED(m_pRenderDev->SetRenderTarget(m_PostEffectBuffer)))
	{
		LERROR("PostEffectCapture: �ڲ����� (f2dRenderDevice::SetRenderTarget failed.)");
		return false;
	}
	m_pRenderDev->SetViewport(orgVP);

	m_pRenderDev->ClearColor();
	m_bPostEffectCaptureStarted = true;
	return true;
}

LNOINLINE bool AppFrame::PostEffectApply(ResFX* shader, BlendMode blend)LNOEXCEPT
{
	f2dEffectTechnique* pTechnique = shader->GetEffect()->GetTechnique(0U);
	if (!m_bRenderStarted || !m_bPostEffectCaptureStarted || !pTechnique)
	{
		LERROR("PostEffectApply: �Ƿ�����");
		return false;
	}
	
	// ��ֹ��Ⱦ����
	bool bRestartRenderPeriod = false;
	switch (m_GraphType)
	{
	case GraphicsType::Graph2D:
		if (m_Graph2D->IsInRender())
		{
			bRestartRenderPeriod = true;
			m_Graph2D->End();
		}
		break;
	case GraphicsType::Graph3D:
		if (m_Graph3D->IsInRender())
		{
			bRestartRenderPeriod = true;
			m_Graph3D->End();
		}
		break;
	}

	// �л�RenderTarget
	fcyRect orgVP = m_pRenderDev->GetViewport();
	if (FCYFAILED(m_pRenderDev->SetRenderTarget(nullptr)))
	{
		// �� �쳣�˳����ɻָ���Ⱦ����
		LERROR("PostEffectCapture: �ڲ����� (f2dRenderDevice::SetRenderTarget failed)");
		m_bPostEffectCaptureStarted = false;
		return false;
	}
	m_pRenderDev->SetViewport(orgVP);

	// ������Ⱦ״̬
	updateGraph3DBlendMode(blend);

	// ����effect
	shader->SetPostEffectTexture(m_PostEffectBuffer);
	shader->SetViewport(orgVP);
	shader->SetScreenSize(fcyVec2((float)m_pRenderDev->GetBufferWidth(), (float)m_pRenderDev->GetBufferHeight()));
	m_Graph3D->SetEffect(shader->GetEffect());
	if (FCYFAILED(m_Graph3D->Begin()))
	{
		// �� �쳣�˳����ɻָ���Ⱦ����
		LERROR("PostEffectCapture: �ڲ����� (f2dGraphics3D::Begin failed)");
		m_bPostEffectCaptureStarted = false;
		return false;
	}
	// ִ�����е�pass
	for (fuInt i = 0; i < pTechnique->GetPassCount(); ++i)
	{
		m_Graph3D->BeginPass(i);
		m_Graph3D->RenderPostEffect();
		m_Graph3D->EndPass();
	}
	m_Graph3D->End();
	shader->SetPostEffectTexture(NULL);
	
	// ������Ⱦ����
	if (bRestartRenderPeriod)
	{
		switch (m_GraphType)
		{
		case GraphicsType::Graph2D:
			m_Graph2D->Begin();
			break;
		case GraphicsType::Graph3D:
			m_Graph3D->Begin();
			break;
		}
	}

	m_bPostEffectCaptureStarted = false;
	return true;
}

#pragma endregion

#pragma region ��ܺ���
bool AppFrame::Init()LNOEXCEPT
{
	LASSERT(m_iStatus == AppStatus::NotInitialized);

	LINFO("��ʼ��ʼ�� �汾: %s", LVERSION);
	m_iStatus = AppStatus::Initializing;

	Scope tSplashWindowExit([this]() {
		m_SplashWindow.HideSplashWindow();
	});

	//////////////////////////////////////// Lua��ʼ������
	LINFO("��ʼ��ʼ��Lua����� �汾: %m", LVERSION_LUA);
	L = lua_open();
	if (!L)
	{
		LERROR("�޷���ʼ��Lua�����");
		return false;
	}
	if (0 != luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON))
		LWARNING("�޷�����JITģʽ");

	luaL_openlibs(L);  // �ڽ���
	luaopen_lfs(L);  // �ļ�ϵͳ��
	luaopen_cjson(L);  // CJSON��
	ColorWrapper::Register(L);  // ��ɫ����
	RandomizerWrapper::Register(L);  // �����������
	BentLaserDataWrapper::Register(L);  // ���߼���
	BuiltInFunctionWrapper::Register(L);  // �ڽ�������
	
	// Ϊ����ط���ռ�
	LINFO("��ʼ������� ����=%u", LGOBJ_MAXCNT);
	try
	{
		m_GameObjectPool = make_unique<GameObjectPool>(L);
	}
	catch (const bad_alloc&)
	{
		LERROR("�޷�Ϊ����ط����㹻�ڴ�");
		return false;
	}

	// ���������в���
	lua_getglobal(L, "lstg");  // t
	lua_newtable(L);  // t t
	for (int i = 0; i < __argc; ++i)
	{
		lua_pushnumber(L, i + 1);  // t t i
		lua_pushstring(L, __argv[i]);  // t t i s
		lua_settable(L, -3);  // t t
	}
	lua_setfield(L, -2, "args");  // t
	lua_pop(L, 1);

	//////////////////////////////////////// װ�س�ʼ���ű�
	LINFO("װ�س�ʼ���ű�'%s'", LLAUNCH_SCRIPT);
	if (!m_ResourceMgr.LoadFile(LLAUNCH_SCRIPT, m_TempBuffer))
		return false;
	if (!SafeCallScript(m_TempBuffer.data(), m_TempBuffer.size(), "launch"))
		return false;
	
	//////////////////////////////////////// ��ʼ��fancy2d����
	LINFO("��ʼ��fancy2d �汾 %d.%d (�ֱ���: %dx%d ��ֱͬ��: %b ���ڻ�: %b)",
		(F2DVERSION & 0xFFFF0000) >> 16, F2DVERSION & 0x0000FFFF,
		(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed);
	struct : public f2dInitialErrListener
	{
		void OnErr(fuInt TimeTick, fcStr Src, fcStr Desc)
		{
			LERROR("��ʼ��fancy2dʧ�� (�쳣��Ϣ'%m' Դ'%m')", Desc, Src);
		}
	} tErrListener;

	if (FCYFAILED(CreateF2DEngineAndInit(
		F2DVERSION,
		fcyRect(0.f, 0.f, m_OptionResolution.x, m_OptionResolution.y),
		m_OptionTitle.c_str(),
		m_bSplashWindowEnabled ? true : m_OptionWindowed,
		m_OptionVsync,
		F2DAALEVEL_NONE,
		this,
		&m_pEngine,
		&tErrListener
		)))
	{
		return false;
	}

	// ��ȡ���
	m_pMainWindow = m_pEngine->GetMainWindow();
	m_pRenderer = m_pEngine->GetRenderer();
	m_pRenderDev = m_pRenderer->GetDevice();
	m_pSoundSys = m_pEngine->GetSoundSys();
	m_pInputSys = m_pEngine->GetInputSys();

	// ������Ⱦ��
	if (FCYFAILED(m_pRenderDev->CreateGraphics2D(1024, 2048, &m_Graph2D)))
	{
		LERROR("�޷�������Ⱦ�� (fcyRenderDevice::CreateGraphics2D failed)");
		return false;
	}
	m_Graph2DLastBlendMode = BlendMode::AddAlpha;
	m_Graph2DBlendState = m_Graph2D->GetBlendState();
	m_Graph2DColorBlendState = m_Graph2D->GetColorBlendType();
	m_bRenderStarted = false;

	// ����������Ⱦ��
	if (FCYFAILED(m_pRenderer->CreateFontRenderer(nullptr, &m_FontRenderer)))
	{
		LERROR("�޷�����������Ⱦ�� (fcyRenderer::CreateFontRenderer failed)");
		return false;
	}
	m_FontRenderer->SetZ(0.5f);

	// ����ͼԪ��Ⱦ��
	if (FCYFAILED(m_pRenderer->CreateGeometryRenderer(&m_GRenderer)))
	{
		LERROR("�޷�����ͼԪ��Ⱦ�� (fcyRenderer::CreateGeometryRenderer failed)");
		return false;
	}

	// ����3D��Ⱦ��
	if (FCYFAILED(m_pRenderDev->CreateGraphics3D(nullptr, &m_Graph3D)))
	{
		LERROR("�޷�����3D��Ⱦ�� (fcyRenderDevice::CreateGraphics3D failed)");
		return false;
	}
	m_Graph3DLastBlendMode = BlendMode::AddAlpha;
	m_Graph3DBlendState = m_Graph3D->GetBlendState();
	
	// ����PostEffect����
	if (FCYFAILED(m_pRenderDev->CreateRenderTarget(
		m_pRenderDev->GetBufferWidth(),
		m_pRenderDev->GetBufferHeight(),
		true,
		&m_PostEffectBuffer)))
	{
		LERROR("�޷�����POSTEFFECT������ (fcyRenderDevice::CreateRenderTarget failed)");
		return false;
	}

	// ������������
	m_pInputSys->CreateKeyboard(-1, false, &m_Keyboard);
	if (!m_Keyboard)
		LWARNING("�޷����������豸����ʹ�ô�����Ϣ��Ϊ����Դ (f2dInputSys::CreateKeyboard failed.)");
	
	// luastg��ʹ��ZBuffer������رա�
	m_pRenderDev->SetZBufferEnable(false);
	
	// ���ô���ͼ��
	HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON));
	SendMessage((HWND)m_pMainWindow->GetHandle(), WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
	SendMessage((HWND)m_pMainWindow->GetHandle(), WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
	DestroyIcon(hIcon);

	// ��û�����봰�ڣ�����ʾ��Ϸ����
	if (!m_bSplashWindowEnabled)
	{
		// ��ʾ����
		m_pMainWindow->MoveToCenter();
		m_pMainWindow->SetVisiable(true);
	}

	m_LastChar = 0;
	m_LastKey = 0;
	memset(m_KeyStateMap, 0, sizeof(m_KeyStateMap));

	//////////////////////////////////////// װ�غ��Ľű���ִ��GameInit
	LINFO("װ�غ��Ľű�'%s'", LCORE_SCRIPT);
	if (!m_ResourceMgr.LoadFile(LCORE_SCRIPT, m_TempBuffer))
		return false;
	if (!SafeCallScript(m_TempBuffer.data(), m_TempBuffer.size(), "core.lua"))
		return false;
	if (!SafeCallGlobalFunction(LFUNC_GAMEINIT))
		return false;

	m_iStatus = AppStatus::Initialized;
	LINFO("��ʼ���ɹ����");
	return true;
}

void AppFrame::Shutdown()LNOEXCEPT
{
	m_GameObjectPool = nullptr;
	LINFO("����ն����");

	m_ResourceMgr.ClearAllResource();
	LINFO("�����������Դ");

	m_Keyboard = nullptr;
	m_PostEffectBuffer = nullptr;
	m_Graph3D = nullptr;
	m_GRenderer = nullptr;
	m_FontRenderer = nullptr;
	m_Graph2D = nullptr;
	m_pInputSys = nullptr;
	m_pSoundSys = nullptr;
	m_pRenderDev = nullptr;
	m_pRenderer = nullptr;
	m_pMainWindow = nullptr;
	m_pEngine = nullptr;
	LINFO("��ж��fancy2d");

	if (L)
	{
		lua_close(L);
		L = nullptr;
		LINFO("��ж��Lua�����");
	}
	m_ResourceMgr.UnloadAllPack();
	LINFO("��ж��������Դ��");

	m_iStatus = AppStatus::Destroyed;
	LINFO("�������");
}

void AppFrame::Run()LNOEXCEPT
{
	LASSERT(m_iStatus == AppStatus::Initialized);
	LINFO("��ʼִ����Ϸѭ��");

	if (m_bSplashWindowEnabled)  // ��ʾ�����봰��
	{
		// ��ʾ����
		m_pMainWindow->MoveToCenter();
		m_pMainWindow->SetVisiable(true);

		// �ı���ʾģʽ��ȫ��
		if (!m_OptionWindowed)
			ChangeVideoMode((int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionWindowed, m_OptionVsync);
	}
	m_bSplashWindowEnabled = false;

	// ����ǰ�ơ���ʾ���������ָ��
	SetWindowPos((HWND)m_pMainWindow->GetHandle(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	m_pMainWindow->HideMouse(!m_OptionSplash);

	// ������Ϸѭ��
	m_pEngine->Run(F2DENGTHREADMODE_MULTITHREAD, m_OptionFPSLimit);

	LINFO("�˳���Ϸѭ��");
}

bool AppFrame::SafeCallScript(const char* source, size_t len, const char* desc)LNOEXCEPT
{
	if (0 != luaL_loadbuffer(L, source, len, desc))
	{
		try
		{
			wstring tErrorInfo = StringFormat(
				L"�ű�'%m'����ʧ��: %m",
				desc,
				lua_tostring(L, -1)
			);

			LERROR("�ű�����%s", tErrorInfo.c_str());
			MessageBox(
				m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
				tErrorInfo.c_str(),
				L"LuaSTGPlus�ű�����",
				MB_ICONERROR | MB_OK
			);
		}
		catch (const bad_alloc&)
		{
			LERROR("����д���ű�����ʱ�����ڴ治�����");
		}
		
		lua_pop(L, 1);
		return false;
	}

	if (0 != lua_pcall(L, 0, 0, 0))
	{
		try
		{
			wstring tErrorInfo = StringFormat(
				L"�ű�'%m'�в���δ���������ʱ����:\n\t%m",
				desc,
				lua_tostring(L, -1)
			);

			LERROR("�ű�����%s", tErrorInfo.c_str());
			MessageBox(
				m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
				tErrorInfo.c_str(),
				L"LuaSTGPlus�ű�����",
				MB_ICONERROR | MB_OK
			);
		}
		catch (const bad_alloc&)
		{
			LERROR("����д���ű�����ʱ�����ڴ治�����");
		}

		lua_pop(L, 1);
		return false;
	}

	return true;
}

bool AppFrame::SafeCallGlobalFunction(const char* name, int argc, int retc)LNOEXCEPT
{
	lua_getglobal(L, name);
	if (0 != lua_pcall(L, argc, retc, 0))
	{
		try
		{
			wstring tErrorInfo = StringFormat(
				L"ִ�к���'%m'ʱ����δ���������ʱ����:\n\t%m",
				name,
				lua_tostring(L, -1)
			);

			LERROR("�ű�����%s", tErrorInfo.c_str());
			MessageBox(
				m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
				tErrorInfo.c_str(),
				L"LuaSTGPlus�ű�����",
				MB_ICONERROR | MB_OK
			);
		}
		catch (const bad_alloc&)
		{
			LERROR("����д���ű�����ʱ�����ڴ治�����");
		}

		lua_pop(L, 1);
		return false;
	}

	return true;
}
#pragma endregion

#pragma region ��Ϸѭ��
fBool AppFrame::OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump)
{
	m_fFPS = (float)pFPSController->GetFPS();

	m_LastChar = 0;
	m_LastKey = 0;

	// ������Ϣ
	f2dMsg tMsg;
	while (FCYOK(pMsgPump->GetMsg(&tMsg)))
	{
		switch (tMsg.Type)
		{
		case F2DMSG_WINDOW_ONCLOSE:
			return false;  // �رմ���ʱ����ѭ��
		case F2DMSG_WINDOW_ONGETFOCUS:
			if (!SafeCallGlobalFunction(LFUNC_GAINFOCUS))
				return false;
		case F2DMSG_WINDOW_ONLOSTFOCUS:
			if (!SafeCallGlobalFunction(LFUNC_LOSEFOCUS))
				return false;
		case F2DMSG_WINDOW_ONCHARINPUT:
			m_LastChar = (fCharW)tMsg.Param1;
			break;
		case F2DMSG_WINDOW_ONKEYDOWN:
			// ctrl+enterȫ��
			if (tMsg.Param1 == VK_RETURN && !m_KeyStateMap[VK_RETURN] && m_KeyStateMap[VK_CONTROL])  // ��ֹ��������
				ChangeVideoMode((int)m_OptionResolution.x, (int)m_OptionResolution.y, !m_OptionWindowed, m_OptionVsync);

			m_LastKey = (fInt)tMsg.Param1;
			if (0 < tMsg.Param1 && tMsg.Param1 < _countof(m_KeyStateMap))
				m_KeyStateMap[tMsg.Param1] = true;
			break;
		case F2DMSG_WINDOW_ONKEYUP:
			if (m_LastKey == tMsg.Param1)
				m_LastKey = 0;
			if (0 < tMsg.Param1 && tMsg.Param1 < _countof(m_KeyStateMap))
				m_KeyStateMap[tMsg.Param1] = false;
			break;
		default:
			break;
		}
	}

	// ִ��֡����
	if (!SafeCallGlobalFunction(LFUNC_FRAME, 0, 1))
		return false;
	bool tAbort = lua_toboolean(L, -1) == 0 ? false : true;
	lua_pop(L, 1);

	return !tAbort;
}

fBool AppFrame::OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController)
{
	m_pRenderDev->Clear();

	// ִ����Ⱦ����
	m_bRenderStarted = true;
	m_bPostEffectCaptureStarted = false;
	if (!SafeCallGlobalFunction(LFUNC_RENDER, 0, 0))
		m_pEngine->Abort();
	m_bRenderStarted = false;

	return true;
}
#pragma endregion
