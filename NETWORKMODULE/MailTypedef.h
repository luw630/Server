#pragma once

class CMail
{
public:
	char	m_szSender[CONST_USERNAME];
	char	m_szRecver[CONST_USERNAME];
	char	m_szText[100];
	char	m_szTime[32];
	
	bool	m_bIsRead;
	int		m_iNumber;

	CMail() : m_bIsRead(false)
	{
		m_szSender[0] = 0;
		m_szRecver[0] = 0;
		m_szText[0] = 0;
		m_szTime[0] = 0;
		m_iNumber = -1;
	}
};