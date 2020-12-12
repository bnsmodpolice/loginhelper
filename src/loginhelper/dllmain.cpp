#include <pe/module.h>
#include <fnv1a.h>
#include <xorstr.hpp>
#include <regex>
#include "versioninfo.h"
#include "pluginsdk.h"
#include "searchers.h"
#include "Data.h"

#ifdef _M_X64
#define thiscall_(name, thisarg, ...) name(thisarg, ## __VA_ARGS__) 
#else
#define thiscall_(name, thisarg, ...) __fastcall name(thisarg, intptr_t vv, ## __VA_ARGS__) 
#endif
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))

#ifndef _M_X64
bool Detour32(char* src, char* dst, const uintptr_t len)
{
	if (len < 5) return false;
	DWORD  curProtection;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);
	uintptr_t  relativeAddress = (uintptr_t)(dst - (uintptr_t)src) - 5;

	*src = (char)'\xE9';
	*(uintptr_t*)((uintptr_t)src + 1) = relativeAddress;
	for (uintptr_t i = 1 + sizeof(uintptr_t); i < len; i++) {
		*(src + i) = (char)'\x90';
	}

	VirtualProtect(src, len, curProtection, &curProtection);
	return true;
}
#endif

#ifdef READ_WINDOW_TITLE
BOOL is_main_window(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}
struct handle_data {
	unsigned long process_id;
	HWND window_handle;
};
BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle))
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}
HWND find_main_window()
{
	handle_data data;
	data.process_id = GetCurrentProcessId();
	data.window_handle = 0;
	EnumWindows((WNDENUMPROC)enum_windows_callback, (LPARAM)&data);
	return data.window_handle;
}
#endif

// Pin
int pincode[6] = { -1 };
bool keeppin = false;
std::wstring pintext;
int count = 0;
uintptr_t pinClassOffset = 0;
bool GetPin()
{
	if (pincode[0] == -2)
		return false;
	if (pincode[0] != -1)
		return true;
	// Environment Variable: "BNS_PINCODE=123456"
	WCHAR pinchar[256];
	if (GetEnvironmentVariableW(L"BNS_PINCODE", &pinchar[0], 256)) {
		pintext = pinchar;
		for (int i = 0; i < 6; i++) {
			pincode[i] = pintext[i] - '0';
		}
		if (!keeppin)
			SetEnvironmentVariableW(L"BNS_PINCODE", L"");
		return true;
	}
	// Command line argument: "Client.exe xx -PIN:123456 xx"
	std::wstring args = GetCommandLineW();
	std::wregex regex_cmd(L" -PIN[:=]\"?([0-9]{6})\"?(?:$| )", std::regex_constants::icase);
	std::wsmatch matches_cmd;
	if (std::regex_search(args, matches_cmd, regex_cmd)) {
		pintext = matches_cmd[1].str();
		for (int i = 0; i < 6; i++) {
			pincode[i] = pintext[i] - '0';
		}
		return true;
	}
	// Window title for custom replacement launcher: "Blade & Soul; PIN: 123456; NAME: Abc.exe"
#ifdef READ_WINDOW_TITLE
	std::wstring window_title;
	WCHAR window_char[512];
	HWND hwnd = find_main_window();
	GetWindowTextW(hwnd, window_char, 512);
	window_title = window_char;
	std::wregex regex_wnd(L"PIN: ([0-9]{6})");
	std::wsmatch matches_wnd;
	if (std::regex_search(window_title, matches_wnd, regex_wnd)) {
		pintext = matches_wnd[1].str();
		for (int i = 0; i < 6; i++) {
			pincode[i] = pintext[i] - '0';
		}
		return true;
	}
#endif
	return false;
}
#ifdef _M_X64
bool(__stdcall* oPinSubmit)(uintptr_t*);
bool __stdcall hkPinSubmit(uintptr_t* p1)
{
	if (!keeppin)
		pincode[0] = -2;
	count = 0;
	return oPinSubmit(p1);
}

