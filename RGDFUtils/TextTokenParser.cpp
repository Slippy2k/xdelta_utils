#include "TextTokenParser.h"
#include <memory.h>
#include <assert.h>

static const int WORD_MAX_LENGTH = 512;
static const int STRING_MAX_LENGTH = 24 * 1024;
static const int NUMBER_MAX_LENGTH = 128;


CharacterSet::CharacterSet()
{
	Clear();
	return;
}

CharacterSet::~CharacterSet()
{
}

void CharacterSet::AddChar(unsigned char ch)
{
	m_uszChar[ch] = 1;
	return;
}

void CharacterSet::AddChar(unsigned char chMin, unsigned char chMax)
{
	for (int i = chMin; i <= chMax; i++)
		m_uszChar[i] = 1;
}

void CharacterSet::RemoveChar(unsigned char ch)
{
	m_uszChar[ch] = 0;
	return;
}

void CharacterSet::RemoveChar(unsigned char chMin, unsigned char chMax)
{
	for (int i = chMin; i <= chMax; i++)
		m_uszChar[i] = 0;
}

bool CharacterSet::IsHere(unsigned char ch) const
{
	return m_uszChar[ch] != 0;
}

void CharacterSet::Clear()
{
	memset(m_uszChar, 0, sizeof(char) * 256);
}


TextTokenParser::TextTokenParser() :
m_szText(nullptr),
m_pTextEnd(nullptr),
m_lastParsed(TextTokenParser::LAST_PARSED_NOTHING),
m_szWord(nullptr),
m_szString(nullptr),
m_szNumber(nullptr),
m_hSymbol('\0'),
m_pPos(nullptr)
{
}


TextTokenParser::~TextTokenParser()
{
	Close();
}



bool TextTokenParser::Open(const char *szText, const char *szTextEnd)
{
	if (!szText || !szTextEnd || szTextEnd <= szText)
	{
		assert(false && "TextTokenParser::Open( const char *, const char *) : bad ptr");
		return false;
	}

	if (m_szText != nullptr)
	{
		assert(false && "TextTokenParser::Open : reentry. Was opened yet.");
		return false;
	}

	m_szText = szText;
	m_pTextEnd = szTextEnd;
	m_pPos = m_szText;

	assert(!m_szWord && !m_szString);
	m_szWord = new char[WORD_MAX_LENGTH];
	*m_szWord = '\0';
	m_szString = new char[STRING_MAX_LENGTH];
	*m_szString = '\0';
	m_szNumber = new char[NUMBER_MAX_LENGTH];
	*m_szNumber = '\0';

	return true;
}

bool TextTokenParser::Open(const char *szText, int iTextLength)
{
	if (iTextLength <= 0)
	{
		assert(false && "TextTokenParser::Open( const char *, int ) : bad length.");
		return false;
	}

	if (!szText)
	{
		assert(false && "TextTokenParser::Open( const char *, int ) : bad ptr.");
		return false;
	}

	if (m_szText != nullptr)
	{
		assert(false && "TextTokenParser::Open( const char *, int ) : reentry. Was opened yet.");
		return false;
	}

	return Open(szText, szText + iTextLength);
}

void TextTokenParser::Close()
{
	m_szText = m_pTextEnd = m_pPos = nullptr;
	m_hSymbol = '\0';
	m_lastParsed = LAST_PARSED_NOTHING;

	delete[] m_szWord;
	m_szWord = nullptr;

	delete[] m_szString;
	m_szString = nullptr;

	delete[] m_szNumber;
	m_szNumber = nullptr;
}

bool TextTokenParser::Next()
{
	m_lastParsed = LAST_PARSED_NOTHING;

	if (m_szText == nullptr)
	{
		assert(false && "TextTokenParser::Next : text wasn't opened.");
		return false;
	}

	if (!ParseFirstChar())
		return false;

	assert(m_lastParsed != LAST_PARSED_NOTHING);

	switch (m_lastParsed)
	{
	case LAST_PARSED_WORD:
		ParseWord(); break;

	case LAST_PARSED_STRING:
		ParseString(); break;

	case LAST_PARSED_NUMBER:
		ParseNumber(); break;

	case LAST_PARSED_SYMBOL:
		ParseSymbol(); break;
	};

	return(m_lastParsed != LAST_PARSED_NOTHING);
}

const char* TextTokenParser::GetWord() const
{
	return m_szWord ? m_szWord : "";
}

