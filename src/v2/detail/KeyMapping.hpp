/**
 * @file
 * @author 9chu
 * @date 2022/7/31
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <SDL2/SDL.h>

namespace lstg::v2::detail
{
    /**
     * Win32 VKCode 转 SDL ScanCode
     */
    inline SDL_Scancode VKCodeToSDLScanCode(int32_t vkCode) noexcept
    {
        // SDL_winrtkeyboard.cpp
        static const SDL_Scancode kWindowsOfficialKeycodes[] = {
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.None -- 0 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.LeftButton -- 1 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.RightButton -- 2 */
            SDL_SCANCODE_CANCEL,        /* VirtualKey.Cancel -- 3 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.MiddleButton -- 4 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.XButton1 -- 5 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.XButton2 -- 6 */
            SDL_SCANCODE_UNKNOWN,       /* -- 7 */
            SDL_SCANCODE_BACKSPACE,     /* VirtualKey.Back -- 8 */
            SDL_SCANCODE_TAB,           /* VirtualKey.Tab -- 9 */
            SDL_SCANCODE_UNKNOWN,       /* -- 10 */
            SDL_SCANCODE_UNKNOWN,       /* -- 11 */
            SDL_SCANCODE_CLEAR,         /* VirtualKey.Clear -- 12 */
            SDL_SCANCODE_RETURN,        /* VirtualKey.Enter -- 13 */
            SDL_SCANCODE_UNKNOWN,       /* -- 14 */
            SDL_SCANCODE_UNKNOWN,       /* -- 15 */
            SDL_SCANCODE_LSHIFT,        /* VirtualKey.Shift -- 16 */
            SDL_SCANCODE_LCTRL,         /* VirtualKey.Control -- 17 */
            SDL_SCANCODE_MENU,          /* VirtualKey.Menu -- 18 */
            SDL_SCANCODE_PAUSE,         /* VirtualKey.Pause -- 19 */
            SDL_SCANCODE_CAPSLOCK,      /* VirtualKey.CapitalLock -- 20 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.Kana or VirtualKey.Hangul -- 21 */
            SDL_SCANCODE_UNKNOWN,       /* -- 22 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.Junja -- 23 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.Final -- 24 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.Hanja or VirtualKey.Kanji -- 25 */
            SDL_SCANCODE_UNKNOWN,       /* -- 26 */
            SDL_SCANCODE_ESCAPE,        /* VirtualKey.Escape -- 27 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.Convert -- 28 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.NonConvert -- 29 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.Accept -- 30 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.ModeChange -- 31  (maybe SDL_SCANCODE_MODE ?) */
            SDL_SCANCODE_SPACE,         /* VirtualKey.Space -- 32 */
            SDL_SCANCODE_PAGEUP,        /* VirtualKey.PageUp -- 33 */
            SDL_SCANCODE_PAGEDOWN,      /* VirtualKey.PageDown -- 34 */
            SDL_SCANCODE_END,           /* VirtualKey.End -- 35 */
            SDL_SCANCODE_HOME,          /* VirtualKey.Home -- 36 */
            SDL_SCANCODE_LEFT,          /* VirtualKey.Left -- 37 */
            SDL_SCANCODE_UP,            /* VirtualKey.Up -- 38 */
            SDL_SCANCODE_RIGHT,         /* VirtualKey.Right -- 39 */
            SDL_SCANCODE_DOWN,          /* VirtualKey.Down -- 40 */
            SDL_SCANCODE_SELECT,        /* VirtualKey.Select -- 41 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.Print -- 42  (maybe SDL_SCANCODE_PRINTSCREEN ?) */
            SDL_SCANCODE_EXECUTE,       /* VirtualKey.Execute -- 43 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.Snapshot -- 44 */
            SDL_SCANCODE_INSERT,        /* VirtualKey.Insert -- 45 */
            SDL_SCANCODE_DELETE,        /* VirtualKey.Delete -- 46 */
            SDL_SCANCODE_HELP,          /* VirtualKey.Help -- 47 */
            SDL_SCANCODE_0,             /* VirtualKey.Number0 -- 48 */
            SDL_SCANCODE_1,             /* VirtualKey.Number1 -- 49 */
            SDL_SCANCODE_2,             /* VirtualKey.Number2 -- 50 */
            SDL_SCANCODE_3,             /* VirtualKey.Number3 -- 51 */
            SDL_SCANCODE_4,             /* VirtualKey.Number4 -- 52 */
            SDL_SCANCODE_5,             /* VirtualKey.Number5 -- 53 */
            SDL_SCANCODE_6,             /* VirtualKey.Number6 -- 54 */
            SDL_SCANCODE_7,             /* VirtualKey.Number7 -- 55 */
            SDL_SCANCODE_8,             /* VirtualKey.Number8 -- 56 */
            SDL_SCANCODE_9,             /* VirtualKey.Number9 -- 57 */
            SDL_SCANCODE_UNKNOWN,       /* -- 58 */
            SDL_SCANCODE_UNKNOWN,       /* -- 59 */
            SDL_SCANCODE_UNKNOWN,       /* -- 60 */
            SDL_SCANCODE_UNKNOWN,       /* -- 61 */
            SDL_SCANCODE_UNKNOWN,       /* -- 62 */
            SDL_SCANCODE_UNKNOWN,       /* -- 63 */
            SDL_SCANCODE_UNKNOWN,       /* -- 64 */
            SDL_SCANCODE_A,             /* VirtualKey.A -- 65 */
            SDL_SCANCODE_B,             /* VirtualKey.B -- 66 */
            SDL_SCANCODE_C,             /* VirtualKey.C -- 67 */
            SDL_SCANCODE_D,             /* VirtualKey.D -- 68 */
            SDL_SCANCODE_E,             /* VirtualKey.E -- 69 */
            SDL_SCANCODE_F,             /* VirtualKey.F -- 70 */
            SDL_SCANCODE_G,             /* VirtualKey.G -- 71 */
            SDL_SCANCODE_H,             /* VirtualKey.H -- 72 */
            SDL_SCANCODE_I,             /* VirtualKey.I -- 73 */
            SDL_SCANCODE_J,             /* VirtualKey.J -- 74 */
            SDL_SCANCODE_K,             /* VirtualKey.K -- 75 */
            SDL_SCANCODE_L,             /* VirtualKey.L -- 76 */
            SDL_SCANCODE_M,             /* VirtualKey.M -- 77 */
            SDL_SCANCODE_N,             /* VirtualKey.N -- 78 */
            SDL_SCANCODE_O,             /* VirtualKey.O -- 79 */
            SDL_SCANCODE_P,             /* VirtualKey.P -- 80 */
            SDL_SCANCODE_Q,             /* VirtualKey.Q -- 81 */
            SDL_SCANCODE_R,             /* VirtualKey.R -- 82 */
            SDL_SCANCODE_S,             /* VirtualKey.S -- 83 */
            SDL_SCANCODE_T,             /* VirtualKey.T -- 84 */
            SDL_SCANCODE_U,             /* VirtualKey.U -- 85 */
            SDL_SCANCODE_V,             /* VirtualKey.V -- 86 */
            SDL_SCANCODE_W,             /* VirtualKey.W -- 87 */
            SDL_SCANCODE_X,             /* VirtualKey.X -- 88 */
            SDL_SCANCODE_Y,             /* VirtualKey.Y -- 89 */
            SDL_SCANCODE_Z,             /* VirtualKey.Z -- 90 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.LeftWindows -- 91  (maybe SDL_SCANCODE_APPLICATION or SDL_SCANCODE_LGUI ?) */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.RightWindows -- 92  (maybe SDL_SCANCODE_APPLICATION or SDL_SCANCODE_RGUI ?) */
            SDL_SCANCODE_APPLICATION,   /* VirtualKey.Application -- 93 */
            SDL_SCANCODE_UNKNOWN,       /* -- 94 */
            SDL_SCANCODE_SLEEP,         /* VirtualKey.Sleep -- 95 */
            SDL_SCANCODE_KP_0,          /* VirtualKey.NumberPad0 -- 96 */
            SDL_SCANCODE_KP_1,          /* VirtualKey.NumberPad1 -- 97 */
            SDL_SCANCODE_KP_2,          /* VirtualKey.NumberPad2 -- 98 */
            SDL_SCANCODE_KP_3,          /* VirtualKey.NumberPad3 -- 99 */
            SDL_SCANCODE_KP_4,          /* VirtualKey.NumberPad4 -- 100 */
            SDL_SCANCODE_KP_5,          /* VirtualKey.NumberPad5 -- 101 */
            SDL_SCANCODE_KP_6,          /* VirtualKey.NumberPad6 -- 102 */
            SDL_SCANCODE_KP_7,          /* VirtualKey.NumberPad7 -- 103 */
            SDL_SCANCODE_KP_8,          /* VirtualKey.NumberPad8 -- 104 */
            SDL_SCANCODE_KP_9,          /* VirtualKey.NumberPad9 -- 105 */
            SDL_SCANCODE_KP_MULTIPLY,   /* VirtualKey.Multiply -- 106 */
            SDL_SCANCODE_KP_PLUS,       /* VirtualKey.Add -- 107 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.Separator -- 108 */
            SDL_SCANCODE_KP_MINUS,      /* VirtualKey.Subtract -- 109 */
            SDL_SCANCODE_UNKNOWN,       /* VirtualKey.Decimal -- 110  (maybe SDL_SCANCODE_DECIMALSEPARATOR, SDL_SCANCODE_KP_DECIMAL, or SDL_SCANCODE_KP_PERIOD ?) */
            SDL_SCANCODE_KP_DIVIDE,     /* VirtualKey.Divide -- 111 */
            SDL_SCANCODE_F1,            /* VirtualKey.F1 -- 112 */
            SDL_SCANCODE_F2,            /* VirtualKey.F2 -- 113 */
            SDL_SCANCODE_F3,            /* VirtualKey.F3 -- 114 */
            SDL_SCANCODE_F4,            /* VirtualKey.F4 -- 115 */
            SDL_SCANCODE_F5,            /* VirtualKey.F5 -- 116 */
            SDL_SCANCODE_F6,            /* VirtualKey.F6 -- 117 */
            SDL_SCANCODE_F7,            /* VirtualKey.F7 -- 118 */
            SDL_SCANCODE_F8,            /* VirtualKey.F8 -- 119 */
            SDL_SCANCODE_F9,            /* VirtualKey.F9 -- 120 */
            SDL_SCANCODE_F10,           /* VirtualKey.F10 -- 121 */
            SDL_SCANCODE_F11,           /* VirtualKey.F11 -- 122 */
            SDL_SCANCODE_F12,           /* VirtualKey.F12 -- 123 */
            SDL_SCANCODE_F13,           /* VirtualKey.F13 -- 124 */
            SDL_SCANCODE_F14,           /* VirtualKey.F14 -- 125 */
            SDL_SCANCODE_F15,           /* VirtualKey.F15 -- 126 */
            SDL_SCANCODE_F16,           /* VirtualKey.F16 -- 127 */
            SDL_SCANCODE_F17,           /* VirtualKey.F17 -- 128 */
            SDL_SCANCODE_F18,           /* VirtualKey.F18 -- 129 */
            SDL_SCANCODE_F19,           /* VirtualKey.F19 -- 130 */
            SDL_SCANCODE_F20,           /* VirtualKey.F20 -- 131 */
            SDL_SCANCODE_F21,           /* VirtualKey.F21 -- 132 */
            SDL_SCANCODE_F22,           /* VirtualKey.F22 -- 133 */
            SDL_SCANCODE_F23,           /* VirtualKey.F23 -- 134 */
            SDL_SCANCODE_F24,           /* VirtualKey.F24 -- 135 */
            SDL_SCANCODE_UNKNOWN,       /* -- 136 */
            SDL_SCANCODE_UNKNOWN,       /* -- 137 */
            SDL_SCANCODE_UNKNOWN,       /* -- 138 */
            SDL_SCANCODE_UNKNOWN,       /* -- 139 */
            SDL_SCANCODE_UNKNOWN,       /* -- 140 */
            SDL_SCANCODE_UNKNOWN,       /* -- 141 */
            SDL_SCANCODE_UNKNOWN,       /* -- 142 */
            SDL_SCANCODE_UNKNOWN,       /* -- 143 */
            SDL_SCANCODE_NUMLOCKCLEAR,  /* VirtualKey.NumberKeyLock -- 144 */
            SDL_SCANCODE_SCROLLLOCK,    /* VirtualKey.Scroll -- 145 */
            SDL_SCANCODE_UNKNOWN,       /* -- 146 */
            SDL_SCANCODE_UNKNOWN,       /* -- 147 */
            SDL_SCANCODE_UNKNOWN,       /* -- 148 */
            SDL_SCANCODE_UNKNOWN,       /* -- 149 */
            SDL_SCANCODE_UNKNOWN,       /* -- 150 */
            SDL_SCANCODE_UNKNOWN,       /* -- 151 */
            SDL_SCANCODE_UNKNOWN,       /* -- 152 */
            SDL_SCANCODE_UNKNOWN,       /* -- 153 */
            SDL_SCANCODE_UNKNOWN,       /* -- 154 */
            SDL_SCANCODE_UNKNOWN,       /* -- 155 */
            SDL_SCANCODE_UNKNOWN,       /* -- 156 */
            SDL_SCANCODE_UNKNOWN,       /* -- 157 */
            SDL_SCANCODE_UNKNOWN,       /* -- 158 */
            SDL_SCANCODE_UNKNOWN,       /* -- 159 */
            SDL_SCANCODE_LSHIFT,        /* VirtualKey.LeftShift -- 160 */
            SDL_SCANCODE_RSHIFT,        /* VirtualKey.RightShift -- 161 */
            SDL_SCANCODE_LCTRL,         /* VirtualKey.LeftControl -- 162 */
            SDL_SCANCODE_RCTRL,         /* VirtualKey.RightControl -- 163 */
            SDL_SCANCODE_MENU,          /* VirtualKey.LeftMenu -- 164 */
            SDL_SCANCODE_MENU,          /* VirtualKey.RightMenu -- 165 */
            SDL_SCANCODE_AC_BACK,       /* VirtualKey.GoBack -- 166 : The go back key. */
            SDL_SCANCODE_AC_FORWARD,    /* VirtualKey.GoForward -- 167 : The go forward key. */
            SDL_SCANCODE_AC_REFRESH,    /* VirtualKey.Refresh -- 168 : The refresh key. */
            SDL_SCANCODE_AC_STOP,       /* VirtualKey.Stop -- 169 : The stop key. */
            SDL_SCANCODE_AC_SEARCH,     /* VirtualKey.Search -- 170 : The search key. */
            SDL_SCANCODE_AC_BOOKMARKS,  /* VirtualKey.Favorites -- 171 : The favorites key. */
            SDL_SCANCODE_AC_HOME        /* VirtualKey.GoHome -- 172 : The go home key. */
        };

        SDL_Scancode ret = SDL_SCANCODE_UNKNOWN;
        if (0 <= vkCode && vkCode < static_cast<int>(SDL_arraysize(kWindowsOfficialKeycodes)))
            ret = kWindowsOfficialKeycodes[vkCode];

        if (ret == SDL_SCANCODE_UNKNOWN)
        {
            switch (vkCode)
            {
                case 173:
                    ret = SDL_SCANCODE_MUTE;         /* VK_VOLUME_MUTE */
                    break;
                case 174:
                    ret = SDL_SCANCODE_VOLUMEDOWN;   /* VK_VOLUME_DOWN */
                    break;
                case 175:
                    ret = SDL_SCANCODE_VOLUMEUP;     /* VK_VOLUME_UP */
                    break;
                case 176:
                    ret = SDL_SCANCODE_AUDIONEXT;    /* VK_MEDIA_NEXT_TRACK */
                    break;
                case 177:
                    ret = SDL_SCANCODE_AUDIOPREV;    /* VK_MEDIA_PREV_TRACK */
                    break;
                    // case 178: return ;                       /* VK_MEDIA_STOP */
                case 179:
                    ret = SDL_SCANCODE_AUDIOPLAY;    /* VK_MEDIA_PLAY_PAUSE */
                    break;
                case 180:
                    ret = SDL_SCANCODE_MAIL;         /* VK_LAUNCH_MAIL */
                    break;
                case 181:
                    ret = SDL_SCANCODE_MEDIASELECT;  /* VK_LAUNCH_MEDIA_SELECT */
                    break;
                    // case 182: return ;                       /* VK_LAUNCH_APP1 */
                case 183:
                    ret = SDL_SCANCODE_CALCULATOR;   /* VK_LAUNCH_APP2 */
                    break;
                    // case 184: return ;                       /* ... reserved ... */
                    // case 185: return ;                       /* ... reserved ... */
                case 186:
                    ret = SDL_SCANCODE_SEMICOLON;    /* VK_OEM_1, ';:' key on US standard keyboards */
                    break;
                case 187:
                    ret = SDL_SCANCODE_EQUALS;       /* VK_OEM_PLUS */
                    break;
                case 188:
                    ret = SDL_SCANCODE_COMMA;        /* VK_OEM_COMMA */
                    break;
                case 189:
                    ret = SDL_SCANCODE_MINUS;        /* VK_OEM_MINUS */
                    break;
                case 190:
                    ret = SDL_SCANCODE_PERIOD;       /* VK_OEM_PERIOD */
                    break;
                case 191:
                    ret = SDL_SCANCODE_SLASH;        /* VK_OEM_2, '/?' key on US standard keyboards */
                    break;
                case 192:
                    ret = SDL_SCANCODE_GRAVE;        /* VK_OEM_3, '`~' key on US standard keyboards */
                    break;
                    // ?
                    // ... reserved or unassigned ...
                    // ?
                case 219:
                    ret = SDL_SCANCODE_LEFTBRACKET;  /* VK_OEM_4, '[{' key on US standard keyboards */
                    break;
                case 220:
                    ret = SDL_SCANCODE_BACKSLASH;    /* VK_OEM_5, '\|' key on US standard keyboards */
                    break;
                case 221:
                    ret = SDL_SCANCODE_RIGHTBRACKET; /* VK_OEM_6, ']}' key on US standard keyboards */
                    break;
                case 222:
                    ret = SDL_SCANCODE_APOSTROPHE;   /* VK_OEM_7, 'single/double quote' on US standard keyboards */
                    break;
                default:
                    break;
            }
        }
        return ret;
    }

    /**
     * SDL 扫描码转 VK 码
     * @param code 扫描码
     * @return VK 码
     */
    inline int32_t SDLScancodeToVKCode(SDL_Scancode code) noexcept
    {
        switch (code)
        {
            case SDL_SCANCODE_CANCEL: return 3;
            case SDL_SCANCODE_BACKSPACE: return 8;
            case SDL_SCANCODE_TAB: return 9;
            case SDL_SCANCODE_CLEAR: return 12;
            case SDL_SCANCODE_RETURN: return 13;
            case SDL_SCANCODE_LSHIFT: return 16;
            case SDL_SCANCODE_LCTRL: return 17;
            case SDL_SCANCODE_MENU: return 18;
            case SDL_SCANCODE_PAUSE: return 19;
            case SDL_SCANCODE_CAPSLOCK: return 20;
            case SDL_SCANCODE_ESCAPE: return 27;
            case SDL_SCANCODE_SPACE: return 32;
            case SDL_SCANCODE_PAGEUP: return 33;
            case SDL_SCANCODE_PAGEDOWN: return 34;
            case SDL_SCANCODE_END: return 35;
            case SDL_SCANCODE_HOME: return 36;
            case SDL_SCANCODE_LEFT: return 37;
            case SDL_SCANCODE_UP: return 38;
            case SDL_SCANCODE_RIGHT: return 39;
            case SDL_SCANCODE_DOWN: return 40;
            case SDL_SCANCODE_SELECT: return 41;
            case SDL_SCANCODE_EXECUTE: return 43;
            case SDL_SCANCODE_INSERT: return 45;
            case SDL_SCANCODE_DELETE: return 46;
            case SDL_SCANCODE_HELP: return 47;
            case SDL_SCANCODE_0: return 48;
            case SDL_SCANCODE_1: return 49;
            case SDL_SCANCODE_2: return 50;
            case SDL_SCANCODE_3: return 51;
            case SDL_SCANCODE_4: return 52;
            case SDL_SCANCODE_5: return 53;
            case SDL_SCANCODE_6: return 54;
            case SDL_SCANCODE_7: return 55;
            case SDL_SCANCODE_8: return 56;
            case SDL_SCANCODE_9: return 57;
            case SDL_SCANCODE_A: return 65;
            case SDL_SCANCODE_B: return 66;
            case SDL_SCANCODE_C: return 67;
            case SDL_SCANCODE_D: return 68;
            case SDL_SCANCODE_E: return 69;
            case SDL_SCANCODE_F: return 70;
            case SDL_SCANCODE_G: return 71;
            case SDL_SCANCODE_H: return 72;
            case SDL_SCANCODE_I: return 73;
            case SDL_SCANCODE_J: return 74;
            case SDL_SCANCODE_K: return 75;
            case SDL_SCANCODE_L: return 76;
            case SDL_SCANCODE_M: return 77;
            case SDL_SCANCODE_N: return 78;
            case SDL_SCANCODE_O: return 79;
            case SDL_SCANCODE_P: return 80;
            case SDL_SCANCODE_Q: return 81;
            case SDL_SCANCODE_R: return 82;
            case SDL_SCANCODE_S: return 83;
            case SDL_SCANCODE_T: return 84;
            case SDL_SCANCODE_U: return 85;
            case SDL_SCANCODE_V: return 86;
            case SDL_SCANCODE_W: return 87;
            case SDL_SCANCODE_X: return 88;
            case SDL_SCANCODE_Y: return 89;
            case SDL_SCANCODE_Z: return 90;
            case SDL_SCANCODE_APPLICATION: return 93;
            case SDL_SCANCODE_SLEEP: return 95;
            case SDL_SCANCODE_KP_0: return 96;
            case SDL_SCANCODE_KP_1: return 97;
            case SDL_SCANCODE_KP_2: return 98;
            case SDL_SCANCODE_KP_3: return 99;
            case SDL_SCANCODE_KP_4: return 100;
            case SDL_SCANCODE_KP_5: return 101;
            case SDL_SCANCODE_KP_6: return 102;
            case SDL_SCANCODE_KP_7: return 103;
            case SDL_SCANCODE_KP_8: return 104;
            case SDL_SCANCODE_KP_9: return 105;
            case SDL_SCANCODE_KP_MULTIPLY: return 106;
            case SDL_SCANCODE_KP_PLUS: return 107;
            case SDL_SCANCODE_KP_MINUS: return 109;
            case SDL_SCANCODE_KP_DIVIDE: return 111;
            case SDL_SCANCODE_F1: return 112;
            case SDL_SCANCODE_F2: return 113;
            case SDL_SCANCODE_F3: return 114;
            case SDL_SCANCODE_F4: return 115;
            case SDL_SCANCODE_F5: return 116;
            case SDL_SCANCODE_F6: return 117;
            case SDL_SCANCODE_F7: return 118;
            case SDL_SCANCODE_F8: return 119;
            case SDL_SCANCODE_F9: return 120;
            case SDL_SCANCODE_F10: return 121;
            case SDL_SCANCODE_F11: return 122;
            case SDL_SCANCODE_F12: return 123;
            case SDL_SCANCODE_F13: return 124;
            case SDL_SCANCODE_F14: return 125;
            case SDL_SCANCODE_F15: return 126;
            case SDL_SCANCODE_F16: return 127;
            case SDL_SCANCODE_F17: return 128;
            case SDL_SCANCODE_F18: return 129;
            case SDL_SCANCODE_F19: return 130;
            case SDL_SCANCODE_F20: return 131;
            case SDL_SCANCODE_F21: return 132;
            case SDL_SCANCODE_F22: return 133;
            case SDL_SCANCODE_F23: return 134;
            case SDL_SCANCODE_F24: return 135;
            case SDL_SCANCODE_NUMLOCKCLEAR: return 144;
            case SDL_SCANCODE_SCROLLLOCK: return 145;
//            case SDL_SCANCODE_LSHIFT: return 160;
            case SDL_SCANCODE_RSHIFT: return 161;
//            case SDL_SCANCODE_LCTRL: return 162;
            case SDL_SCANCODE_RCTRL: return 163;
//            case SDL_SCANCODE_MENU: return 164;
//            case SDL_SCANCODE_MENU: return 165;
            case SDL_SCANCODE_AC_BACK: return 166;
            case SDL_SCANCODE_AC_FORWARD: return 167;
            case SDL_SCANCODE_AC_REFRESH: return 168;
            case SDL_SCANCODE_AC_STOP: return 169;
            case SDL_SCANCODE_AC_SEARCH: return 170;
            case SDL_SCANCODE_AC_BOOKMARKS: return 171;
            case SDL_SCANCODE_AC_HOME: return 172;
            case SDL_SCANCODE_MUTE: return 173;
            case SDL_SCANCODE_VOLUMEDOWN: return 174;
            case SDL_SCANCODE_VOLUMEUP: return 175;
            case SDL_SCANCODE_AUDIONEXT: return 176;
            case SDL_SCANCODE_AUDIOPREV: return 177;
            case SDL_SCANCODE_AUDIOPLAY: return 179;
            case SDL_SCANCODE_MAIL: return 180;
            case SDL_SCANCODE_MEDIASELECT: return 181;
            case SDL_SCANCODE_CALCULATOR: return 183;
            case SDL_SCANCODE_SEMICOLON: return 186;
            case SDL_SCANCODE_EQUALS: return 187;
            case SDL_SCANCODE_COMMA: return 188;
            case SDL_SCANCODE_MINUS: return 189;
            case SDL_SCANCODE_PERIOD: return 190;
            case SDL_SCANCODE_SLASH: return 191;
            case SDL_SCANCODE_GRAVE: return 192;
            case SDL_SCANCODE_LEFTBRACKET: return 219;
            case SDL_SCANCODE_BACKSLASH: return 220;
            case SDL_SCANCODE_RIGHTBRACKET: return 221;
            case SDL_SCANCODE_APOSTROPHE: return 222;
            default: return 0;
        }
    }
}
