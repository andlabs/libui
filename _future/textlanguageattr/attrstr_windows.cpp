	WCHAR *localeName;

	// locale names are specified as BCP 47: https://msdn.microsoft.com/en-us/library/windows/desktop/dd373814(v=vs.85).aspx https://www.ietf.org/rfc/rfc4646.txt
	case uiAttributeLanguage:
		localeName = toUTF16((char *) (spec->Value));
		hr = p->layout->SetLocaleName(localeName, range);
		if (hr != S_OK)
			logHRESULT(L"error applying locale name attribute", hr);
		uiFree(localeName);
		break;