bool(__thiscall* oPinInput)(PinClass*);
bool thiscall_(hkPinInput, PinClass* loginC)
{
	PinClass* ologinC = loginC;
	loginC = (PinClass*)((uintptr_t)loginC + pinClassOffset);
	loginC->useKeyboard = true;
	for (int i = 0; i < 10; i++) {
		loginC->numpads[i].number = i;
	}
	count++;
	if (loginC->failCount == 0 && count - 1 == 0) {
		if (GetPin()) {
			for (int i = 0; i < 6; i++)
			{
				loginC->inputs[i].number = pincode[i];
			}
			loginC->inputCount = 6;
			hkPinSubmit((uintptr_t*)ologinC);
		}
	}
	if (loginC->failCount > 4)
		loginC->failCount = 1;
	bool v = oPinInput(ologinC);
	return v;
}
#else
bool(__stdcall* oPinSubmit)();
bool __stdcall hkPinSubmit()
{
	if (!keeppin)
		pincode[0] = -2;
	count = 0; // it's fine to reset it here because if it fails failCount will be != 0
	return oPinSubmit();
}

uintptr_t aPinInputTarget = 0;
PinClass* loginC;
__declspec(naked) void hkPinInputAsm()
{
	{
		__asm
		{
			push esp
			push edx
			push ebp
			push eax
			push ecx
			push esp
			push esi
		}

		__asm
		{
			mov loginC, edi
		}
		loginC = (PinClass*)((uintptr_t)loginC + pinClassOffset);
		loginC->useKeyboard = true;
		// for loop on numpads crashes the game :shrug:
		loginC->numpads[0].number = 0;
		loginC->numpads[1].number = 1;
		loginC->numpads[2].number = 2;
		loginC->numpads[3].number = 3;
		loginC->numpads[4].number = 4;
		loginC->numpads[5].number = 5;
		loginC->numpads[6].number = 6;
		loginC->numpads[7].number = 7;
		loginC->numpads[8].number = 8;
		loginC->numpads[9].number = 9;
		count++;
		if (loginC->failCount == 0 && count - 1 == 0) {
			if (GetPin()) {
				for (int i = 0; i < 6; i++)
				{
					loginC->inputs[i].number = pincode[i];
				}
				loginC->inputCount = 6;
				hkPinSubmit();
			}
		}
		if (loginC->failCount > 4)
			loginC->failCount = 1;

		__asm
		{
			pop esi
			pop esp
			pop ecx
			pop eax
			pop ebp
			pop edx
			pop esp
		}
		__asm
		{
			push ebp
			mov ebp, esp
			and esp, -0x08
		}
		__asm
		{
			jmp[aPinInputTarget]
		}
	}
}
#endif

