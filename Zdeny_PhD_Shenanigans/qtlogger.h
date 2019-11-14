#pragma once
#include "stdafx.h"

enum LOGLEVEL { FATAL, WARN, SPECIAL, INFO, DEBUG };
static const std::vector<std::string> LOGLEVEL_STR{ "FATAL", "WARN", "INFO", "INFO", "DEBUG" };
static const std::vector<QColor> LOGLEVEL_CLR{ QColor(255, 0, 0), QColor(255, 165, 0), QColor(255, 255, 0), QColor(0, 170, 255), QColor(150, 150, 150) };

class QtLogger
{

private:
	LOGLEVEL m_loglevel;
	QTextBrowser* m_TextBrowser;
public:
	QtLogger(LOGLEVEL loglevel, QTextBrowser* TextBrowser) : m_loglevel(loglevel), m_TextBrowser(TextBrowser) {};

	inline void LogMessage(std::string msg, LOGLEVEL loglevel)
	{
		if (loglevel <= m_loglevel)
		{
			QString qmsg = QString::fromUtf8((" [" + currentTime() + "] [" + LOGLEVEL_STR[loglevel] + "]: " + msg).c_str());
			m_TextBrowser->setTextColor(LOGLEVEL_CLR[loglevel]);
			m_TextBrowser->append(qmsg);
		}
	}

	template <typename T>
	inline void LogValue(std::string name, T value)
	{
		QString qmsg = QString::fromUtf8((" [" + currentTime() + "] [VALUE]: " + name + " = " + to_string(value)).c_str());
		m_TextBrowser->setTextColor(LOGLEVEL_CLR[DEBUG]);
		m_TextBrowser->append(qmsg);
	}
};

