// pulse-filetree-generator.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <shlobj.h>
#include <tchar.h>
#include <locale.h>
#include <winbase.h>
#include <string>
#include <vector>
#include <set>
#include <sstream>

#include <algorithm>

WIN32_FIND_DATA data;
//using namespace std;

inline std::wstring wtrim_right_copy(
	const std::wstring& s,
	const std::wstring& delimiters = L" \f\n\r\t\v")
{
	return s.substr(0, s.find_last_not_of(delimiters) + 1);
}

inline std::wstring wtrim_left_copy(
	const std::wstring& s,
	const std::wstring& delimiters = L" \f\n\r\t\v")
{
	return s.substr(s.find_first_not_of(delimiters));
}

inline std::wstring wtrim_copy(
	const std::wstring& s,
	const std::wstring& delimiters = L" \f\n\r\t\v")
{
	return wtrim_left_copy(wtrim_right_copy(s, delimiters), delimiters);
}

inline bool is_in_vector(
	std::vector<std::wstring> &parts,
	const std::wstring& value)
{
	std::vector< std::wstring >::iterator it = std::find(parts.begin(), parts.end(), value);
	if (it != parts.end()) {
		return true;
	}
	return false;
}

inline bool is_in_set(
	std::set<std::wstring> &parts,
	const std::wstring& value)
{
	std::set< std::wstring >::iterator it = std::find(parts.begin(), parts.end(), value);
	if (it != parts.end()) {
		return true;
	}
	return false;
}

void listdir(const wchar_t dir[], FILE * fp, std::set<std::wstring> &parts) {
	wchar_t nom[MAX_PATH];
	wcscpy_s(nom, dir);
	wcscat_s(nom, L"\\*");
	wchar_t espacea[5];
	wcscpy_s(espacea, L" {");
	HANDLE h = FindFirstFile(nom, &data);
	if (h == INVALID_HANDLE_VALUE) { fwprintf_s(stderr, L" Error %s", nom); return; }
	h = FindFirstFile(nom, &data);
	do { // folders
		if (FILE_ATTRIBUTE_DIRECTORY&data.dwFileAttributes) {
			if (data.cFileName[0] == '.') continue; // �vite "." et ".."
			//SetConsoleTextAttribute(hConsole, 12);
			//wprintf(L"%s\"text\" : \"%s\", \"children\" : [", espacea, data.cFileName);
			fwprintf_s(fp, L"%s\"text\" : \"%s\", \"children\" : [", espacea, data.cFileName);
			wcscpy_s(espacea, L",{");
			//wprintf(L"%s\\\n",  data.cFileName);
			wcscpy_s(nom, dir);
			wcscat_s(nom, L"\\");
			wcscat_s(nom, data.cFileName);
			std::wstring nomwstring = std::wstring(nom);
			// voir si nom est dans la liste des exclusions.
			std::set<std::wstring>::iterator it = parts.find(nomwstring);
			const bool is_in = (it != parts.end());
			if (it != parts.end()) { 
				// on supprime de la list des exclusions
				parts.erase(it); 
			}
			//if (wcsncmp(nom, L"c:\\\\Windows\\\\", 8) != 0)
			if (!is_in )
				listdir(nom, fp, parts); // folder
			//wprintf( L"]" );
			fwprintf_s(fp, L"]}");
		}
	} while (FindNextFile(h, &data));
	FindClose(h);
}

bool dirExists(_TCHAR* dirName_in)
{
	DWORD dwAttrib = GetFileAttributes((LPCWSTR)dirName_in);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
int _tmain(int argc, _TCHAR* argv[])
{
	//std::vector<std::wstring> parts;
	//std::vector< std::wstring >::iterator it;
	std::set<std::wstring> parts;
	std::set< std::wstring >::iterator it;
	wchar_t temp[MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t file[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	size_t driveNumberOfElements = _MAX_DRIVE, dirNumberOfElements= _MAX_DIR, nameNumberOfElements=_MAX_FNAME, extNumberOfElements= _MAX_PATH;
	size_t ReturnValue;

	char outfilename[MAX_PATH];
	char envnom[MAX_PATH];
	FILE * fp;

	setlocale(LC_ALL, "en_US.utf8");

	if (argc == 1) {
		fwprintf_s(stderr, L"usage : %s \"dir_start\" [file result json]", argv[0]);
		return -1;
	}

	if (!dirExists(argv[1])) {
		fwprintf(stderr, L"\nerror arg1 (%s) is not folder\n", argv[1]);
		return -1;
	}

	if (argc < 3) {
		getenv_s(&ReturnValue, envnom, MAX_PATH, "TMP");
		strcpy_s(outfilename, MAX_PATH, envnom);
		strcat_s(outfilename, MAX_PATH, "\\file.txt");
	}
	else {
		char buffer[MAX_PATH];
		int ret = wcstombs_s(&ReturnValue, buffer, MAX_PATH, argv[2], MAX_PATH);
		strcpy_s(outfilename, MAX_PATH, buffer);
	}
	
	//lit fichier init pour voir si il y a des exclusion str separe par des virgule.
	GetPrivateProfileString(
		_T("browserfile"),
		_T("listexclude"),
		_T(""),
		(LPWSTR)temp,
		sizeof(temp),
		_T("C:\\Program Files\\Pulse\\etc\\agentconf.ini"));
	
	printf("\nfile result in %s\n", outfilename);
	std::wstring stringdata(temp), tempstr;
	std::wstringstream wss(temp);
	while (std::getline(wss, tempstr, L','))//std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
		//parts.push_back(wtrim_copy(tempstr)); // pour vector
		parts.insert(wtrim_copy(tempstr)); //pour set
	//affiche vector des exclusion
	for (it = parts.begin(); it != parts.end(); ++it)
		std::wcout << *it << std::endl;
	
	// vector  part contient les exclusions.

	printf("\file result in %s", outfilename);
	errno_t err = fopen_s(&fp, (char*)outfilename, "w+");
	_wsplitpath_s(argv[1], drive, driveNumberOfElements, dir, dirNumberOfElements, file, nameNumberOfElements, ext, extNumberOfElements);
	fwprintf_s(fp, L"{\"text\" : \"%s\", \"children\" : [", file);
	listdir(argv[1], fp, parts);
	fprintf(fp, "]}");
	fclose(fp);
	//fwprintf(stderr, L"\n arg1 (%s)\n", file);

	for (it = parts.begin(); it != parts.end(); ++it)
		std::wcout << *it << std::endl;

}
