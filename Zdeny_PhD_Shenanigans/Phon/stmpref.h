#pragma once
#include "stdafx.h"

void ExtractTrigramsFromStmToFile(const std::string &stmfilepath, const std::string &unkwordsfilepath, const std::string &outputfilepath)
{
  // search through the STM file for all occurences of words specified by the UNKWORDS file and output corresponding STM trigrams to the OUTPUT file

  // check validity of files
  std::ifstream stmfile(stmfilepath);
  if (!stmfile.is_open() || !stmfile.good())
  {
    LOG_ERROR("Could not open file {}", stmfilepath);
    return;
  }

  std::ifstream unkwordsfile(unkwordsfilepath);
  if (!unkwordsfile.is_open() || !unkwordsfile.good())
  {
    LOG_ERROR("Could not open file {}", unkwordsfilepath);
    return;
  }

  // etc...
}
