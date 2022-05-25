#pragma once

template <size_t S> class Sizer { };

int UIPanelOffset;
struct UIPanel
{
    uintptr_t* vtable_ptr;
    char Padding_1[0x120];
};
Sizer<sizeof(UIPanel)> xddd;

#pragma region Pin
struct OtpInput
{
    unsigned int inputImage;
    int inputBtnIndex;
};
struct OtpNumpad
{
    unsigned int numpadBtn;
    int numpadValue;
};

class LoginOtpPanel2
{
public:
    char Padding_1[0xB0];
    struct OtpInput _otpInputList[8];
    struct OtpNumpad _otpNumpadList[10];
    char Padding_2[0x1C];
    int _verifyFailCount;
    int _curInputCount;
    char Padding_3[0xC];
    bool _keyboardEnabled;
};

struct __declspec(align(8)) LoginOtpPanel : UIPanel
{
    char Padding_1[0x18];
    OtpInput _otpInputList[8];
    OtpNumpad _otpNumpadList[10];
    unsigned int _initNumBtn;
    unsigned int _backNumBtn;
    unsigned int _helpLabel;
    unsigned int _confirmBtn;
    unsigned int _closeBtn;
    unsigned int _verifyModeHolder;
    bool _verifyReq;
    int _verifyFailCount;
    int _curInputCount;
    int _securityCardCodeIndex;
    bool _inputing;
    bool _secondaryAuthenticationLocked;
    bool _systemErrorOccured;
    unsigned int _errorMessageTimerId;
    bool _keyboardEnabled;
};
Sizer<offsetof(LoginOtpPanel, _verifyModeHolder)> xd1;
static_assert(offsetof(LoginOtpPanel, _verifyModeHolder) == 0x1E4);
#pragma endregion

#pragma region Lobby
struct LoginHelperData
{
    std::wstring npAddress;
    unsigned __int16 npPort;
    unsigned __int16 npProgramId;
    std::wstring npGameCode;
    std::wstring lobbyAddress;
    unsigned __int16 lobbyPort;
};

struct UIEngineInterface
{
    char Padding_1[0x150];
    bool(__fastcall* SetTextValue)(UIEngineInterface* thisptr, unsigned int Id, const wchar_t *szValue);
};
static_assert(offsetof(UIEngineInterface, SetTextValue) == 0x150);

struct TitleLoginPanel : UIPanel
{
    unsigned int _userIdEditBoxID;
    unsigned int _userPasswordEditBoxID;
    unsigned int _loginBtnID;
    unsigned int _loginHolder;
    unsigned int _focusDummyImage;
    unsigned int _focusedEditBox;
};
static_assert(offsetof(TitleLoginPanel, _userIdEditBoxID) == 0x128);
static_assert(offsetof(TitleLoginPanel, _userPasswordEditBoxID) == 0x12C);
#pragma endregion
