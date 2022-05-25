#pragma once

int pincode[6] = { -1 };
bool keeppin = false;
std::wstring pintext;
int pincount = 0;
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
	if (GetEnvironmentVariableW(L"BNS_PROFILE_PIN", &pinchar[0], 256)) {
		pintext = pinchar;
		for (int i = 0; i < 6; i++) {
			pincode[i] = pintext[i] - '0';
		}
		if (!keeppin)
			SetEnvironmentVariableW(L"BNS_PROFILE_PIN", L"");
		return true;
	}

	CHAR otpsecret[2048];
	if (GetEnvironmentVariableA("BNS_PROFILE_OTP_SECRET", &otpsecret[0], 2048)) {
		std::string otpsecrettext = otpsecret;

		std::string normalizedKey = normalizedBase32String(otpsecrettext);
		CppTotp::Bytes::ByteString qui = CppTotp::Bytes::fromUnpaddedBase32(normalizedKey);

		uint32_t p = CppTotp::totp(qui, time(NULL), 0, 30, 6);

		char buffer[7];
		sprintf_s(buffer, "%06d", p);

		for (int i = 0; i < 6; i++) {
			pincode[i] = buffer[i] - '0';
		}

		if (!keeppin)
			SetEnvironmentVariableW(L"BNS_PROFILE_OTP_SECRET", L"");
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
	return false;
}

bool(__stdcall* oLoginOtpPanel_processConfirmClick)(LoginOtpPanel*);
bool __stdcall hkLoginOtpPanel_processConfirmClick(LoginOtpPanel* panel)
{
#ifdef DEBUG
	std::wcout << L"hkLoginOtpPanel_processConfirmClick called!" << std::endl;
#endif
	pincount = 0;
	return oLoginOtpPanel_processConfirmClick(panel);
}

LoginOtpPanel* g_panel;

bool(__thiscall* oLoginOtpPanel_updateVerifyMode)(LoginOtpPanel*);
bool __fastcall hkLoginOtpPanel_updateVerifyMode(LoginOtpPanel* panel_src)
{
	LoginOtpPanel* panel_offset = (LoginOtpPanel*)((uintptr_t)panel_src + UIPanelOffset);
#ifdef DEBUG
	std::wcout << L"hkLoginOtpPanel_updateVerifyMode panel " << std::hex << panel_src << std::endl;
	//system("pause");
	std::wcout << L"hkLoginOtpPanel_updateVerifyMode panel->_keyboardEnabled " << std::hex << panel_offset->_keyboardEnabled << std::endl;
#endif
	panel_offset->_keyboardEnabled = true;
	for (int i = 0; i < 10; i++) {
		//std::wcout << L"hkLoginOtpPanel_updateVerifyMode panel->_otpNumpadList[i].numpadValue " << std::hex << panel->_otpNumpadList[i].numpadValue << std::endl;
		panel_offset->_otpNumpadList[i].numpadValue = i;
	}
	pincount++;
#ifdef DEBUG
	std::wcout << L"hkLoginOtpPanel_updateVerifyMode panel->_verifyFailCount " << std::hex << panel_offset->_verifyFailCount << std::endl;
	std::wcout << L"hkLoginOtpPanel_updateVerifyMode panel->_curInputCount " << std::hex << panel_offset->_curInputCount << std::endl;
	std::wcout << L"hkLoginOtpPanel_updateVerifyMode pincount " << std::hex << pincount << std::endl;
#endif
	if (panel_offset->_verifyFailCount == 0 && pincount - 1 == 0) {
		//system("pause");
		if (GetPin()) {
			for (int i = 0; i < 6; i++)
			{
#ifdef DEBUG
				std::wcout << L"hkLoginOtpPanel_updateVerifyMode panel->_otpInputList[i].inputBtnIndex " << std::hex << panel_offset->_otpInputList[i].inputBtnIndex << std::endl;
				std::wcout << L"hkLoginOtpPanel_updateVerifyMode pincode[i] " << std::hex << pincode[i] << std::endl;
#endif
				panel_offset->_otpInputList[i].inputBtnIndex = pincode[i];
			}
#ifdef DEBUG
			std::wcout << L"hkLoginOtpPanel_updateVerifyMode panel->_curInputCount " << std::hex << panel_offset->_curInputCount << std::endl;
#endif
			panel_offset->_curInputCount = 6;
			hkLoginOtpPanel_processConfirmClick(panel_src);
		}
	}
	if (panel_offset->_verifyFailCount > 4)
		panel_offset->_verifyFailCount = 1;
#ifdef DEBUG
	std::wcout << L"hkLoginOtpPanel_updateVerifyMode Done! finishing call" << std::endl;
#endif
	//system("pause");
	bool v = oLoginOtpPanel_updateVerifyMode(panel_src);
	//CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)DoPin, nullptr, 0, nullptr));
	return v;
}