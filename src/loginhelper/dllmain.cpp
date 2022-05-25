#include <pe/module.h>
#include <fnv1a.h>
#include <xorstr.hpp>
#include <regex>
#include "versioninfo.h"
#include "detours.h"
#include "pluginsdk.h"
#include "searchers.h"
#include <iostream>
//#define DEBUG
#include "Data.h"
#include "totp.h"
#include "LoginOtp.h"
#include "Lobby.h"
#ifdef DEBUG
#include "Quit.h"
#endif
#include "memset.h"

#ifdef _M_X64
#define thiscall_(name, thisarg, ...) name(thisarg, ## __VA_ARGS__) 
#else
#define thiscall_(name, thisarg, ...) __fastcall name(thisarg, intptr_t vv, ## __VA_ARGS__) 
#endif
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))

bool initdone = false;
void __cdecl oep_notify(const Version version)
{
	if (initdone)
		return;
	if (const auto module = pe::get_module()) {
		initdone = true;
		DetourTransactionBegin();
		DetourUpdateThread(NtCurrentThread());

		uintptr_t handle = module->handle();
		const auto sections2 = module->segments();
		const auto& s2 = std::find_if(sections2.begin(), sections2.end(), [](const IMAGE_SECTION_HEADER& x) {
			return x.Characteristics & IMAGE_SCN_CNT_CODE;
			});
		const auto data = s2->as_bytes();

		// For testing on weird clients
		for (auto pair : memsetplist) {
			pair.findoffset(handle, data);
			if (pair.handle_offset == 0) {
				//std::cout << "Pattern not found: " << pair.pattern << std::endl;
				continue;
			}
			//std::cout << (void*)pair.getoffset(handle) << " : " << pair.pattern << std::endl;
			//setaddr(pair.getoffset(handle), &pair.bytearray[0], pair.size());
		}

		int matchpin = 0;
		int matchlobby = 0;

#ifdef DEBUG
		auto start = std::chrono::high_resolution_clock::now();
#endif

		// Above Text.Otp.Numpad.EvenNum / Mark(2x) / CompletedMark(2x)
		auto result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 8B C4 56 57 41 55 41 56 41 57 48 81 EC 80 00 00 00 48 C7 40 98 FE FF FF FF 48 89 58 10 48 89 68 18 0F 29 70 C8")));
		if (result != data.end()) {
			matchpin++;
			uintptr_t aLoginOtpPanel_updateVerifyMode = (uintptr_t)&result[0];
#ifdef DEBUG
			std::wcout << L"aLoginOtpPanel_updateVerifyMode" << L": " << std::hex << aLoginOtpPanel_updateVerifyMode << std::endl;
#endif
			oLoginOtpPanel_updateVerifyMode = module->rva_to<std::remove_pointer_t<decltype(oLoginOtpPanel_updateVerifyMode)>>(aLoginOtpPanel_updateVerifyMode - handle);
			DetourAttach(&(PVOID&)oLoginOtpPanel_updateVerifyMode, &hkLoginOtpPanel_updateVerifyMode);
		}

		// UE3: First call above Text.Otp.HelpMsg.UseKeyboard, call is followed by a jump
		// UE4: Above 3B 10 74 11 FF C7 48 FF C1 48 83 C0 08 48 83 F9 0A 7C ED EB 12
		// Was wrong
		// UE4: Above 83 BB 98 02 00 00 00 ?? ?? ?? ?? ?? ?? 83 BB 9C 02 00 00 00
		// or 
		result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("83 BB ?? ?? 00 00 00 0F8E ?? ?? ?? ?? 83 BB ?? ?? ?? ?? 00 0F8E ?? ?? ?? ?? E8 ?? ?? ?? ??")));
		if (result != data.end()) {
			matchpin++;
			uintptr_t aLoginOtpPanel_processConfirmClick = (uintptr_t)&result[0] - 0x8B;
			uint32_t offset = *(uint32_t*)((uintptr_t)&result[0] + 2);
			UIPanelOffset = offset - 0x0298;
#ifdef DEBUG
			std::wcout << L"aLoginOtpPanel_processConfirmClick" << L": " << std::hex << aLoginOtpPanel_processConfirmClick << std::endl;
			std::wcout << L"UIPanelOffset" << L": " << std::hex << UIPanelOffset  << std::endl;
#endif
			oLoginOtpPanel_processConfirmClick = module->rva_to<std::remove_pointer_t<decltype(oLoginOtpPanel_processConfirmClick)>>(aLoginOtpPanel_processConfirmClick - handle);
			DetourAttach(&(PVOID&)oLoginOtpPanel_processConfirmClick, &hkLoginOtpPanel_processConfirmClick);
		}