bool TextTokenParser::IsWord() const
{
	return(m_lastParsed == LAST_PARSED_WORD);
}

const char* TextTokenParser::GetString() const
{
	return m_szString ? m_szString : "";
}

bool TextTokenParser::IsString() const
{
	return(m_lastParsed == LAST_PARSED_STRING);
}

const char* TextTokenParser::GetNumber() const
{
	return m_szNumber;
}

bool TextTokenParser::IsNumber() const
{
	return(m_lastParsed == LAST_PARSED_NUMBER);
}

char TextTokenParser::GetSymbol() const
{
	return m_hSymbol;
}

bool TextTokenParser::IsSymbol() const
{
	return(m_lastParsed == LAST_PARSED_SYMBOL);
}

void TextTokenParser::SetCharacterRule(char ch, EParsingRule rule)
{
	SetCharacterRule(ch, ch, rule);
}

void TextTokenParser::SetCharacterRule(char chMin, char chMax, EParsingRule rule)
{
	switch (rule)
	{
	case PARSING_RULE_WORD_START:
		m_wordStartSet.AddChar(chMin, chMax);
		m_stringQuotaSet.RemoveChar(chMin, chMax);
		m_numberStartSet.RemoveChar(chMin, chMax);
		m_symbolSet.RemoveChar(chMin, chMax);
		break;

	case PARSING_RULE_WORD_CONTENT:
		m_wordSet.AddChar(chMin, chMax);
		break;

	case PARSING_RULE_STRING_QUOTA:
		m_stringQuotaSet.AddChar(chMin, chMax);
		m_wordStartSet.RemoveChar(chMin, chMax);
		m_numberStartSet.RemoveChar(chMin, chMax);
		m_symbolSet.RemoveChar(chMin, chMax);
		break;

	case PARSING_RULE_NUMBER_START:
		m_numberStartSet.AddChar(chMin, chMax);
		m_stringQuotaSet.RemoveChar(chMin, chMax);
		m_wordStartSet.RemoveChar(chMin, chMax);
		m_symbolSet.RemoveChar(chMin, chMax);
		break;

	case PARSING_RULE_NUMBER_CONTENT:
		m_numberSet.AddChar(chMin, chMax);
		break;

	case PARSING_RULE_SYMBOL:
		m_symbolSet.AddChar(chMin, chMax);
		m_stringQuotaSet.RemoveChar(chMin, chMax);
		m_wordStartSet.RemoveChar(chMin, chMax);
		m_numberStartSet.RemoveChar(chMin, chMax);
		m_wordSet.RemoveChar(chMin, chMax);
		m_numberSet.RemoveChar(chMin, chMax);
		break;

	case PARSING_RULE_IGNORE:
		m_wordStartSet.RemoveChar(chMin, chMax);
		m_stringQuotaSet.RemoveChar(chMin, chMax);
		m_numberStartSet.RemoveChar(chMin, chMax);
		m_symbolSet.RemoveChar(chMin, chMax);
		break;

	default:
		assert(false && "TextTokenParser::SetCharacterRule : unknown rule.");
	}
}


void TextTokenParser::ClearCharsets()
{
	m_wordStartSet.Clear();
	m_stringQuotaSet.Clear();
	m_numberStartSet.Clear();
	m_symbolSet.Clear();
	m_wordSet.Clear();
	m_numberSet.Clear();
}

void TextTokenParser::SetDefaultCharsets()
{
	m_wordStartSet.Clear();
	m_wordStartSet.AddChar('A', 'Z');
	m_wordStartSet.AddChar('a', 'z');
	m_wordStartSet.AddChar('_');

	m_stringQuotaSet.Clear();
	m_stringQuotaSet.AddChar('\"');

	m_numberStartSet.Clear();
	m_numberStartSet.AddChar('0', '9');
	m_numberStartSet.AddChar('+');
	m_numberStartSet.AddChar('-');

	m_symbolSet.Clear();
	m_symbolSet.AddChar('\n');

	m_wordSet.Clear();
	m_wordSet.AddChar('A', 'Z');
	m_wordSet.AddChar('a', 'z');
	m_wordSet.AddChar('_');
	m_wordSet.AddChar('0', '9');

	m_numberSet.Clear();
	m_numberSet.AddChar('0', '9');
	m_numberSet.AddChar('.');
}

