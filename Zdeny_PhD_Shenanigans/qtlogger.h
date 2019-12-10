#pragma once
#include "stdafx.h"
#include "logger.h"

const std::vector<QColor> LOGLEVEL_CLR{ QColor(255, 0, 0), QColor(255, 165, 0), QColor(255, 255, 0), QColor(0, 170, 255), QColor(150, 150, 150) };

struct QtLogger : Logger
{
	QTextBrowser* m_TextBrowser;

	QtLogger(LOGLEVEL loglevel, QTextBrowser* TextBrowser) : Logger(loglevel), m_TextBrowser(TextBrowser) {};

	inline void LogMessage(const std::string& msg, LOGLEVEL loglevel) override
	{
		if (loglevel <= m_loglevel)
		{
			QString qmsg = QString::fromUtf8((" [" + currentTime() + "] [" + LOGLEVEL_STR[loglevel] + "]: " + msg).c_str());
			m_TextBrowser->setTextColor(LOGLEVEL_CLR[loglevel]);
			m_TextBrowser->append(qmsg);
		}
	}

	inline void LogValue(const std::string& name, double value) override
	{
		QString qmsg = QString::fromUtf8((" [" + currentTime() + "] [VALUE]: " + name + " = " + to_string(value)).c_str());
		m_TextBrowser->setTextColor(LOGLEVEL_CLR[DEBUG]);
		m_TextBrowser->append(qmsg);
	}
};

