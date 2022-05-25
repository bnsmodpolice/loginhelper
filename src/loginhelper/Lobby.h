#pragma once

std::wstring m_account;
std::wstring m_password;
int loggedin = 0;
bool GetLogin()
{
	// std::wcout << L"Loginhelper: GetLogin (" << loggedin << L")" << std::endl;
	if (loggedin != 0)
		return loggedin;
	loggedin = 2;
	// Environment Variables
	WCHAR mailchar[256];
	if (GetEnvironmentVariableW(L"BNS_PROFILE_USERNAME", &mailchar[0], 256)) {
		m_account = mailchar;
		// std::wcout << L"Loginhelper: BNS_PROFILE_USERNAME " << m_account << std::endl;
		WCHAR passchar[256];
		SetEnvironmentVariableW(L"BNS_PROFILE_USERNAME", L"");
		if (GetEnvironmentVariableW(L"BNS_PROFILE_PASSWORD", &passchar[0], 256)) {
			m_password = passchar;
			// std::wcout << L"Loginhelper: BNS_PROFILE_USERNAME " << m_password << std::endl;
			SetEnvironmentVariableW(L"BNS_PROFILE_PASSWORD", L"");
			return true;
		}
		else {
			loggedin = -1;
			return false;
		}
	}
	// std::wcout << L"Loginhelper: No env var " << m_password << std::endl;
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
			loggedin = -1;
			return false;
		}
	}
	loggedin = -1;
	return false;
}
LoginHelperData lhdata;
bool GetServer()
{
	//std::wcout << L"GetServer" << std::endl;
	lhdata.npPort = -1;
	lhdata.npProgramId = -1;
	lhdata.lobbyPort = -1;
	bool has_any = false;
	WCHAR EnvVarW[256];
	if (GetEnvironmentVariableW(L"BNS_SERVER_NP_ADDRESS", &EnvVarW[0], 256)) {
		lhdata.npAddress = EnvVarW;
		has_any = true;
	}
	if (GetEnvironmentVariableW(L"BNS_SERVER_NP_PORT", &EnvVarW[0], 256)) {
		lhdata.npPort = std::stoi(EnvVarW, nullptr);
has_any = true;
	}
	if (GetEnvironmentVariableW(L"BNS_SERVER_NP_PROGRAMID", &EnvVarW[0], 256)) {
		lhdata.npProgramId = std::stoi(EnvVarW, nullptr);
		has_any = true;
	}
	if (GetEnvironmentVariableW(L"BNS_SERVER_NP_GAMECODE", &EnvVarW[0], 256)) {
		lhdata.npGameCode = EnvVarW;
		has_any = true;
	}
	if (GetEnvironmentVariableW(L"BNS_SERVER_LOBBY_ADDRESS", &EnvVarW[0], 256)) {
		lhdata.lobbyAddress = EnvVarW;
		has_any = true;
	}
	if (GetEnvironmentVariableW(L"BNS_SERVER_LOBBY_PORT", &EnvVarW[0], 256)) {
		lhdata.lobbyPort = std::stoi(EnvVarW, nullptr);;
		has_any = true;
	}
	return has_any;
}

bool firstrun = true;
bool swap_programid = false;
struct Lobby;
bool(__fastcall* oLobby_RequestLogin)(Lobby*, const wchar_t*, unsigned __int16, int, const wchar_t*, const wchar_t*, unsigned __int16, const wchar_t*, const wchar_t*);
bool __fastcall hkLobby_RequestLogin(
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
#ifdef DEBUG
	std::wcout << L"RequestLogin BEFORE" << std::endl;
	std::wcout << L"npAddress" << L": " << npAddress << std::endl;
	std::wcout << L"account" << L": " << account << std::endl;
	std::wcout << L"npPort" << L": " << npPort << std::endl;
	std::wcout << L"npProgramId" << L": " << npProgramId << std::endl;
	std::wcout << L"npGameCode" << L": " << npGameCode << std::endl;
	std::wcout << L"lobbyAddress" << L": " << lobbyAddress << std::endl;
	std::wcout << L"lobbyPort" << L": " << lobbyPort << std::endl;
	std::wcout << L"password" << L": " << password << std::endl;
#endif

	firstrun = false;

	if (swap_programid) {
		switch (npProgramId) {
		case 302:
			npProgramId = 301;
			break;
		case 306:
			npProgramId = 321;
			break;
		case 342:
			npProgramId = 346;
			break;
		case 343:
			npProgramId = 347;
			break;
		}
	}
	if (GetServer()) {
		if (!lhdata.npAddress.empty())
			npAddress = lhdata.npAddress.c_str();
		if (lhdata.npPort != -1)
			npPort = lhdata.npPort;
		if (lhdata.npProgramId != -1)
			npProgramId = lhdata.npProgramId;
		if (!lhdata.npGameCode.empty())
			npGameCode = lhdata.npGameCode.c_str();
		if (!lhdata.lobbyAddress.empty())
			lobbyAddress = lhdata.lobbyAddress.c_str();
		if (lhdata.lobbyPort != -1)
			lobbyPort = lhdata.lobbyPort;
	}

#ifdef DEBUG
	std::wcout << L"RequestLogin AFTER" << std::endl;
	std::wcout << L"npAddress" << L": " << npAddress << std::endl;
	std::wcout << L"account" << L": " << account << std::endl;
	std::wcout << L"npPort" << L": " << npPort << std::endl;
	std::wcout << L"npProgramId" << L": " << npProgramId << std::endl;
	std::wcout << L"npGameCode" << L": " << npGameCode << std::endl;
	std::wcout << L"lobbyAddress" << L": " << lobbyAddress << std::endl;
	std::wcout << L"lobbyPort" << L": " << lobbyPort << std::endl;
	std::wcout << L"password" << L": " << password << std::endl;
#endif

	return oLobby_RequestLogin(thisptr, npAddress, npPort, npProgramId, npGameCode, lobbyAddress, lobbyPort, account, password);
}