bool TextTokenParser::ParseFirstChar()
{
	assert(m_pPos);
	m_lastParsed = LAST_PARSED_NOTHING;

	// ignore junk characters
	for (; m_pPos < m_pTextEnd && IsIgnorableChar(*m_pPos); m_pPos++)
	{
	}

	if (m_pPos >= m_pTextEnd)
		return false;

	m_hFirstChar = *m_pPos++;

	if (IsStringQuotaChar(m_hFirstChar))
		m_lastParsed = LAST_PARSED_STRING;
	else if (IsWordStartChar(m_hFirstChar))
		m_lastParsed = LAST_PARSED_WORD;
	else if (IsNumberStartChar(m_hFirstChar))
		m_lastParsed = LAST_PARSED_NUMBER;
	else if (IsSymbolChar(m_hFirstChar))
		m_lastParsed = LAST_PARSED_SYMBOL;
	else
	{
		assert(false && "TextTokenParser::ParseFirstChar : unexecutable code executes.");
		return false;
	}

	return true;
}

void TextTokenParser::ParseWord()
{
	assert(m_lastParsed == LAST_PARSED_WORD);

	char * pBuf = m_szWord;
	char * pBufEnd = m_szWord + WORD_MAX_LENGTH;

	*pBuf++ = m_hFirstChar;

	for (; m_pPos < m_pTextEnd && IsWordChar(*m_pPos); m_pPos++)
	{
		if (pBuf < pBufEnd)
			*pBuf++ = *m_pPos;
	}

	if (pBuf != pBufEnd)
		*pBuf = '\0';
	else
	{
		*(pBuf - 1) = '\0';
		assert(false && "TextTokenParser::ParseWord : word buffer overflow.");
	}
}

void TextTokenParser::ParseString()
{
	assert(m_lastParsed == LAST_PARSED_STRING);

	char * pBuf = m_szString;
	char * pBufEnd = m_szString + STRING_MAX_LENGTH;

	while (m_pPos < m_pTextEnd)
	{
		if (IsStringQuotaChar(*m_pPos))
		{
			char* pForward = (char*)m_pPos + 1;
			if (pForward < m_pTextEnd && IsStringQuotaChar(*pForward))
			{//	double quote - copy as single
				m_pPos = pForward;
			}
			else // single quote - end of string
				break;
		}
		if (pBuf < pBufEnd)
			*pBuf++ = *m_pPos;
		m_pPos++;
	}

	if (m_pPos < m_pTextEnd)
		*m_pPos++;

	if (pBuf != pBufEnd)
		*pBuf = '\0';
	else
	{
		*(pBuf - 1) = '\0';
		assert(false && "TextTokenParser::ParseString : string buffer overflow.");
	}
}

void TextTokenParser::ParseNumber()
{
	assert(m_lastParsed == LAST_PARSED_NUMBER);

	char * pBuf = m_szNumber;
	char * pBufEnd = m_szNumber + NUMBER_MAX_LENGTH;

	*pBuf++ = m_hFirstChar;

	for (; m_pPos < m_pTextEnd && IsNumberChar(*m_pPos); m_pPos++)
	{
		if (pBuf < pBufEnd)
			*pBuf++ = *m_pPos;
	}

	if (pBuf != pBufEnd)
		*pBuf = '\0';
	else
	{
		*(pBuf - 1) = '\0';
		assert(false && "TextTokenParser::ParseNumber : number buffer overflow.");
	}
}

void TextTokenParser::ParseSymbol()
{
	assert(m_lastParsed == LAST_PARSED_SYMBOL);

	m_hSymbol = m_hFirstChar;
}

bool TextTokenParser::IsIgnorableChar(char ch) const
{
	return !(IsWordStartChar(ch) || IsStringQuotaChar(ch) ||
		IsNumberStartChar(ch) || IsSymbolChar(ch));
}

bool TextTokenParser::IsWordStartChar(char ch) const
{
	return m_wordStartSet.IsHere(ch);
}

bool TextTokenParser::IsStringQuotaChar(char ch) const
{
	return m_stringQuotaSet.IsHere(ch);
}


bool TextTokenParser::IsNumberStartChar(char ch) const
{
	return m_numberStartSet.IsHere(ch);
}

bool TextTokenParser::IsSymbolChar(char ch) const
{
	return m_symbolSet.IsHere(ch);
}

bool TextTokenParser::IsWordChar(char ch) const
{
	return m_wordSet.IsHere(ch);
}

bool TextTokenParser::IsNumberChar(char ch) const
{
	return m_numberSet.IsHere(ch);
}
