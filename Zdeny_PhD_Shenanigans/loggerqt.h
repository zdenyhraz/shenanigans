#pragma once
#include "stdafx.h"
#include "logger.h"

const std::vector<QColor> LOGLEVEL_CLR{ QColor(0, 255, 0), QColor(255, 255, 0), QColor(255, 0, 0), QColor(0,250,154), QColor(255, 140, 0), QColor(0, 170, 255), QColor(150, 150, 150) };

struct QtLogger : Logger
{
	QTextBrowser* m_TextBrowser;

	QtLogger(LOGLEVEL loglevel, QTextBrowser* TextBrowser) : Logger(loglevel), m_TextBrowser(TextBrowser) {};

	inline void Log(const std::string& msg, LOGLEVEL loglevel) override
	{
		if (loglevel <= m_loglevel)
		{
			m_TextBrowser->setTextColor(LOGLEVEL_CLR[loglevel]);
			if (loglevel != DEBUG) m_TextBrowser->append(QString::fromUtf8((LOGLEVEL_STRS[loglevel] + "[" + currentTime() + "] [" + LOGLEVEL_STR[loglevel] + "]: " + msg).c_str()));
			else m_TextBrowser->append(QString::fromUtf8((LOGLEVEL_STRS[loglevel] + msg).c_str()));

			QCoreApplication::processEvents();//this can be prolly avoided with Qthreads
		}
	}
};