// Login
std::wstring m_account;
std::wstring m_password;
int loggedin = 0;
bool GetLogin()
{
	if (loggedin > 1)
		return false;
	loggedin = 2;
	// Environment Variables
	WCHAR mailchar[256];
	if (GetEnvironmentVariableW(L"BNS_PROFILE_USERNAME", &mailchar[0], 256)) {
		m_account = mailchar;
		WCHAR passchar[256];
		SetEnvironmentVariableW(L"BNS_PROFILE_USERNAME", L"");
		if (GetEnvironmentVariableW(L"BNS_PROFILE_PASSWORD", &passchar[0], 256)) {
			m_password = passchar;
			SetEnvironmentVariableW(L"BNS_PROFILE_PASSWORD", L"");
			return true;
		}
		else {
			return false;
		}
	}
	// Command line arguments
	std::wstring args = GetCommandLineW();
	std::wregex email_regex_cmd(L" -USERNAME[:=]\"?([^ \"]*)\"?(?:$| )", std::regex_constants::icase);
	std::wsmatch email_matches_cmd;
	if (std::regex_search(args, email_matches_cmd, email_regex_cmd)) {
		m_account = email_matches_cmd[1].str();
		std::wregex pass_regex_cmd(L" -PASSWORD[:=](?:\"([^\"]*)\"|([^ \"]*))(?:$| )", std::regex_constants::icase);
		std::wsmatch pass_matches_cmd;
		if (std::regex_search(args, pass_matches_cmd, pass_regex_cmd)) {
			if (pass_matches_cmd[2].matched)
				m_password = pass_matches_cmd[2].str();
			else
				m_password = pass_matches_cmd[1].str();
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}
struct Lobby;
bool(__fastcall* oRequestLogin)(Lobby*, const wchar_t*, unsigned __int16, int, const wchar_t*, const wchar_t*, unsigned __int16, const wchar_t*, const wchar_t*);
bool __fastcall hkRequestLogin(
	Lobby* thisptr,
	const wchar_t* npAddress,
	unsigned __int16 npPort,
	unsigned __int16 npProgramId,
	const wchar_t* npGameCode,
	const wchar_t* lobbyAddress,
	unsigned __int16 lobbyPort,
	const wchar_t* account,
	const wchar_t* password)
{
	if (GetLogin()) {
		account = m_account.c_str();
		password = m_password.c_str();
	}
	return oRequestLogin(thisptr, npAddress, npPort, npProgramId, npGameCode, lobbyAddress, lobbyPort, account, password);
}
#ifndef _M_X64
bool(__fastcall* oRequestLoginPS)(const wchar_t*, const wchar_t*, unsigned __int16, unsigned __int16, const wchar_t*, const wchar_t*, unsigned __int16, const wchar_t*);
bool __fastcall hkRequestLoginPS(
	const wchar_t* npAddress,
	const wchar_t* account,
	unsigned __int16 npPort,
	unsigned __int16 npProgramId,
	const wchar_t* npGameCode,
	const wchar_t* lobbyAddress,
	unsigned __int16 lobbyPort,
	const wchar_t* password)
{
	if (GetLogin()) {
		account = m_account.c_str();
		password = m_password.c_str();
	}
	return oRequestLoginPS(npAddress, account, npPort, npProgramId, npGameCode, lobbyAddress, lobbyPort, password);
}
#endif
bool(__fastcall* oLogin)();
bool __fastcall hkLogin()
{
	return oLogin();
}
void DoLogin() {
	Sleep(100);
	hkLogin();
}
struct BNSClient;
bool(__fastcall* oLobbyHelper)(BNSClient*, const wchar_t*, const wchar_t*);
bool __fastcall hkLobbyHelper(BNSClient* thisptr, const wchar_t* p1, const wchar_t* p2)
{
	if (loggedin == 0) {
		loggedin = 1;
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)DoLogin, nullptr, 0, nullptr)); // Separate thread might be unnecessary
	}
	bool v = oLobbyHelper(thisptr, p1, p2);
	return v;
}