#ifdef DEBUG
		std::wcout << L"UIPanelOffset" << L": " << std::hex << UIPanelOffset << std::endl;
#endif

		
		// Refs to static address containing lobby/cligate address etc followed by single call to RequestLogin
		// Also linked under authfail_invalid_account/authfail_invalid_password/connecting surrounded by qword calls
		// On BNSR it goes to RequestLogin directly
		// -0x5E 0F 84 F0 03 00 00 33 F6 66 41 3B F0 0F 84 E4 03 00 00 4D 85 ED 0F 84 DB 03 00 00 66 3B 75 70 0F 84 D1 03 00 00 48 85 DB 0F 84 C8 03 00 00 48 85 C0 0F 84 BF 03 00 00
		result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("0F 84 F0 03 00 00 33 F6 66 41 3B F0 0F 84 E4 03 00 00 4D 85 ED 0F 84 DB 03 00 00 66 3B 75 70 0F 84 D1 03 00 00 48 85 DB 0F 84 C8 03 00 00 48 85 C0 0F 84 BF 03 00 00")));
		if (result != data.end()) {
			matchlobby++;
			uintptr_t aLobby_RequestLogin = (uintptr_t)&result[0] - 0x5E;
#ifdef DEBUG
			std::wcout << L"aLobby_RequestLogin" << L": " << std::hex << aLobby_RequestLogin << std::endl;
#endif
			oLobby_RequestLogin = module->rva_to<std::remove_pointer_t<decltype(oLobby_RequestLogin)>>(aLobby_RequestLogin - handle);
			DetourAttach(&(PVOID&)oLobby_RequestLogin, &hkLobby_RequestLogin);
		}

		// One of 2 containing "lobby_account"
		result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 89 7C 24 18 41 56 48 83 EC 20 4C 8B F1 FF 15 ?? ?? ?? ?? 48 8B F8 48 85 C0 0F 84 E9 01 00 00")));
		if (result != data.end()) {
			matchlobby++;
			uintptr_t aTitleLoginPanel_OpenLoginPanel = (uintptr_t)&result[0];
#ifdef DEBUG
			std::wcout << L"aTitleLoginPanel_OpenLoginPanel" << L": " << std::hex << aTitleLoginPanel_OpenLoginPanel << std::endl;
#endif
			oTitleLoginPanel_OpenLoginPanel = module->rva_to<std::remove_pointer_t<decltype(oTitleLoginPanel_OpenLoginPanel)>>(aTitleLoginPanel_OpenLoginPanel - handle);
			DetourAttach(&(PVOID&)oTitleLoginPanel_OpenLoginPanel, &hkTitleLoginPanel_OpenLoginPanel);

			uintptr_t offset_addr = aTitleLoginPanel_OpenLoginPanel + 0x10;
			int offset = *(unsigned int*)offset_addr;
			oUIEngineInterfaceGetInstance = module->rva_to<std::remove_pointer_t<decltype(oUIEngineInterfaceGetInstance)>>((offset_addr + offset + 4) - handle);
#ifdef DEBUG
			std::wcout << L"oUIEngineInterfaceGetInstance" << L": " << std::hex << oUIEngineInterfaceGetInstance << std::endl;
#endif
		}

		// contains authfail_invalid_account/authfail_invalid_password/connecting
		result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 8B C4 55 57 41 56 48 8D 68 A8 48 81 EC 40 01 00 00 48 C7 44 24 68 FE FF FF FF")));
		if (result != data.end()) {
			matchlobby++;
			uintptr_t aTitleLoginPanel_processLogin = (uintptr_t)&result[0];
#ifdef DEBUG
			std::wcout << L"aTitleLoginPanel_processLogin" << L": " << std::hex << aTitleLoginPanel_processLogin << std::endl;
#endif
			oTitleLoginPanel_processLogin = module->rva_to<std::remove_pointer_t<decltype(oTitleLoginPanel_processLogin)>>(aTitleLoginPanel_processLogin - handle);
			//DetourAttach(&(PVOID&)oTitleLoginPanel_processLogin, &hkTitleLoginPanel_processLogin);
		}
		else
		{
			result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("33 DB 48 89 5D D7 4C 8B 08 4C 8D 45 D7 41 8B 94 24 ?? ?? 00 00 48 8B C8")));
			if (result != data.end()) {
				matchlobby++;
				uintptr_t aTitleLoginPanel_processLogin = (uintptr_t)&result[0] - 0x52;
#ifdef DEBUG
				std::wcout << L"aTitleLoginPanel_processLogin" << L": " << std::hex << aTitleLoginPanel_processLogin << std::endl;
#endif
				oTitleLoginPanel_processLogin = module->rva_to<std::remove_pointer_t<decltype(oTitleLoginPanel_processLogin)>>(aTitleLoginPanel_processLogin - handle);
				//DetourAttach(&(PVOID&)oTitleLoginPanel_processLogin, &hkTitleLoginPanel_processLogin);
			}
		}

		// calls TitleLoginPanel::processLogin
		result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 89 5C 24 08 57 48 83 EC 20 48 8B FA 48 8B D9 FF 15 ?? ?? ?? ?? 4C 8B C8")));
		if (result != data.end()) {
			matchlobby++;
			uintptr_t aTitleLoginPanel_ProcessEvent = (uintptr_t)&result[0];
#ifdef DEBUG
			std::wcout << L"aTitleLoginPanel_ProcessEvent" << L": " << std::hex << aTitleLoginPanel_ProcessEvent << std::endl;
#endif
			oTitleLoginPanel_ProcessEvent = module->rva_to<std::remove_pointer_t<decltype(oTitleLoginPanel_ProcessEvent)>>(aTitleLoginPanel_ProcessEvent - handle);
			DetourAttach(&(PVOID&)oTitleLoginPanel_ProcessEvent, &hkTitleLoginPanel_ProcessEvent);
		}

	