void(__fastcall* oTitleLoginPanel_processLogin)(TitleLoginPanel*);
void __fastcall hkTitleLoginPanel_processLogin(TitleLoginPanel* thisptr)
{
#ifdef DEBUG
	std::wcout << L"Loginhelper: oTitleLoginPanel_processLogin " << thisptr << std::endl;
#endif
	oTitleLoginPanel_processLogin(thisptr);
}

struct EngineEvent;
bool(__fastcall* oTitleLoginPanel_ProcessEvent)(TitleLoginPanel*, EngineEvent*);
bool __fastcall hkTitleLoginPanel_ProcessEvent(TitleLoginPanel* thisptr, EngineEvent* _event)
{
#ifdef DEBUG
	std::wcout << L"Loginhelper: oTitleLoginPanel_ProcessEvent " << thisptr << std::endl;
#endif
	if (firstrun) {
		pincount = 0;
#ifdef DEBUG
		std::wcout << L"Loginhelper: oTitleLoginPanel_ProcessEvent calling oTitleLoginPanel_processLogin" << std::endl;
#endif
		oTitleLoginPanel_processLogin(thisptr);
#ifdef DEBUG
		std::wcout << L"Loginhelper: oTitleLoginPanel_ProcessEvent done oTitleLoginPanel_processLogin" << std::endl;
#endif
	}
	auto v = oTitleLoginPanel_ProcessEvent(thisptr, _event);
	return v;
}

UIEngineInterface* UIEngineInterfaceInstance;
UIEngineInterface** (__fastcall** oUIEngineInterfaceGetInstance)();

// hkTitleLoginPanel_OpenLoginPanel
void(__fastcall* oTitleLoginPanel_OpenLoginPanel)(TitleLoginPanel*);
void __fastcall hkTitleLoginPanel_OpenLoginPanel(TitleLoginPanel* thisptr_src)
{
	TitleLoginPanel* thisptr_offset = (TitleLoginPanel*)((uintptr_t)thisptr_src + UIPanelOffset);
	oTitleLoginPanel_OpenLoginPanel(thisptr_src);
#ifdef DEBUG
	std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel" << std::endl;
	std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel n1 " << std::endl;
	std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel calling " << std::hex << *oUIEngineInterfaceGetInstance << std::endl;
#endif
	UIEngineInterfaceInstance = *(*oUIEngineInterfaceGetInstance)();
#ifdef DEBUG
	std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel UIEngineInterfaceInstance " << UIEngineInterfaceInstance << std::endl;
	std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel _userIdEditBoxID " << thisptr_offset->_userIdEditBoxID << std::endl;
	std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel _userPasswordEditBoxID " << thisptr_offset->_userPasswordEditBoxID << std::endl;

	std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel login set!" << std::endl;
#endif
	if (GetLogin())
	{
#ifdef DEBUG
		std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel GetLogin success!" << std::endl;
#endif
		uintptr_t UIEngineInterfaceOffset = 0;
		if (UIPanelOffset > 0)
			UIEngineInterfaceOffset = 8 * 3;
#ifdef DEBUG
		std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel UIEngineInterfaceInstance " << std::hex << (uintptr_t)UIEngineInterfaceInstance << std::endl;
		std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel UIEngineInterfaceInstance+Offset " << std::hex << ((uintptr_t)UIEngineInterfaceInstance + UIEngineInterfaceOffset) << std::endl;
		std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel setting _userIdEditBoxID" << std::endl;
#endif
		((UIEngineInterface*)((uintptr_t)UIEngineInterfaceInstance + UIEngineInterfaceOffset))->SetTextValue(UIEngineInterfaceInstance, thisptr_offset->_userIdEditBoxID, m_account.c_str());
#ifdef DEBUG
		std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel setting _userPasswordEditBoxID" << std::endl;
#endif
		((UIEngineInterface*)((uintptr_t)UIEngineInterfaceInstance + UIEngineInterfaceOffset))->SetTextValue(UIEngineInterfaceInstance, thisptr_offset->_userPasswordEditBoxID, m_password.c_str());
	}
	else {
#ifdef DEBUG
		std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel GetLogin fail." << std::endl;
#endif
	}

#ifdef DEBUG
	std::wcout << L"Loginhelper: oTitleLoginPanel_OpenLoginPanel done" << std::endl;
#endif
	return;
}