bool initdone = false;
uintptr_t WINAPI InitDetour(const struct DllNotificationData* Data)
{
	if (initdone)
		return 0;
	if (const auto module = pe::get_module()) {
		initdone = true;
		Data->Detours->TransactionBegin();
		Data->Detours->UpdateThread(NtCurrentThread());

		uintptr_t handle = module->handle();
		const auto sections2 = module->segments();
		const auto& s2 = std::find_if(sections2.begin(), sections2.end(), [](const IMAGE_SECTION_HEADER& x) {
			return x.Characteristics & IMAGE_SCN_CNT_CODE;
			});
		const auto data = s2->as_bytes();
#ifdef _M_X64
		// module->rva_to<std::remove_pointer_t might be uint32_t only? will be a problem for future BNSR support
		uintptr_t aPinInput = (uintptr_t)&std::search(data.begin(), data.end(), pattern_searcher(xorstr_("4C 8B 0B 41 B0 01 8B 96 ?? 01 00 00 48 8B CB 41 FF 51 30 48 8B 03 41 B0 01 8B 96 ?? 01 00 00 48 8B CB FF 50 28 45 33 F6 41 8B EE")))[0] - 0x5C; // EU64
		oPinInput = module->rva_to<std::remove_pointer_t<decltype(oPinInput)>>(aPinInput - handle);
		Data->Detours->Attach(&(PVOID&)oPinInput, &hkPinInput);

		uintptr_t aPinSubmit = (uintptr_t)&std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 8B C4 57 41 54 41 55 48 81 EC 50 01 00 00 48 C7 44 24 48 FE FF FF FF 48 89 58 08 48 89 68 10 48 89 70 18 48 8B D9")))[0]; // EU64
		oPinSubmit = module->rva_to<std::remove_pointer_t<decltype(oPinSubmit)>>(aPinSubmit - handle);
		Data->Detours->Attach(&(PVOID&)oPinSubmit, &hkPinSubmit);

		uintptr_t aRequestLogin = (uintptr_t)&std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 8B C4 41 55 41 56 41 57 48 81 EC 80 00 00 00 48 C7 44 24 40 FE FF FF FF")))[0]; // EU64
		oRequestLogin = module->rva_to<std::remove_pointer_t<decltype(oRequestLogin)>>(aRequestLogin - handle);
		Data->Detours->Attach(&(PVOID&)oRequestLogin, &hkRequestLogin);

		uintptr_t aLobbyHelper = (uintptr_t)&std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 89 5C 24 08 57 48 83 EC 20 48 8B FA 48 8B D9 FF 15 ???????? 48 85 C0 4C 8B D8 0F 84 ???????? 48 85 FF 0F 84")))[0]; // EU64
		oLobbyHelper = module->rva_to<std::remove_pointer_t<decltype(oLobbyHelper)>>(aLobbyHelper - handle);
		Data->Detours->Attach(&(PVOID&)oLobbyHelper, &hkLobbyHelper);

		uintptr_t aLogin = (uintptr_t)&std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 83 EC 58 48 83 3D ???????? 08 4C 89 44 24 40 44 0FB7 05 ???????? 48 8B C2 4C 8D 0D ???????? 48 8D 0D ???????? 4C 0F43 0D ????????")))[0]; // EU64
		oLogin = module->rva_to<std::remove_pointer_t<decltype(oLogin)>>(aLogin - handle);
		Data->Detours->Attach(&(PVOID&)oLogin, &hkLogin);
#else
		auto itPinInput = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("8B 8F ?? 01 00 00 8B 06 8B 50 18 6A 01 51 8B CE FF D2 8B 8F ?? 01 00 00 8B 06 8B 50 14 6A 01 51 8B CE FF D2 C7 44 24 10 00 00 00 00"))); // EU32
		uintptr_t aPinInput = NULL;
		if (itPinInput != data.end()) {
			aPinInput = (uintptr_t)&itPinInput[0] - 0x5D; // EU32
		}
		else {
			itPinInput = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("00 00 8B 06 8B 50 14 6A 01 51 8B CE FF D2 8B 8F F0 00 00 00 8B 06 8B 50 10 6A 01 51 8B CE FF D2 33 DB 8D 6F 4C 3B 9F FC 00 00 00 8B 4D 00"))); // CN175
			if (itPinInput != data.end())
				aPinInput = (uintptr_t)&itPinInput[0] - 0x5C; // Not tested, don't know any CN175 servers with PIN
		}
		aPinInputTarget = aPinInput + 0x6;
		Detour32((char*)aPinInput, (char*)hkPinInputAsm, 6);

		uintptr_t aPinSubmit = (uintptr_t)&std::search(data.begin(), data.end(), pattern_searcher(xorstr_("33 C4 89 84 24 40 01 00 00 53 55 56 A1")))[0] - 0x19; // EU32, CN175
		oPinSubmit = reinterpret_cast<decltype(oPinSubmit)>(aPinSubmit);
		Data->Detours->Attach(&(PVOID&)oPinSubmit, &hkPinSubmit);

		auto itRequestLogin = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("33 C4 50 8D 44 24 58 64 A3 00 00 00 00 8B 44 24 70 8B 5C 24"))); // EU32
		uintptr_t aRequestLogin = NULL;
		if (itRequestLogin != data.end()) {
			aRequestLogin = (uintptr_t)&itRequestLogin[0] - 0x25; // EU32, 0x00CD2D60
			oRequestLogin = reinterpret_cast<decltype(oRequestLogin)>(aRequestLogin);
			Data->Detours->Attach(&(PVOID&)oRequestLogin, &hkRequestLogin);
		}
		else {
			itRequestLogin = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("33 C4 50 8D 44 24 54 64 A3 00 00 00 00 8B 44 24 6C 8B 6C 24 78 8B F1 85 F6 89 44 24 18 8B 44 24 70 89 44 24 14 8B FA"))); // CN175
			if (itRequestLogin != data.end()) {
				aRequestLogin = (uintptr_t)&itRequestLogin[0] - 0x24; // CN175, 0x00D27930
				oRequestLoginPS = reinterpret_cast<decltype(oRequestLoginPS)>(aRequestLogin);
				Data->Detours->Attach(&(PVOID&)oRequestLoginPS, &hkRequestLoginPS);
			}
		}

		uintptr_t aLobbyHelper = (uintptr_t)&std::search(data.begin(), data.end(), pattern_searcher(xorstr_("56 57 8B F1 FF 15 ???????? 8B C8 85 C9 0F 84 ???????? 8B 7C 24 0C 85 FF")))[0]; // EU32, CN175
		oLobbyHelper = reinterpret_cast<decltype(oLobbyHelper)>(aLobbyHelper);
		Data->Detours->Attach(&(PVOID&)oLobbyHelper, &hkLobbyHelper);

		auto itLogin = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("8B 0D ???????? BA 08 00 00 00 39 15 ???????? 73 05 B9 ???????? 39 15 ???????? A1 ???????? 73 05 B8 ???????? 39 15 ???????? 8B 15 ???????? 73 05"))); // EU32
		uintptr_t aLogin = NULL;
		if (itLogin != data.end()) {
			aLogin = (uintptr_t)&itLogin[0];
		}
		else {
			itLogin = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("8B 15 ???????? B9 08 00 00 00 39 0D ???????? 73 05 BA ???????? 39 0D ???????? A1 ???????? 73 05"))); // CN175
			if (itLogin != data.end()) {
				aLogin = (uintptr_t)&itLogin[0];
			}
		}
		oLogin = reinterpret_cast<decltype(oLogin)>(aLogin);
		//Data->Detours->Attach(&(PVOID&)oLogin, &hkLogin); // Empty hook

		// Disable login fail increment, not worth it
		/*auto itLoginIncrement = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("83 47 68 01 6A 0A")));
		char* LoginIncrement = NULL;
		if (itLoginIncrement != data.end()) {
			LoginIncrement = (char*)((uintptr_t)&itLoginIncrement[0] + 0x3);
			*LoginIncrement = 0x0;
		}*/