#ifdef DEBUG
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
#endif

#ifdef DEBUG
		std::wcout << L"Loginhelper: Hooked in " << duration << " ms" << std::endl;
		std::wcout << L"Loginhelper: Hooked PIN " << matchpin << std::endl;
		std::wcout << L"Loginhelper: Hooked Lobby " << matchlobby << std::endl;
#endif

		}

		DetourTransactionCommit();

		std::wstring args = GetCommandLineW();
		{
			std::wregex regex_cmd(L" -KEEPPIN(?:$| )", std::regex_constants::icase);
			std::wsmatch matches_cmd;
			if (std::regex_search(args, matches_cmd, regex_cmd))
				keeppin = true;
		}
		{
			WCHAR fixprogramid[256];
			if (GetEnvironmentVariableW(L"BNS_SERVER_FIXPROGRAMID", &fixprogramid[0], 256)) {
				if (std::stoi(fixprogramid) == 1)
					swap_programid = true;
			}
			if (!swap_programid) {
				std::wregex regex_cmd(L" -FIXPROGRAMID(?:$| )", std::regex_constants::icase);
				std::wsmatch matches_cmd;
				if (std::regex_search(args, matches_cmd, regex_cmd))
					swap_programid = true;
			}
		}
	}
	//system("pause");
	return;
}


BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls(hInstance);
	return TRUE;
}

bool  __cdecl init(const Version version)
{
#ifdef DEBUG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	std::wcout << L"Loginhelper: Init" << std::endl;
#endif
	return true;
}

extern "C" __declspec(dllexport) PluginInfo GPluginInfo = {
  .hide_from_peb = true,
  .erase_pe_header = true,
  .init = init,
  .oep_notify = oep_notify,
  .priority = 1,
  .target_apps = L"BNSR.exe"
};