#endif

		// Offsets based on version
		const VS_FIXEDFILEINFO* vsf;
		if (GetModuleVersionInfo(nullptr, L"\\", &(LPCVOID&)vsf) >= 0) {
			int v1 = HIWORD(vsf->dwProductVersionLS);
			int v2 = LOWORD(vsf->dwProductVersionLS);
			if (v1 > 399 || v2 > 9121) {
#ifdef _M_X64
				pinClassOffset = 6 * 8;
#else
				pinClassOffset = 6 * 4;
#endif
			}
		}

		Data->Detours->TransactionCommit();

		std::wstring args = GetCommandLineW();
		std::wregex regex_cmd(L" -KEEPPIN(?:$| )", std::regex_constants::icase);
		std::wsmatch matches_cmd;
		if (std::regex_search(args, matches_cmd, regex_cmd))
			keeppin = true;
	}
	return 0;
}


BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls(hInstance);
	return TRUE;
}

void __cdecl DllLoadedNotification(const struct DllNotificationData* Data, void* Context) {
	const wchar_t* fileName;
	if (GetModuleVersionInfo(Data->BaseOfImage, xorstr_(L"\\StringFileInfo\\*\\OriginalFilename"), &(LPCVOID&)fileName) >= 0) {
		switch (fnv1a::make_hash(fileName, towlower)) {
		case L"PSAPI"_fnv1al:
		// case L"iconv.dll"_fnv1al:
			InitDetour(Data); // Could be separate thread since it's a lot of pattern scanning
			break;
		}
	}
}

void __cdecl InitNotification(const struct InitNotificationData* Data, void* Context) {
	const wchar_t* fileName;
	if (GetModuleVersionInfo(nullptr, xorstr_(L"\\StringFileInfo\\*\\OriginalFilename"), &(LPCVOID&)fileName) >= 0) {
		switch (fnv1a::make_hash(fileName, towlower)) {
		case L"client.exe"_fnv1al:
			AllocConsole();
			freopen("CONOUT$", "w", stdout);
			break;
		}
	}
}

#ifndef __DATEW__
#define __DATEW__ _CRT_WIDE(__DATE__)
#endif

__declspec(dllexport)
void __cdecl GetPluginInfo2(PluginInfo2* plgi)
{
	static std::once_flag once_flag;
	static auto name = xorstr(L"loginhelper");
	static auto version = xorstr(__DATEW__);
	static auto description = xorstr(L"Allows automatic login+pin and makes pin input easier");

	std::call_once(once_flag, [](auto& name, auto& version, auto& description) {
		name.crypt();
		version.crypt();
		description.crypt();
		}, name, version, description);

	plgi->Name = name.get();
	plgi->Version = version.get();
	plgi->Description = description.get();
	//plgi->InitNotification = &InitNotification;
	plgi->DllLoadedNotification = &DllLoadedNotification;
}